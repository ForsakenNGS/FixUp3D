/*
 * PrinterIntercept.cpp
 *
 *  Created on: 17.01.2015
 *      Author: Forsaken
 */

#define DEBUG_MEMWRITE

#include "PrinterIntercept.h"
#include "PrinterSettings.h"
#include "UpPrinterData.h"
#include <iostream>
#include <direct.h>
#include <Shlobj.h>
#include <TCHAR.H>
#include <stdio.h>
#include <math.h>

namespace Core {

PrinterIntercept* PrinterIntercept::instance = NULL;

PrinterIntercept* PrinterIntercept::getInstance() {
	if (instance == 0) {
		instance = new PrinterIntercept();
	}
	return instance;
}

PrinterIntercept::PrinterIntercept() : fileMemDump(), fileDebugCSV(), log("PrinterIntercept", LogSections::SECTION_DEFAULT), logRaw("USB Raw", LogSections::SECTION_USB_RAW) {
	customCommandsSending = false;
	preheatStatus = FIXUP3D_PREHEAT_DISABLED;
	// Initialize custom logs
#ifdef DEBUG_MEMWRITE
	TCHAR sHomeDir[MAX_PATH];
	TCHAR sFilename[MAX_PATH];
	if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, sHomeDir))) {
		_stprintf(sFilename, TEXT("%s\\UpUsbIntercept"), sHomeDir);
		// Ensure directory exists
		_mkdir(sFilename);
		// Open target files
		_stprintf(sFilename, TEXT("%s\\UpUsbIntercept\\MemDump.dat"), sHomeDir);
		fileMemDump.open(sFilename, std::ios_base::binary | std::ios_base::out);
		_stprintf(sFilename, TEXT("%s\\UpUsbIntercept\\debug.csv"), sHomeDir);
		fileDebugCSV.open(sFilename, std::ios_base::out);
		fileDebugCSV << "Speed;Divider;X;Y;E\n";
		fileDebugCSV.flush();
	}
#endif
	// Initialize members
	interceptReply = FIXUP3D_REPLY_DONT_INTERCEPT;
	printerStatus = 0;
	lastWriteCommand = 0;
	lastWriteArgumentLo = 0;
	lastWriteArgumentHi = 0;
	lastWriteArgumentLong = 0;
	lastWriteCustom = 0;
	lastWriteKeep = false;
	memCurrentLayer = 0;
	memset(&memLastBlock, 0, sizeof(FixUp3DMemBlock));
	temperatureNozzle1Base = 0;
	temperatureNozzle2Base = 0;
	temperatureNozzle3Base = 0;
	memPosX = 0.0f;
	memPosY = 0.0f;
	memPosZ = 0.0f;
	memSpeedX = 0.0f;
	memSpeedY = 0.0f;
	memSpeedZ = 0.0f;
	memExtrudeSpeed = 0.0f;
}

PrinterIntercept::~PrinterIntercept() {
	if (fileMemDump.is_open()) {
		fileMemDump.flush();
		fileMemDump.close();
	}
	if (fileDebugCSV.is_open()) {
		fileDebugCSV.flush();
		fileDebugCSV.close();
	}
}

void PrinterIntercept::addCustomCommand(FixUp3DCustomCommand &command) {
	customCommands.push(command);
}

void PrinterIntercept::addCustomCommandDelay(ULONG delayInMs) {
	ULARGE_INTEGER timeResume;
	timeResume.QuadPart = 0;
	GetSystemTimeAsFileTime((LPFILETIME)&timeResume);
	timeResume.QuadPart += delayInMs * (ULONGLONG)10000;
	FixUp3DCustomCommand	cmdDelay;
	cmdDelay.command = FIXUP3D_CMD_PAUSE;
	cmdDelay.commandBytes = 2;
	memcpy( &cmdDelay.arguments, &timeResume, sizeof(ULARGE_INTEGER));
	cmdDelay.argumentsLength = sizeof(ULARGE_INTEGER);
	cmdDelay.responseLength = 0;
	addCustomCommand(cmdDelay);
}

BOOL PrinterIntercept::sendCustomCommand(WINUSB_INTERFACE_HANDLE interfaceHandle, FixUp3DCustomCommand &command) {
	if (command.command == FIXUP3D_CMD_PAUSE) {
		// Check if resume time is reached
		ULARGE_INTEGER	timeNow;
		GetSystemTimeAsFileTime((LPFILETIME)&timeNow);
		ULARGE_INTEGER	timeResume;
		memcpy( &timeResume, &command.arguments, sizeof(ULARGE_INTEGER) );
		if (timeNow.QuadPart >= timeResume.QuadPart) {
			return true;
		} else {
			return false;
		}
	}
	log.get(LogLevel::DEBUG) << "[CustomCmd] Sending command: 0x" << std::hex << command.command << " ...\n";
	ULONG	cmdBufferLen = command.commandBytes + command.argumentsLength;
	UCHAR	cmdBuffer[cmdBufferLen];
	ULONG	transferLen = 0;
	// Write command to buffer
	memcpy(cmdBuffer, &command.command, command.commandBytes);
	if (command.argumentsLength > 8) {
		if (command.argumentsPtr != NULL) {
			// Write arguments to buffer (from pointer)
			memcpy(cmdBuffer + command.commandBytes, command.argumentsPtr, command.argumentsLength);
			// Free memory allocated for the argument
			free(command.argumentsPtr);
			command.argumentsPtr = NULL;
		} else {
			log.get(LogLevel::WARNING) << "[CustomCmd] Arguments exceed 8 bytes, but null pointer given!";
		}
	} else if (command.argumentsLength > 0) {
		// Write arguments to buffer (from commands argument buffer)
		memcpy(cmdBuffer + command.commandBytes, command.arguments, command.argumentsLength);
	}
	log.get(LogLevel::DEBUG) << "[CustomCmd] Debug: 0x";
	log.writeBinaryAsHex(LogLevel::DEBUG, cmdBuffer, cmdBufferLen);
	log.get(LogLevel::DEBUG) << "\n";
	// Write prepared command buffer
	if (!WinUsb_WritePipe(interfaceHandle, 0x01, cmdBuffer, cmdBufferLen, &transferLen, NULL)) {
		log.get(LogLevel::DEBUG) << "[CustomCmd] Failed to send custom command! Command: 0x" << std::hex << command.command << " Arguments: ";
		log.writeBinaryAsHex(LogLevel::DEBUG, command.arguments, command.argumentsLength);
		log.get(LogLevel::DEBUG) << "\n";
	} else {
		logUpCmdSend(command.command, cmdBuffer, cmdBufferLen);
		log.get(LogLevel::DEBUG) << "[CustomCmd] Wrote command: 0x" << std::hex << command.command << " ...\n";
		if (command.responseLength > 0) {
			UCHAR	respBuffer[command.responseLength];
			if (!WinUsb_ReadPipe(interfaceHandle, 0x81, respBuffer, command.responseLength, &transferLen, NULL)) {
				log.get(LogLevel::DEBUG) << "[CustomCmd] Failed to read custom command reply! Command: 0x" << std::hex << command.command << ".\n";
			} else {
				logUpCmdReply(command.command, respBuffer, transferLen);
				log.get(LogLevel::DEBUG) << "[CustomCmd] Command 0x" << std::hex << command.command << " successfully sent!\n";
				log.get(LogLevel::DEBUG) << "Result: ";
				log.writeBinaryAsHex(LogLevel::DEBUG, respBuffer, transferLen);
				log.get(LogLevel::DEBUG) << "\n";
			}
		} else {
			log.get(LogLevel::DEBUG) << "[CustomCmd] Command 0x" << std::hex << command.command << " successfully sent!\n";
		}
	}
	return true;
}



/**
 * Called when a command is sent to the up printer.
 * Return false to prevent sending this command.
 */
BOOL PrinterIntercept::logUpCmdSend(USHORT command, PUCHAR buffer, ULONG bufferLength) {
	PrinterSettings*	settings = PrinterSettings::getInstance();
	switch (command) {
		case FIXUP3D_CMD_GET_PRINTERPARAM:
		{
			logRaw.get(LogLevel::DEBUG) << "> GET_PRINTERPARAM: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_SET_BED_TEMP:
		{
			logRaw.get(LogLevel::DEBUG) << "> SET_BED_TEMP: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_SET_NOZZLE1_TEMP:
		{
			logRaw.get(LogLevel::DEBUG) << "> SET_NOZZLE1_TEMP: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_SET_NOZZLE2_TEMP:
		{
			logRaw.get(LogLevel::DEBUG) << "> SET_NOZZLE2_TEMP: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_COMMIT_IDBLK:
		{
			logRaw.get(LogLevel::DEBUG) << "> CMD_COMMIT_IDBLK: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_COMMIT_INPORTBLK:
		{
			logRaw.get(LogLevel::DEBUG) << "> CMD_COMMIT_INPORTBLK: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_COMMIT_OUTPORTBLK:
		{
			logRaw.get(LogLevel::DEBUG) << "> CMD_COMMIT_OUTPORTBLK: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_COMMIT_SETBLK:
		{
			logRaw.get(LogLevel::DEBUG) << "> CMD_COMMIT_SETBLK: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_COMMIT_UNK00:
		{
			logRaw.get(LogLevel::DEBUG) << "> CMD_COMMIT_UNK00: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_COMMIT_UNK01:
		{
			logRaw.get(LogLevel::DEBUG) << "> CMD_COMMIT_UNK01: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_COMMIT_UNK04:
		{
			logRaw.get(LogLevel::DEBUG) << "> CMD_COMMIT_UNK04: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_PROGRAM_NEW:	// Called after sending print data
		{
			logRaw.get(LogLevel::DEBUG) << "> PROGRAM_NEW: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_PROGRAM_GO:	// Called after sending print data
		{
			logRaw.get(LogLevel::DEBUG) << "> PROGRAM_GO: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_SET_PREHEAT_TIMER:
		{
			logRaw.get(LogLevel::DEBUG) << "> SET_PREHEAT_TIMER: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;

		case FIXUP3D_CMD_SET_PRINTER_STATUS:
		{
			logRaw.get(LogLevel::DEBUG) << "> SET_PRINTER_STATUS: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_SET_UNKNOWN0A:
		case FIXUP3D_CMD_SET_UNKNOWN0B:
		case FIXUP3D_CMD_SET_UNKNOWN14:
		case FIXUP3D_CMD_SET_UNKNOWN4C:
		case FIXUP3D_CMD_SET_UNKNOWN4D:
		case FIXUP3D_CMD_SET_UNKNOWN8E:
		case FIXUP3D_CMD_SET_UNKNOWN94:
		{
			logRaw.get(LogLevel::DEBUG) << "> SET_UNKNOWN_" << std::hex << (command >> 8) << ": ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
		}
		break;
		case FIXUP3D_CMD_WRITE_MEM_1:
		case FIXUP3D_CMD_WRITE_MEM_2:
		case FIXUP3D_CMD_WRITE_MEM_3:
		{
			logRaw.get(LogLevel::DEBUG) << "> WRITE_MEM: ";
			logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
			logRaw.get(LogLevel::DEBUG) << "\n";
			logRaw.get(LogLevel::DEBUG) << "  LENGTH: " << bufferLength << "\n";
		}
		break;
		default:
			if ((command & 0xFF) == 0x76) {
				logRaw.get(LogLevel::DEBUG) << "> GET_" << std::hex << (command >> 8) << ": ";
				logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
				logRaw.get(LogLevel::DEBUG) << "\n";
			} else {
				logRaw.get(LogLevel::DEBUG) << "> UNKNOWN: ";
				logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, bufferLength);
				logRaw.get(LogLevel::DEBUG) << "\n";
			}
			break;
	}
	return true;
}

void PrinterIntercept::logUpCmdReply(USHORT command, PUCHAR buffer, ULONG lengthTransferred) {
	PrinterSettings* settings = PrinterSettings::getInstance();
	logRaw.get(LogLevel::DEBUG) << "< REPLY: ";
	logRaw.writeBinaryAsHex(LogLevel::DEBUG, buffer, lengthTransferred);
	logRaw.get(LogLevel::DEBUG) << "\n";
}

/**
 * Called before the up software reads data from the printer
 * Write some response into the buffer and return the number of bytes written to prevent reading and use the custom response instead.
 */
ULONG PrinterIntercept::handleUsbPreRead(WINUSB_INTERFACE_HANDLE interfaceHandle, UCHAR pipeID, PUCHAR buffer, ULONG bufferLength) {
	ULONG	bytesWritten = 0;
	switch (interceptReply) {
		case FIXUP3D_REPLY_ACKNOWLEDGED:
		{
			buffer[0] = 0x06;
			bytesWritten = 1;
		}
		break;

		case FIXUP3D_REPLY_CUSTOMPRINTERDATA:
		{
			bytesWritten = UpPrinterData::getInstance()->GetPrinterDataEmulation( buffer, bufferLength );

			//in case we got valid data from the emulation we return and keep the state (to emulate more data)
			if( bytesWritten>0 )
			{
				log.get(LogLevel::DEBUG) << "[GetPrinterParamEmulation] Result: " << std::dec << bytesWritten << " : ";
				log.writeBinaryAsHex(LogLevel::DEBUG, (void*)buffer, bytesWritten);
				log.get(LogLevel::DEBUG) << "\n";
				return bytesWritten;
			}
		}
		break;
	}
	interceptReply = FIXUP3D_REPLY_DONT_INTERCEPT;
	return bytesWritten;
}

/**
 * Called when the up software reads data from the printer
 */
void PrinterIntercept::handleUsbRead(WINUSB_INTERFACE_HANDLE InterfaceHandle, UCHAR PipeID, PUCHAR Buffer, ULONG LengthTransferred) {
	if (customCommandsSending) {
		return;	// Do not handle own commands
	}
	handleUpCmdReply(lastWriteCommand, lastWriteArgumentLo, lastWriteArgumentHi, lastWriteArgumentLong, Buffer, LengthTransferred);
	if (!lastWriteKeep) {
		lastWriteCustom = (lastWriteArgumentLo<<16)|lastWriteCommand;
		lastWriteCommand = FIXUP3D_CMD_NONE;
		lastWriteArgumentLo = 0;
		lastWriteArgumentHi = 0;
		lastWriteArgumentLong = 0;
	}
	lastWriteKeep = false;
}

BOOL PrinterIntercept::handleUsbWrite(WINUSB_INTERFACE_HANDLE interfaceHandle, UCHAR pipeID, PUCHAR buffer, ULONG bufferLength) {
	if (customCommandsSending) {
		return true;	// Do not handle own commands
	}
	// Send queued custom commands to be sent
	if (!customCommands.empty()) {
		customCommandsSending = true;
		while (!customCommands.empty()) {
			FixUp3DCustomCommand& command = customCommands.front();
			if (command.command == FIXUP3D_CMD_WRITE_MEM_3) {
				// Merge memwrites
				UCHAR	memWriteBuffer[2048];
				ULONG	memWriteTransfered = 0;
				ULONG	memWriteSize = 0;
				memset(memWriteBuffer, 0, 2048);
				while ( ((memWriteSize + 2 + command.commandBytes + command.argumentsLength) < 2048)
						&& ((command.command == FIXUP3D_CMD_WRITE_MEM_1) || (command.command == FIXUP3D_CMD_WRITE_MEM_2) || (command.command == FIXUP3D_CMD_WRITE_MEM_3)) ) {
					if (memWriteSize > 0) {
						// Add 2 bytes padding between each command
						memWriteSize += 2;
					}
					// Write command to buffer
					memcpy(memWriteBuffer + memWriteSize, &command.command, command.commandBytes);
					memWriteSize += command.commandBytes;
					// Write arguments to buffer
					if (command.argumentsLength > 8) {
						memcpy(memWriteBuffer + memWriteSize, command.argumentsPtr, command.argumentsLength);
						free(command.argumentsPtr);
						command.argumentsPtr = NULL;
					} else {
						memcpy(memWriteBuffer + memWriteSize, &command.arguments, command.argumentsLength);
					}
					memWriteSize += command.argumentsLength;
					// Remove command from queue
					customCommands.pop();
					if (!customCommands.empty()) {
						// Next command
						command = customCommands.front();
					} else {
						// Leave while
						command.command = FIXUP3D_CMD_NONE;
					}
				}
				// Write prepared command buffer
				if (!WinUsb_WritePipe(interfaceHandle, 0x01, memWriteBuffer, memWriteSize, &memWriteTransfered, NULL)) {
					log.get(LogLevel::DEBUG) << "[CustomCmd] Failed to send program data: ";
					log.writeBinaryAsHex(LogLevel::DEBUG, memWriteBuffer, memWriteSize);
					log.get(LogLevel::DEBUG) << "\n";
				} else {
					logRaw.get(LogLevel::DEBUG) << "> WRITE_MEM: ";
					logRaw.writeBinaryAsHex(LogLevel::DEBUG, memWriteBuffer, memWriteTransfered);
					logRaw.get(LogLevel::DEBUG) << "\n";
					logRaw.get(LogLevel::DEBUG) << "  LENGTH: " << memWriteTransfered << "/" << memWriteSize << "\n";
				}
			}
			if (command.command != FIXUP3D_CMD_NONE) {
				// Send other commands
				if (sendCustomCommand(interfaceHandle, customCommands.front())) {
					customCommands.pop();
				} else {
					break;
				}
			}
		}
		customCommandsSending = false;
	}
	// Handle write
	if (bufferLength >= 2) {
		// Get command and argument(s)
		lastWriteCommand = *((PUSHORT)buffer);
		if (bufferLength >= 4) {
			lastWriteArgumentLo = *((PUSHORT)(buffer+2));
		} else {
			lastWriteArgumentLo = 0;
		}
		if (bufferLength >= 6) {
			lastWriteArgumentHi = *((PUSHORT)(buffer+4));
		} else {
			lastWriteArgumentHi = 0;
		}
		lastWriteArgumentLong = (lastWriteArgumentHi<<16)|lastWriteArgumentLo;
		lastWriteCustom = 0;
		return handleUpCmdSend(lastWriteCommand, lastWriteArgumentLo, lastWriteArgumentHi, lastWriteArgumentLong, buffer, bufferLength);
	} else {
		// One byte command
		lastWriteCommand = *((PUCHAR)buffer);
		lastWriteArgumentLo = 0;
		lastWriteArgumentHi = 0;
		lastWriteArgumentLong = 0;
		lastWriteCustom = 0;
		return handleUpCmdSend(lastWriteCommand, lastWriteArgumentLo, lastWriteArgumentHi, lastWriteArgumentLong, buffer, bufferLength);
	}
	return true;
}

/**
 * Called when a command is sent to the up printer.
 * Return false to prevent sending this command.
 */
BOOL PrinterIntercept::handleUpCmdSend(USHORT command, USHORT argLo, USHORT argHi, ULONG argLong, PUCHAR buffer, ULONG bufferLength) {
	PrinterSettings*	settings = PrinterSettings::getInstance();
	logUpCmdSend(command, buffer, bufferLength);
	switch (command) {
		case FIXUP3D_CMD_GET_PRINTERPARAM:
		{
			//if we do not have print sets from real printer we initiate the real command, init play back of our sets otherwise
			if( !UpPrinterData::getInstance()->PrinterDataAvalibale() )
				UpPrinterData::getInstance()->PrinterDataReset();
			else
			{
				UpPrinterData::getInstance()->PrinterDataEmulationInit();
				interceptReply = FIXUP3D_REPLY_CUSTOMPRINTERDATA;
				return false; //do not send the original USB command
			}
		}
		break;
		case FIXUP3D_CMD_SET_BED_TEMP:
		{
			// TODO: Override bed temp
		}
		break;
		case FIXUP3D_CMD_SET_NOZZLE1_TEMP:
		{
			temperatureNozzle1Base = argLong;
			settings->setHeaterTemperature(3, argLong, false);
			ULONG TargetTemperature = settings->getHeaterTemperature(3);
			if (TargetTemperature != argLong) {
				// Override temperature!
				log.get(LogLevel::DEBUG) << "[SetNozzle1Temp] Overriding Temperature: " << std::dec << argLong << "°C > " << TargetTemperature << "°C\n";
				UPCMD_SetArgLong(buffer, TargetTemperature);
			}
		}
		break;
		case FIXUP3D_CMD_SET_NOZZLE2_TEMP:
		{
			temperatureNozzle2Base = argLong;
			settings->setHeaterTemperature(3, argLong, false);
			ULONG TargetTemperature = settings->getHeaterTemperature(3);
			if (TargetTemperature != argLong) {
				// Override temperature!
				log.get(LogLevel::DEBUG) << "[SetNozzle2Temp] Overriding Temperature: " << std::dec << argLong << "°C > " << TargetTemperature << "°C\n";
				UPCMD_SetArgLong(buffer, TargetTemperature);
			}
		}
		break;
		case FIXUP3D_CMD_PROGRAM_GO:	// Called after sending print data
		{
			memCurrentLayer = 0;
		}
		break;
		case FIXUP3D_CMD_SET_UNKNOWN0A:
		case FIXUP3D_CMD_SET_UNKNOWN0B:
		case FIXUP3D_CMD_SET_UNKNOWN14:
		case FIXUP3D_CMD_SET_UNKNOWN4C:
		case FIXUP3D_CMD_SET_UNKNOWN4D:
		case FIXUP3D_CMD_SET_UNKNOWN8E:
		case FIXUP3D_CMD_SET_UNKNOWN94:
		{
			log.get(LogLevel::DEBUG) << "[SetUnknown" << std::hex << command << "] Data: ";
			log.writeBinaryAsHex(LogLevel::DEBUG, buffer+2, bufferLength-2);
			log.get(LogLevel::DEBUG) << "\n";
			break;
		}
		case FIXUP3D_CMD_WRITE_MEM_1:
		case FIXUP3D_CMD_WRITE_MEM_2:
		case FIXUP3D_CMD_WRITE_MEM_3:
		{
			PUCHAR pBuf = buffer;
			ULONG len = bufferLength;
			for (;;) {
				UCHAR numBlocks = *(pBuf + 1);
				pBuf += 2; //JUMP OVER 2Fxx
				len -= 2;
				// Iterate memory blocks
				for (UCHAR curBlock = 0; curBlock < numBlocks; curBlock++) {
					handleUpMemBlock((FixUp3DMemBlock*) pBuf);
					pBuf += sizeof(FixUp3DMemBlock);
					len -= sizeof(FixUp3DMemBlock);
				}
				// Check for end of buffer
				if (0 == len) {
					break; //END OF BUFFER
				}
				// Check for padding bytes
				if ( (0 != *pBuf) || (0 != *(pBuf+1)) ) {
					break; //INVALID chaining terminator
				}
				pBuf += 2;	//JUMP OVER 0000
				len -= 2;
				// Check for next command
				if (0x2F != *pBuf) {
					break; //INVALID NEXT CMD
				}
			}
			break;
		}
		default:
			break;
	}
	return true;
}

void PrinterIntercept::handleUpCmdReply(USHORT command, USHORT argLo, USHORT argHi, ULONG argLong, PUCHAR buffer, ULONG lengthTransferred) {
	PrinterSettings* settings = PrinterSettings::getInstance();
	logUpCmdReply(command, buffer, lengthTransferred);
	switch (command)
	{
		case FIXUP3D_CMD_GET_PRINTERPARAM:
		{
			log.get(LogLevel::DEBUG) << "[GetPrinterParam] Result: " << std::dec << lengthTransferred << " : ";
			log.writeBinaryAsHex(LogLevel::DEBUG, buffer, lengthTransferred);
			log.get(LogLevel::DEBUG) << "\n";
			lastWriteKeep = UpPrinterData::getInstance()->PrinterDataFromUpResponse(buffer,lengthTransferred);
			PrinterSettings::getInstance()->updatePrintSet();
		}
		break;

		case FIXUP3D_CMD_GET_BED_TEMP:
		{
			FLOAT temperature = *((PFLOAT)buffer);
			log.get(LogLevel::DEBUG) << "[GetBedTemp] Result: " << std::dec << temperature << "°C\n";
			break;
		}
		case FIXUP3D_CMD_GET_NOZZLE1_TEMP:
		{
			FLOAT temperature = *((PFLOAT)buffer);
			log.get(LogLevel::DEBUG) << "[GetNozzle1Temp] Result: " << std::dec << temperature << "°C\n";
			break;
		}
		case FIXUP3D_CMD_GET_NOZZLE2_TEMP:
		{
			FLOAT temperature = *((PFLOAT)buffer);
			log.get(LogLevel::DEBUG) << "[GetNozzle2Temp] Result: " << std::dec << temperature << "°C\n";
			break;
		}
		case FIXUP3D_CMD_GET_POSITION:
		{
			if (lengthTransferred == 5) {
				ULONG result = *((PULONG)buffer);
				if (result == 1) {
					// Next read also belongs to this command
					lastWriteKeep = true;
				}
				log.get(LogLevel::DEBUG) << "[GetPosition] Read 1 / Result: " << std::dec << result << "\n";
			} else if (lengthTransferred >= 49) {
				// Cmd: 0x768C /  Example:
				/*
				 * Offset   0        4          9        13         18       22         27       31         36       40
				 * Hex      0000dcc2 0000000000 00007042 0000000000 0000e6c2 0000000000 00f89745 0011123111 23111230 0000220030000000000000001000000 06
				 * Decim    -110                60                  -115                4863
				 * Desc     PosX     ???        PosY     ???        PosZ     ???        Nozzle	 ???        ???      ???                             END
				 */
				FLOAT posX = *((PFLOAT)buffer);
				FLOAT posY = *((PFLOAT)(buffer+9));
				FLOAT posZ = *((PFLOAT)(buffer+18));
				FLOAT nozzle = *((PFLOAT)(buffer+27));
				log.get(LogLevel::DEBUG) << "[GetPosition] Read 2 / PosX: " << std::dec << posX << " PosY: " << posY << " PosZ: " << posZ << " Nozzle:" << nozzle << " Debug: " << lengthTransferred << "\n";
			}
			break;
		}
		case FIXUP3D_CMD_GET_PREHEAT_TIMER:
		{
			ULONG result = *((PUSHORT)buffer);	// Time is stored in 2-second units
			if (!settings->getPreheatDelayPrint()) {
				preheatStatus = FIXUP3D_PREHEAT_DISABLED;
			} else if ((preheatStatus == FIXUP3D_PREHEAT_DISABLED) && settings->getPreheatDelayPrint()) {
				preheatStatus = FIXUP3D_PREHEAT_IDLE;
			}
			settings->updatePreheatTimer(result);
			settings->updateWindowTitle();
			log.get(LogLevel::DEBUG) << "[GetPreheatTimer] Result: " << std::dec << result << "\n";
			if (result == 0) {
				// Not heating
				if (preheatStatus == FIXUP3D_PREHEAT_HEATING) {
					preheatStatus = FIXUP3D_PREHEAT_PRINTING;
					printAgain();
				}
			} else if (result > 1) {
				// Preheat cooldown running
				if (preheatStatus == FIXUP3D_PREHEAT_STOPPING) {
					preheatStatus = FIXUP3D_PREHEAT_HEATING;
				}
			}
			break;
		}
		case FIXUP3D_CMD_GET_LAYER:
		{
			ULONG result = *((PUSHORT)buffer);	// Time is stored in 2-minute units
			log.get(LogLevel::DEBUG) << "[GetLayer] Result: " << std::dec << result << "\n";
			break;
		}
		// Yet unknown set commands
		case FIXUP3D_CMD_SET_UNKNOWN0A:
		case FIXUP3D_CMD_SET_UNKNOWN0B:
		case FIXUP3D_CMD_SET_PRINTER_STATUS:
		case FIXUP3D_CMD_SET_UNKNOWN14:
		case FIXUP3D_CMD_SET_UNKNOWN4C:
		case FIXUP3D_CMD_SET_UNKNOWN4D:
		case FIXUP3D_CMD_SET_UNKNOWN8E:
		case FIXUP3D_CMD_SET_UNKNOWN94:
		{
			log.get(LogLevel::DEBUG) << "[SetUnknown" << std::hex << command << "] Args: " << std::dec << argLong << " Result: ";
			log.writeBinaryAsHex(LogLevel::DEBUG, buffer, lengthTransferred);
			log.get(LogLevel::DEBUG) << "\n";
			break;
		}
		case FIXUP3D_CMD_GET_UNKNOWN_STATUS:
		{
			ULONG result = *((PUSHORT)buffer);
			log.get(LogLevel::DEBUG) << "[GetUnknownStatus] Result: " << std::dec << result << "\n";
			switch (result) {
				case 2:
					// Wait until preheat is done?
					if (printerStatus == FIXUP3D_STATUS_PRINTING) {
						if (preheatStatus == FIXUP3D_PREHEAT_IDLE) {
							preheatStatus = FIXUP3D_PREHEAT_STOPPING;
							// Wait until beeping stopped
							addCustomCommandDelay(5000);
							// Send stop command to printer in order to prevent starting the print job
							stopPrint();
							// Wait until print was stopped...
							addCustomCommandDelay(6000);
							// ... then start the preheat timer ...
							setPreheatTimer(settings->getPreheatTime());
						} else if (preheatStatus == FIXUP3D_PREHEAT_HEATING) {
							preheatStatus = FIXUP3D_PREHEAT_PRINTING;
						}
					} else {
						if (preheatStatus == FIXUP3D_PREHEAT_PRINTING) {
							preheatStatus = FIXUP3D_PREHEAT_IDLE;
						}
					}
					break;
			}
		}
		break;
		case FIXUP3D_CMD_GET_PRINTER_STATUS:
		{
			ULONG result = *((PUSHORT)buffer);
			log.get(LogLevel::DEBUG) << "[GetPrinterStatus] Result: " << std::dec << result << "\n";
			printerStatus = result;
			break;
		}
		// Yet unknown simple get commands
		case FIXUP3D_CMD_GET_UNKOWN01:
		case FIXUP3D_CMD_GET_UNKOWN02:
		case FIXUP3D_CMD_GET_UNKOWN03:
		case FIXUP3D_CMD_GET_UNKOWN04:
		case FIXUP3D_CMD_GET_UNKOWN05:
		case FIXUP3D_CMD_GET_UNKOWN0B:
		case FIXUP3D_CMD_GET_UNKOWN0F:
		case FIXUP3D_CMD_GET_UNKOWN14:
		case FIXUP3D_CMD_GET_UNKOWN15:
		case FIXUP3D_CMD_GET_UNKOWN1E:
		case FIXUP3D_CMD_GET_UNKOWN1F:
		case FIXUP3D_CMD_GET_UNKOWN20:
		case FIXUP3D_CMD_GET_UNKOWN21:
		case FIXUP3D_CMD_GET_UNKOWN2A:
		case FIXUP3D_CMD_GET_UNKOWN2B:
		case FIXUP3D_CMD_GET_UNKOWN32:
		case FIXUP3D_CMD_GET_UNKOWN36:
		case FIXUP3D_CMD_GET_UNKOWN3E:
		{
			ULONG result = *((PUSHORT)buffer);
			log.get(LogLevel::DEBUG) << "[GetUnknown" << std::hex << command << "] Result: " << std::dec << result << "\n";
			break;
		}
		case FIXUP3D_CMD_SET_PREHEAT_TIMER:
		{
			if (argLo == 0) {
				log.get(LogLevel::DEBUG) << "[SetPreheatTimer] Disabled preheating. Result: ";
				log.writeBinaryAsHex(LogLevel::DEBUG, buffer, lengthTransferred);
				log.get(LogLevel::DEBUG) << "\n";
			} else {
				log.get(LogLevel::DEBUG) << "[SetPreheatTimer] Duration: " << std::dec << (argLo / 30) << "min. Result: ";
				log.writeBinaryAsHex(LogLevel::DEBUG, buffer, lengthTransferred);
				log.get(LogLevel::DEBUG) << "\n";
			}
			break;
		}
		case FIXUP3D_CMD_SET_BED_TEMP:
		{
			log.get(LogLevel::DEBUG) << "[SetBedTemp] Result: ";
			log.writeBinaryAsHex(LogLevel::DEBUG, buffer, lengthTransferred);
			log.get(LogLevel::DEBUG) << "\n";
			break;
		}
		case FIXUP3D_CMD_SET_NOZZLE1_TEMP:
		{
			log.get(LogLevel::DEBUG) << "[SetNozzle1Temp] Result: ";
			log.writeBinaryAsHex(LogLevel::DEBUG, buffer, lengthTransferred);
			log.get(LogLevel::DEBUG) << "\n";
			break;
		}
		case FIXUP3D_CMD_SET_NOZZLE2_TEMP:
		{
			log.get(LogLevel::DEBUG) << "[SetNozzle2Temp] Result: ";
			log.writeBinaryAsHex(LogLevel::DEBUG, buffer, lengthTransferred);
			log.get(LogLevel::DEBUG) << "\n";
			break;
		}
		case FIXUP3D_CMD_WRITE_MEM_1:
		case FIXUP3D_CMD_WRITE_MEM_2:
		case FIXUP3D_CMD_WRITE_MEM_3:
		{
			UCHAR numBlocks = UPCMD_GetArgHi(command);
			log.get(LogLevel::DEBUG) << "[WriteMem" << std::dec << numBlocks << "] Result: ";
			log.writeBinaryAsHex(LogLevel::DEBUG, buffer, lengthTransferred);
			log.get(LogLevel::DEBUG) << "\n";
			break;
		}
		case FIXUP3D_CMD_NONE:
		{
			// Unknown command
			log.get(LogLevel::DEBUG) << "[NoCmd 0x" << std::hex << lastWriteCustom << "] Result: ";
			log.writeBinaryAsHex(LogLevel::DEBUG, buffer, lengthTransferred);
			log.get(LogLevel::DEBUG) << "\n";
			break;
		}
		default:
		{
			// Unknown command
			log.get(LogLevel::DEBUG) << "[UnknownCmd 0x" << std::hex << command << "] Args: " << argLong << " Result: ";
			log.writeBinaryAsHex(LogLevel::DEBUG, buffer, lengthTransferred);
			log.get(LogLevel::DEBUG) << "\n";
			break;
		}
	}
}

/**
 * Process a memory-/command-block for writing to sdcard
 */
void PrinterIntercept::handleUpMemBlock(FixUp3DMemBlock* memBlock) {
#ifdef DEBUG_MEMWRITE
	if (fileMemDump.is_open()) {
		fileMemDump.write((char*)memBlock, sizeof(FixUp3DMemBlock));
		fileMemDump.flush();
	}
#endif
	switch (memBlock->command) {
		case FIXUP3D_MEM_CMD_STOP:
		{
			log.get(LogLevel::INFO) << "[WriteMem] Stop!\n";
		}
		break;
		case FIXUP3D_MEM_CMD_MOVE_FLOAT:
		{
			if (memLastBlock.command != FIXUP3D_MEM_CMD_MOVE_FLOAT) {
				// First block
				memcpy(&memLastBlock, memBlock, sizeof(FixUp3DMemBlock));
			} else {
				// Second block
				memSpeedX = memLastBlock.params.floats.fParam1;
				memPosX = memLastBlock.params.floats.fParam2;
				memSpeedY = memLastBlock.params.floats.fParam3;
				memPosY = memLastBlock.params.floats.fParam4;
				memSpeedZ = memBlock->params.floats.fParam1;
				memPosZ = memBlock->params.floats.fParam2;
				memExtrudeSpeed = memBlock->params.floats.fParam4 / memBlock->params.floats.fParam3;
				log.get(LogLevel::INFO) << "[WriteMem] MOVE_FLOAT\t" << std::dec << std::setfill(' ')
					<< "  X: " << std::setw(12) << memPosX << "mm (" << std::setw(12) << memSpeedX << "mm/s)"
					<< "  Y:" << std::setw(12) << memPosY << "mm (" << std::setw(12) << memSpeedY << "mm/s)"
					<< "  Z: " << std::setw(12) << memPosZ << "mm (" << std::setw(12) << memSpeedZ << "mm/s)"
					<< "  E: " << std::setw(12) << memBlock->params.floats.fParam4 << "mm (" << std::setw(12) << memBlock->params.floats.fParam3 << "mm/s\n";
				/*
				float speedX = memLastBlock.params.floats.fParam1;
				float posX = memLastBlock.params.floats.fParam2;
				float speedY = memLastBlock.params.floats.fParam3;
				float posY = memLastBlock.params.floats.fParam4;
				float speedZ = memBlock->params.floats.fParam1;
				float posZ = memBlock->params.floats.fParam2;
				float speedE = memBlock->params.floats.fParam3;
				float posE = memBlock->params.floats.fParam4;
				log.get(LogLevel::INFO) << "[WriteMem] MOVE_FLOAT" << std::dec
						<< "\tX: " << posX << "(" << speedX << ")"
						<< "\tY:" << posY << "(" << speedY << ")"
						<< "\tZ: " << posZ << "(" << speedZ << ")"
						<< "\tE: " << posE << "(" << speedE << ")" << "\n";
				*/
				// Clear prev block
				memset(&memLastBlock, 0, sizeof(FixUp3DMemBlock));
			}
		}
		break;
		case FIXUP3D_MEM_CMD_MOVE_SHORT:
		{
			int32_t moveSpeed = memBlock->params.shorts.wParam1;
			int32_t moveDivider = memBlock->params.shorts.wParam2;
			int32_t p3 = memBlock->params.shorts.wParam3;
			int32_t p4 = memBlock->params.shorts.wParam4;
			int32_t p5 = memBlock->params.shorts.wParam5;
			int32_t p6 = memBlock->params.shorts.wParam6;
			int32_t p7 = memBlock->params.shorts.wParam7;
			int32_t p8 = memBlock->params.shorts.wParam8;
			int32_t v10 = (int32_t)0xFFFFFFF / moveSpeed;
			int32_t v13 = moveSpeed*(moveSpeed - 1);
			double moveDistX = ((( (p3 + v10) * moveSpeed ) + ((v13*p6)*2) - 511 - moveSpeed*v10) / 512.0) / 850.0;
			double moveDistY = ((( (p4 + v10) * moveSpeed ) + ((v13*p7)*2) - 511 - moveSpeed*v10) / 512.0) / 850.0;
			double moveDistE = ((( (p5 + v10) * moveSpeed ) + ((v13*p8)*2) - 511 - moveSpeed*v10) / 512.0) / 40.0;
			memSpeedX = moveDistX / moveSpeed;
			memPosX += moveDistX;
			memSpeedY = moveDistY / moveSpeed;
			memPosY += moveDistY;
			memExtrudeSpeed = (float)moveDistE;
			log.get(LogLevel::INFO) << "[WriteMem] MOVE_SHORT\t" << std::dec << std::setfill(' ')
				<< "  X: " << std::setw(12) << memPosX << "mm (" << std::setw(12) << memSpeedX << "mm/s)"
				<< "  Y:" << std::setw(12) << memPosY << "mm (" << std::setw(12) << memSpeedY << "mm/s)"
				<< "  E: " << std::setw(12) << memExtrudeSpeed << "mm/s  XY: " << std::setw(12) << (fabsf(memSpeedX) + fabsf(memSpeedY)) << "  T: " << moveDistE << "ms\n";
			fileDebugCSV << moveSpeed << ";" << moveDivider << ";" << moveDistX << ";" << moveDistY << ";"  << moveDistE << "\n";
			fileDebugCSV.flush();
			/*
			int32_t moveSpeed = memBlock->params.shorts.wParam1;
			int32_t moveDivider = memBlock->params.shorts.wParam2;
			int32_t p3 = memBlock->params.shorts.wParam3;
			int32_t p4 = memBlock->params.shorts.wParam4;
			int32_t p5 = memBlock->params.shorts.wParam5;
			int32_t p6 = memBlock->params.shorts.wParam6;
			int32_t p7 = memBlock->params.shorts.wParam7;
			int32_t p8 = memBlock->params.shorts.wParam8;
			 int32_t v10 = (int32_t)0xFFFFFFF / moveSpeed;
			 int32_t v13 = moveSpeed*(moveSpeed - 1);

			 int32_t r1 = (( (p3 + v10) * p1 ) + ((v13*p6)/2) - 511 - p1*v10)/512.0;
			 int32_t r2 = (( (p4 + v10) * p1 ) + ((v13*p7)/2) - 511 - p1*v10)/512.0;
			 int32_t r3 = (( (p5 + v10) * p1 ) + ((v13*p8)/2) - 511 - p1*v10)/512.0;

			 double r4 = p6/512.0;
			 double r5 = p7/512.0;
			 double r6 = p8/512.0;
			fileDebugCSV << p1 << ";" << p2 << ";" << v10 << ";" << v13 << ";" << ((float)r1 / p1) << ";" << r2 << ";" << r3 << ";" << r4 << ";" << r5 << ";" << r6 << "\n";
			fileDebugCSV.flush();
			*/
		}
		break;
		case FIXUP3D_MEM_CMD_UNKNOWN5:
		{
			log.get(LogLevel::INFO) << "[WriteMem] Unknown5: " << memBlock->params.longs.lParam1 << "\n";
		}
		break;
		case FIXUP3D_MEM_CMD_SET_PARAM:
		{
			handleUpMemBlock_SetParam(memBlock->params);
		}
		break;
		default:
		{
		#ifdef DEBUG_MEMWRITE
			log.get(LogLevel::INFO) << "[WriteMem] Unknown MemBlock: ";
			log.writeBinaryAsHex(LogLevel::INFO, memBlock, sizeof(FixUp3DMemBlock));
			log.get(LogLevel::INFO) << "\n";
		#endif
		}
		break;
	}
}

void PrinterIntercept::handleUpMemBlock_SetParam(FixUp3DMemBlockParams& params) {
		ULONG				paramType = params.longs.lParam1;
	switch (paramType) {
		case FIXUP3D_MEM_PARAM_LAYER:
		{
			memCurrentLayer = params.longs.lParam2;
			log.get(LogLevel::INFO) << "[WriteMem] SET_PARAM Layer " << std::dec << memCurrentLayer << "\n";
		}
		break;
		case FIXUP3D_MEM_PARAM_BED_TEMP:
		{
			log.get(LogLevel::INFO) << "[WriteMem] SET_PARAM Bed Temperature: " << std::dec << params.longs.lParam2 << "°C\n";
		}
		break;
		case FIXUP3D_MEM_PARAM_HEIGHT:
		{
			log.get(LogLevel::INFO) << "[WriteMem] SET_PARAM Report Height: " << std::dec << params.floats.fParam2 << "\n";
		}
		break;
		case FIXUP3D_MEM_PARAM_TIME_REMAINING:
		{
			log.get(LogLevel::INFO) << "[WriteMem] SET_PARAM Report Time remaining: " << std::dec << params.longs.lParam2 << "\n";
		}
		break;
		case FIXUP3D_MEM_PARAM_TIME_PERCENT:
		{
			log.get(LogLevel::INFO) << "[WriteMem] SET_PARAM Report Time gone: " << std::dec << params.longs.lParam2 << "%\n";
		}
		break;
		case FIXUP3D_MEM_PARAM_NOZZLE1_TEMP:
		{
			ULONG&	temperature = params.longs.lParam2;
			if (temperatureNozzle1Base == 0) {
				temperatureNozzle1Base = temperature;
			}
			PrinterSettings*	settings = PrinterSettings::getInstance();
			settings->setHeaterTemperature(memCurrentLayer, temperature, false);
			ULONG TargetTemperature = settings->getHeaterTemperature(memCurrentLayer);
			if (TargetTemperature != temperature) {
				// Override temperature!
				log.get(LogLevel::INFO) << "[WriteMem] Overriding Nozzle 1 Temperature: " << std::dec << temperature << "°C > " << TargetTemperature << "°C\n";
				params.longs.lParam2 = TargetTemperature;
			}
			log.get(LogLevel::INFO) << "[WriteMem] SET_PARAM Nozzle 1 Temperature " << std::dec << TargetTemperature << "°C\n";
		}
		break;
		case FIXUP3D_MEM_PARAM_NOZZLE2_TEMP:
		{
			ULONG&	temperature = params.longs.lParam2;
			if (temperatureNozzle2Base == 0) {
				temperatureNozzle2Base = temperature;
			}
			/*
			PrinterSettings*	settings = PrinterSettings::getInstance();
			settings->setHeaterTemperature(memCurrentLayer, temperature, false);
			ULONG TargetTemperature = settings->getHeaterTemperature(memCurrentLayer);
			if (TargetTemperature != temperature) {
				// Override temperature!
				log.get(LogLevel::INFO) << "[WriteMem] Overriding Nozzle 2 Temperature: " << std::dec << temperature << "°C > " << TargetTemperature << "°C\n";
				params.longs.lParam2 = TargetTemperature;
			}
			*/
			log.get(LogLevel::INFO) << "[WriteMem] SET_PARAM Nozzle 2 Temperature " << std::dec << temperature << "°C\n";
		}
		break;
		case FIXUP3D_MEM_PARAM_MOTORS_OFF:
		{
			log.get(LogLevel::INFO) << "[WriteMem] SET_PARAM Disable Servos? " << params.longs.lParam2 << "\n";
		}
		break;
		default:
		{
		#ifdef DEBUG_MEMWRITE
			log.get(LogLevel::INFO) << "[WriteMem] Unknown MemBlock Parameter: ";
			log.writeBinaryAsHex(LogLevel::DEBUG, &params, sizeof(FixUp3DMemBlockParams));
			log.get(LogLevel::INFO) << "\n";
		#endif
		}
		break;
	}
}

void PrinterIntercept::sendGcode(const char* file) {
	// TODO: Convert gcode and write result to sdcard / send it to the printer
}

void PrinterIntercept::sendGetConnected() {
	FixUp3DCustomCommand	cmdGetConnected;
	cmdGetConnected.command = FIXUP3D_CMD_GET_CONNECTED;
	cmdGetConnected.commandBytes = 2;
	cmdGetConnected.argumentsLength = 0;
	cmdGetConnected.responseLength = 5;
	addCustomCommand(cmdGetConnected);
}

void PrinterIntercept::sendGetUnknownStatus() {
	FixUp3DCustomCommand	cmdGetUnknownStatus;
	cmdGetUnknownStatus.command = FIXUP3D_CMD_GET_UNKNOWN_STATUS;
	cmdGetUnknownStatus.commandBytes = 2;
	cmdGetUnknownStatus.argumentsLength = 0;
	cmdGetUnknownStatus.responseLength = 5;
	addCustomCommand(cmdGetUnknownStatus);
}

void PrinterIntercept::sendGetPrinterStatus() {
	FixUp3DCustomCommand	cmdGetPrinterStatus;
	cmdGetPrinterStatus.command = FIXUP3D_CMD_GET_PRINTER_STATUS;
	cmdGetPrinterStatus.commandBytes = 2;
	cmdGetPrinterStatus.argumentsLength = 0;
	cmdGetPrinterStatus.responseLength = 5;
	addCustomCommand(cmdGetPrinterStatus);
}

void PrinterIntercept::sendGetUnknown8E() {
	FixUp3DCustomCommand	cmdGetUnknown8E;
	cmdGetUnknown8E.command = FIXUP3D_CMD_GET_UNKOWN8E;
	cmdGetUnknown8E.commandBytes = 2;
	cmdGetUnknown8E.argumentsLength = 0;
	cmdGetUnknown8E.responseLength = 5;
	addCustomCommand(cmdGetUnknown8E);
}

void PrinterIntercept::sendProgramGo() {
	FixUp3DCustomCommand	cmdProgramGo;
	cmdProgramGo.command = FIXUP3D_CMD_PROGRAM_GO;
	cmdProgramGo.commandBytes = 1;
	cmdProgramGo.argumentsLength = 0;
	cmdProgramGo.responseLength = 12;
	addCustomCommand(cmdProgramGo);
}

void PrinterIntercept::sendProgramNew() {
	FixUp3DCustomCommand	cmdProgramNew;
	cmdProgramNew.command = FIXUP3D_CMD_PROGRAM_NEW;
	cmdProgramNew.commandBytes = 1;
	cmdProgramNew.argumentsLength = 0;
	cmdProgramNew.responseLength = 1;
	addCustomCommand(cmdProgramNew);
}

/**
 * Gets the number of commands left in the currently writing program
 */
void PrinterIntercept::sendProgramCmdsFree() {
	FixUp3DCustomCommand	cmdUnknown46;
	cmdUnknown46.command = FIXUP3D_CMD_PROGRAM_CMDS_FREE;
	cmdUnknown46.commandBytes = 1;
	cmdUnknown46.argumentsLength = 0;
	cmdUnknown46.responseLength = 5;
	addCustomCommand(cmdUnknown46);
}

void PrinterIntercept::sendUnknown53() {
	FixUp3DCustomCommand	cmdUnknown53;
	cmdUnknown53.command = FIXUP3D_CMD_UNKNOWN53;
	cmdUnknown53.commandBytes = 1;
	cmdUnknown53.argumentsLength = 0;
	cmdUnknown53.responseLength = 1;
	addCustomCommand(cmdUnknown53);
}

void PrinterIntercept::sendProgramCommitHead() {
	FixUp3DCustomCommand	cmdUnknown4C32;
	cmdUnknown4C32.command = FIXUP3D_CMD_UNKNOWN4C32;
	cmdUnknown4C32.commandBytes = 2;
	cmdUnknown4C32.argumentsLength = 0;
	cmdUnknown4C32.responseLength = 5;
	addCustomCommand(cmdUnknown4C32);
}

void PrinterIntercept::sendUnknown4C33() {
	FixUp3DCustomCommand	cmdUnknown4C33;
	cmdUnknown4C33.command = FIXUP3D_CMD_UNKNOWN4C33;
	cmdUnknown4C33.commandBytes = 2;
	cmdUnknown4C33.argumentsLength = 0;
	cmdUnknown4C33.responseLength = 5;
	addCustomCommand(cmdUnknown4C33);
}

void PrinterIntercept::sendProgramCommitLayers() {
	FixUp3DCustomCommand	cmdUnknown4C35;
	cmdUnknown4C35.command = FIXUP3D_CMD_UNKNOWN4C35;
	cmdUnknown4C35.commandBytes = 2;
	cmdUnknown4C35.argumentsLength = 0;
	cmdUnknown4C35.responseLength = 5;
	addCustomCommand(cmdUnknown4C35);
}

/**
 * Starts/stops writing a program to sdcard
 * programId	index of the program to be written (only 9 seen so far)
 * programMode	1 = Start writing / 0 = Stop writing
 */
void PrinterIntercept::sendProgramWrite(UCHAR programId, UCHAR programMode) {
	FixUp3DCustomCommand	cmdUnknown6C;
	cmdUnknown6C.command = FIXUP3D_CMD_PROGRAM_WRITE;
	cmdUnknown6C.commandBytes = 1;
	memcpy(cmdUnknown6C.arguments, &programId, 1);
	memcpy(cmdUnknown6C.arguments + 1, &programMode, 1);
	cmdUnknown6C.argumentsLength = 2;
	cmdUnknown6C.responseLength = 1;
	addCustomCommand(cmdUnknown6C);
}

void PrinterIntercept::sendUnknown7330() {
	FixUp3DCustomCommand	cmdUnknown7330;
	cmdUnknown7330.command = FIXUP3D_CMD_UNKNOWN7330;
	cmdUnknown7330.commandBytes = 2;
	cmdUnknown7330.argumentsLength = 0;
	cmdUnknown7330.responseLength = 1;
	addCustomCommand(cmdUnknown7330);
}

void PrinterIntercept::sendUnknown7331() {
	FixUp3DCustomCommand	cmdUnknown7331;
	cmdUnknown7331.command = FIXUP3D_CMD_UNKNOWN7331;
	cmdUnknown7331.commandBytes = 2;
	cmdUnknown7331.argumentsLength = 0;
	cmdUnknown7331.responseLength = 1;
	addCustomCommand(cmdUnknown7331);
}

void PrinterIntercept::sendUnknown7332() {
	FixUp3DCustomCommand	cmdUnknown7332;
	cmdUnknown7332.command = FIXUP3D_CMD_UNKNOWN7332;
	cmdUnknown7332.commandBytes = 2;
	cmdUnknown7332.argumentsLength = 0;
	cmdUnknown7332.responseLength = 1;
	addCustomCommand(cmdUnknown7332);
}

void PrinterIntercept::sendUnknown7333() {
	FixUp3DCustomCommand	cmdUnknown7333;
	cmdUnknown7333.command = FIXUP3D_CMD_UNKNOWN7333;
	cmdUnknown7333.commandBytes = 2;
	cmdUnknown7333.argumentsLength = 0;
	cmdUnknown7333.responseLength = 1;
	addCustomCommand(cmdUnknown7333);
}

void PrinterIntercept::setNozzle1Temp(ULONG temperature) {
	FixUp3DCustomCommand	cmdSetTemp;
	cmdSetTemp.command = FIXUP3D_CMD_SET_NOZZLE1_TEMP;
	cmdSetTemp.commandBytes = 2;
	memcpy(cmdSetTemp.arguments, &temperature, 4);
	cmdSetTemp.argumentsLength = 4;
	cmdSetTemp.responseLength = 1;
	addCustomCommand(cmdSetTemp);
}

void PrinterIntercept::setPreheatTimer(ULONG value) {
	FixUp3DCustomCommand	cmdSetPreheatTimer;
	cmdSetPreheatTimer.command = FIXUP3D_CMD_SET_PREHEAT_TIMER;
	cmdSetPreheatTimer.commandBytes = 2;
	memcpy(cmdSetPreheatTimer.arguments, &value, 4);
	cmdSetPreheatTimer.argumentsLength = 4;
	cmdSetPreheatTimer.responseLength = 1;
	addCustomCommand(cmdSetPreheatTimer);
}

void PrinterIntercept::setUnknown0A(ULONG value) {
	FixUp3DCustomCommand	cmdSetUnknown0A;
	cmdSetUnknown0A.command = FIXUP3D_CMD_SET_UNKNOWN0A;
	cmdSetUnknown0A.commandBytes = 2;
	memcpy(cmdSetUnknown0A.arguments, &value, 4);
	cmdSetUnknown0A.argumentsLength = 4;
	cmdSetUnknown0A.responseLength = 1;
	addCustomCommand(cmdSetUnknown0A);
}

void PrinterIntercept::setUnknown0B(ULONG value) {
	FixUp3DCustomCommand	cmdSetUnknown0B;
	cmdSetUnknown0B.command = FIXUP3D_CMD_SET_UNKNOWN0B;
	cmdSetUnknown0B.commandBytes = 2;
	memcpy(cmdSetUnknown0B.arguments, &value, 4);
	cmdSetUnknown0B.argumentsLength = 4;
	cmdSetUnknown0B.responseLength = 1;
	addCustomCommand(cmdSetUnknown0B);
}

void PrinterIntercept::setPrinterStatus(ULONG value) {
	FixUp3DCustomCommand	cmdSetUnknown10;
	cmdSetUnknown10.command = FIXUP3D_CMD_SET_PRINTER_STATUS;
	cmdSetUnknown10.commandBytes = 2;
	memcpy(cmdSetUnknown10.arguments, &value, 4);
	cmdSetUnknown10.argumentsLength = 4;
	cmdSetUnknown10.responseLength = 1;
	addCustomCommand(cmdSetUnknown10);
}

void PrinterIntercept::setUnknown14(ULONG value) {
	FixUp3DCustomCommand	cmdSetUnknown14;
	cmdSetUnknown14.command = FIXUP3D_CMD_SET_UNKNOWN14;
	cmdSetUnknown14.commandBytes = 2;
	memcpy(cmdSetUnknown14.arguments, &value, 4);
	cmdSetUnknown14.argumentsLength = 4;
	cmdSetUnknown14.responseLength = 1;
	addCustomCommand(cmdSetUnknown14);
}

void PrinterIntercept::setUnknown4C(ULONG value) {
	FixUp3DCustomCommand	cmdSetUnknown4C;
	cmdSetUnknown4C.command = FIXUP3D_CMD_SET_UNKNOWN4C;
	cmdSetUnknown4C.commandBytes = 2;
	memcpy(cmdSetUnknown4C.arguments, &value, 4);
	cmdSetUnknown4C.argumentsLength = 4;
	cmdSetUnknown4C.responseLength = 1;
	addCustomCommand(cmdSetUnknown4C);
}

void PrinterIntercept::setUnknown4D(ULONG value) {
	FixUp3DCustomCommand	cmdSetUnknown4D;
	cmdSetUnknown4D.command = FIXUP3D_CMD_SET_UNKNOWN4D;
	cmdSetUnknown4D.commandBytes = 2;
	memcpy(cmdSetUnknown4D.arguments, &value, 4);
	cmdSetUnknown4D.argumentsLength = 4;
	cmdSetUnknown4D.responseLength = 1;
	addCustomCommand(cmdSetUnknown4D);
}

void PrinterIntercept::setUnknown8E(ULONG value) {
	FixUp3DCustomCommand	cmdSetUnknown8E;
	cmdSetUnknown8E.command = FIXUP3D_CMD_SET_UNKNOWN8E;
	cmdSetUnknown8E.commandBytes = 2;
	memcpy(cmdSetUnknown8E.arguments, &value, 4);
	cmdSetUnknown8E.argumentsLength = 4;
	cmdSetUnknown8E.responseLength = 1;
	addCustomCommand(cmdSetUnknown8E);
}

void PrinterIntercept::writeMemory(std::queue<FixUp3DMemBlock>& memBlocks) {
	FixUp3DCustomCommand	cmdWriteMem;
	cmdWriteMem.responseLength = 0;
	while (!memBlocks.empty()) {
		if (memBlocks.size() > 2) {
			cmdWriteMem.command = FIXUP3D_CMD_WRITE_MEM_3;
			cmdWriteMem.commandBytes = 2;
			cmdWriteMem.argumentsLength = sizeof(FixUp3DMemBlock) * 3;
			cmdWriteMem.argumentsPtr = malloc( sizeof(FixUp3DMemBlock) * 3 );
			memcpy(cmdWriteMem.argumentsPtr, &memBlocks.front(), sizeof(FixUp3DMemBlock));
			memBlocks.pop();
			memcpy(cmdWriteMem.argumentsPtr + sizeof(FixUp3DMemBlock), &memBlocks.front(), sizeof(FixUp3DMemBlock));
			memBlocks.pop();
			memcpy(cmdWriteMem.argumentsPtr + sizeof(FixUp3DMemBlock) * 2, &memBlocks.front(), sizeof(FixUp3DMemBlock));
			memBlocks.pop();
		} else if (memBlocks.size() > 1) {
			cmdWriteMem.command = FIXUP3D_CMD_WRITE_MEM_2;
			cmdWriteMem.commandBytes = 2;
			cmdWriteMem.argumentsLength = sizeof(FixUp3DMemBlock) * 2;
			cmdWriteMem.argumentsPtr = malloc( sizeof(FixUp3DMemBlock) * 2 );
			memcpy(cmdWriteMem.argumentsPtr, &memBlocks.front(), sizeof(FixUp3DMemBlock));
			memBlocks.pop();
			memcpy(cmdWriteMem.argumentsPtr + sizeof(FixUp3DMemBlock), &memBlocks.front(), sizeof(FixUp3DMemBlock));
			memBlocks.pop();
		} else {
			cmdWriteMem.command = FIXUP3D_CMD_WRITE_MEM_1;
			cmdWriteMem.commandBytes = 2;
			cmdWriteMem.argumentsLength = sizeof(FixUp3DMemBlock);
			cmdWriteMem.argumentsPtr = malloc( sizeof(FixUp3DMemBlock) );
			memcpy(cmdWriteMem.argumentsPtr, &memBlocks.front(), sizeof(FixUp3DMemBlock));
			memBlocks.pop();
		}
		addCustomCommand(cmdWriteMem);
	}
}

void PrinterIntercept::writeMemory1(FixUp3DMemBlock& memBlock) {
	FixUp3DCustomCommand	cmdWriteMem;
	cmdWriteMem.command = FIXUP3D_CMD_WRITE_MEM_1;
	cmdWriteMem.commandBytes = 2;
	cmdWriteMem.argumentsLength = sizeof(FixUp3DMemBlock);
	cmdWriteMem.argumentsPtr = malloc( sizeof(FixUp3DMemBlock) );
	memcpy(cmdWriteMem.argumentsPtr, &memBlock, sizeof(FixUp3DMemBlock));
	cmdWriteMem.responseLength = 0;
	addCustomCommand(cmdWriteMem);
}

void PrinterIntercept::writeMemory2(FixUp3DMemBlock& memBlock, FixUp3DMemBlock& memBlock2) {
	FixUp3DCustomCommand	cmdWriteMem;
	cmdWriteMem.command = FIXUP3D_CMD_WRITE_MEM_2;
	cmdWriteMem.commandBytes = 2;
	cmdWriteMem.argumentsLength = sizeof(FixUp3DMemBlock) * 2;
	cmdWriteMem.argumentsPtr = malloc( sizeof(FixUp3DMemBlock) * 2 );
	memcpy(cmdWriteMem.argumentsPtr, &memBlock, sizeof(FixUp3DMemBlock));
	memcpy(cmdWriteMem.argumentsPtr + sizeof(FixUp3DMemBlock), &memBlock2, sizeof(FixUp3DMemBlock));
	cmdWriteMem.responseLength = 0;
	addCustomCommand(cmdWriteMem);
}

void PrinterIntercept::writeMemory3(FixUp3DMemBlock& memBlock, FixUp3DMemBlock& memBlock2, FixUp3DMemBlock& memBlock3) {
	FixUp3DCustomCommand	cmdWriteMem;
	cmdWriteMem.command = FIXUP3D_CMD_WRITE_MEM_3;
	cmdWriteMem.commandBytes = 2;
	cmdWriteMem.argumentsLength = sizeof(FixUp3DMemBlock) * 3;
	cmdWriteMem.argumentsPtr = malloc( sizeof(FixUp3DMemBlock) * 3 );
	memcpy(cmdWriteMem.argumentsPtr, &memBlock, sizeof(FixUp3DMemBlock));
	memcpy(cmdWriteMem.argumentsPtr + sizeof(FixUp3DMemBlock), &memBlock2, sizeof(FixUp3DMemBlock));
	memcpy(cmdWriteMem.argumentsPtr + sizeof(FixUp3DMemBlock) * 2, &memBlock3, sizeof(FixUp3DMemBlock));
	cmdWriteMem.responseLength = 0;
	addCustomCommand(cmdWriteMem);
}

void PrinterIntercept::stopPrint() {
	// Do something?
	sendUnknown7330();		// >7330
	sendUnknown7331();		// >7331
	sendUnknown7332();		// >7332
	sendUnknown7333();		// >7333
	// Do something?
	sendUnknown53();		// >53
	// Enable something?
	setPrinterStatus(1);		// >5610
	// Disable some stuff... (what is what?)
	setPreheatTimer(0);		// >5616
	setUnknown14(0);		// >5614
	sendGetUnknown8E();		// >768E
	setUnknown8E(0);		// >568E
	sendGetUnknownStatus();	// >7600
	// Execute some program?
	sendProgramNew();		// >63
	sendUnknown4C33();		// >4C33
	sendProgramGo();		// >58
	// Enable something again?
	setPrinterStatus(1);		// >5610
	// Update status
	sendGetConnected();
	sendGetPrinterStatus();
	printerStatus = FIXUP3D_STATUS_UNKNOWN7;
}

void PrinterIntercept::printAgain() {
	// Update status
	sendGetConnected();
	sendGetConnected();
	sendGetPrinterStatus();
	// Execute some program?
	sendProgramNew();	// >63
	sendProgramWrite(9);	// >6C0900
	sendProgramGo();	// >58
	// Update status
	sendGetConnected();
	sendGetPrinterStatus();
}

} /* namespace Core */
