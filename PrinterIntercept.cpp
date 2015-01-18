/*
 * PrinterIntercept.cpp
 *
 *  Created on: 17.01.2015
 *      Author: Forsaken
 */

#include "PrinterIntercept.h"
#include "PrinterSettings.h"
#include <iostream>
#include <direct.h>
#include <Shlobj.h>

namespace Core {

PrinterIntercept* PrinterIntercept::instance = 0;

PrinterIntercept* PrinterIntercept::getInstance() {
	if (instance == 0) {
		instance = new PrinterIntercept();
	}
	return instance;
}

PrinterIntercept::PrinterIntercept() {
	customCommandsSending = false;
	// Initialize log writer
	char	sHomeDir[MAX_PATH];
	char	sFilename[MAX_PATH];
	if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, sHomeDir))) {
		sprintf(sFilename, "%s\\UpUsbIntercept", sHomeDir);
		// Ensure directory exists
		_mkdir(sFilename);
		// Create log writer
		sprintf(sFilename, "%s\\UpUsbIntercept\\PrinterIntercept.log", sHomeDir);
		log = new Core::SimpleLogWriter(sFilename);
	}
	// Initialize members
	lastWriteCommand = 0;
	lastWriteArgumentLo = 0;
	lastWriteArgumentHi = 0;
	lastWriteArgumentLong = 0;
	lastWriteCustom = 0;
	lastWriteKeep = false;
}

PrinterIntercept::~PrinterIntercept() {
	// TODO Auto-generated destructor stub
}

void PrinterIntercept::addCustomCommand(FixUp3DCustomCommand &command) {
	customCommands.push(command);
}

void PrinterIntercept::sendCustomCommand(WINUSB_INTERFACE_HANDLE interfaceHandle, FixUp3DCustomCommand &command) {
	log->writeString("[CustomCmd] Sending command: 0x")->writeBinaryBuffer(&command.command, 2)->writeString(" ...\r\n");
	ULONG	cmdBufferLen = command.argumentsLength + 2;
	UCHAR	cmdBuffer[cmdBufferLen];
	ULONG	transferLen = 0;
	// Write command to buffer
	*((PUSHORT)(cmdBuffer)) = command.command;
	// Write arguments to buffer
	memcpy(cmdBuffer + 2, command.arguments, command.argumentsLength);
	// Free commands argument buffer
	free(command.arguments);
	command.arguments = 0;
	// Write prepared command buffer
	if (!WinUsb_WritePipe(interfaceHandle, 0x01, cmdBuffer, cmdBufferLen, &transferLen, NULL)) {
		log->writeString("[CustomCmd] Failed to send custom command! Command: 0x")->writeBinaryBuffer(&command.command, 2)
			->writeString(" Arguments: ")->writeBinaryBuffer(command.arguments, command.argumentsLength)->writeString("\r\n");
	} else {
		log->writeString("[CustomCmd] Wrote command: 0x")->writeBinaryBuffer(&command.command, 2)->writeString(" ...\r\n");
		UCHAR	respBuffer[command.responseLength];
		if (!WinUsb_ReadPipe(interfaceHandle, 0x81, respBuffer, command.responseLength, &transferLen, NULL)) {
			log->writeString("[CustomCmd] Failed to read custom command reply! Command: 0x")->writeBinaryBuffer(&command.command, 2)
				->writeString(" Arguments: ")->writeBinaryBuffer(command.arguments, command.argumentsLength)->writeString("\r\n");
		} else {
			log->writeString("[CustomCmd] Command 0x")->writeBinaryBuffer(&command.command, 2)->writeString(" successfully sent!")
				->writeString(" Result: ")->writeBinaryBuffer(respBuffer, transferLen)->writeString("\r\n");
		}
	}
}

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

void PrinterIntercept::handleUsbWrite(WINUSB_INTERFACE_HANDLE interfaceHandle, UCHAR pipeID, PUCHAR buffer, ULONG bufferLength) {
	if (customCommandsSending) {
		return;	// Do not handle own commands
	}
	// Send queued custom commands to be sent
	if (!customCommands.empty()) {
		customCommandsSending = true;
		while (!customCommands.empty()) {
			sendCustomCommand(interfaceHandle, customCommands.front());
			customCommands.pop();
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
		handleUpCmdSend(lastWriteCommand, lastWriteArgumentLo, lastWriteArgumentHi, lastWriteArgumentLong, buffer, bufferLength);
	}
}

void PrinterIntercept::handleUpCmdSend(USHORT command, USHORT argLo, USHORT argHi, ULONG argLong, PUCHAR buffer, ULONG bufferLength) {
	PrinterSettings*	settings = PrinterSettings::getInstance();
	switch (lastWriteCommand) {
		case FIXUP3D_CMD_SET_NOZZLE1_TEMP:
		{
			settings->setHeaterTemperature(argLong, false);
			ULONG TargetTemperature = settings->getHeaterTemperature();
			if (TargetTemperature != argLong) {
				// Override temperature!
				log->writeString("[SetNozzle1Temp] Overriding Temperature: ")->writeLong(argLong)->writeString("°C > ")->writeLong(TargetTemperature)->writeString("°C\r\n");
				UPCMD_SetArgLong(buffer, TargetTemperature);
			}
			break;
		}
		case FIXUP3D_CMD_SET_UNKNOWN0A:
		case FIXUP3D_CMD_SET_UNKNOWN0B:
		case FIXUP3D_CMD_SET_UNKNOWN10:
		case FIXUP3D_CMD_SET_UNKNOWN14:
		case FIXUP3D_CMD_SET_UNKNOWN4C:
		case FIXUP3D_CMD_SET_UNKNOWN4D:
		case FIXUP3D_CMD_SET_UNKNOWN8E:
		case FIXUP3D_CMD_SET_UNKNOWN94:
		{
			log->writeString("[SetUnknown")->writeBinaryBuffer(&command, 2)->writeString("] Data: ")->writeBinaryBuffer(buffer+2,bufferLength-2)->writeString("\r\n");
			break;
		}
		case FIXUP3D_CMD_WRITE_MEM:
		{
			// Write to memory
			ULONG MemoryOffset = *((PULONG)(buffer+6));
			log->writeString("[WriteMem] Arg: ")->writeBinaryBuffer(&argLong, 4)->writeString(" Data: ")->writeBinaryBuffer(buffer+10, bufferLength-10)->writeString("\r\n");
			if (MemoryOffset == 0x00000039) {
				struct memSettings39 {
					ULONG	temperature;
					ULONG	unknown1;
					ULONG	unknown2;
				} *memSettings = ((memSettings39*)(buffer+10));
				settings->setHeaterTemperature(memSettings->temperature, false);
				ULONG TargetTemperature = settings->getHeaterTemperature();
				if (TargetTemperature != memSettings->temperature) {
					// Override temperature!
					log->writeString("[WriteMem] Overriding Temperature: ")->writeLong(memSettings->temperature)->writeString("°C > ")->writeLong(TargetTemperature)->writeString("°C\r\n");
					memSettings->temperature = TargetTemperature;
				}
			}
			break;
		}
		default:
			break;
	}
}

void PrinterIntercept::handleUpCmdReply(USHORT command, USHORT argLo, USHORT argHi, ULONG argLong, PUCHAR buffer, ULONG lengthTransferred) {
	switch (command) {
		case FIXUP3D_CMD_GET_BED_TEMP:
		{
			FLOAT temperature = *((PFLOAT)buffer);
			log->writeString("[GetBedTemp] Result: ")->writeFloat(temperature)->writeString("°C\r\n");
			break;
		}
		case FIXUP3D_CMD_GET_NOZZLE1_TEMP:
		{
			FLOAT temperature = *((PFLOAT)buffer);
			log->writeString("[GetNozzle1Temp] Result: ")->writeFloat(temperature)->writeString("°C\r\n");
			break;
		}
		case FIXUP3D_CMD_GET_NOZZLE2_TEMP:
		{
			FLOAT temperature = *((PFLOAT)buffer);
			log->writeString("[GetNozzle2Temp] Result: ")->writeFloat(temperature)->writeString("°C\r\n");
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
				log->writeString("[GetPosition] Read 1 / Result: ")->writeLong(result)->writeString("\r\n");
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
				log->writeString("[GetPosition] Read 2 / PosX: ")->writeFloat(posX)->writeString(" PosY: ")->writeFloat(posY)->writeString(" PosZ: ")->writeFloat(posZ)
						->writeString(" Nozzle: ")->writeFloat(nozzle)->writeString(" Debug: ")->writeLong(lengthTransferred)->writeString("\r\n");
			}
			break;
		}
		case FIXUP3D_CMD_GET_PREHEAT_TIMER:
		{
			ULONG result = *((PUSHORT)buffer);	// Time is stored in 2-minute units
			log->writeString("[GetPreheatTimer] Result: ")->writeLong(result)->writeString("\r\n");
			break;
		}
		case FIXUP3D_CMD_GET_LAYER:
		{
			ULONG result = *((PUSHORT)buffer);	// Time is stored in 2-minute units
			log->writeString("[GetLayer] Result: ")->writeLong(result)->writeString("\r\n");
			break;
		}
		// Yet unknown simple get commands
		case FIXUP3D_CMD_GET_UNKOWN00:
		case FIXUP3D_CMD_GET_UNKOWN01:
		case FIXUP3D_CMD_GET_UNKOWN02:
		case FIXUP3D_CMD_GET_UNKOWN03:
		case FIXUP3D_CMD_GET_UNKOWN04:
		case FIXUP3D_CMD_GET_UNKOWN05:
		case FIXUP3D_CMD_GET_UNKOWN0B:
		case FIXUP3D_CMD_GET_UNKOWN0F:
		case FIXUP3D_CMD_GET_UNKOWN10:
		case FIXUP3D_CMD_GET_UNKOWN14:
		case FIXUP3D_CMD_GET_UNKOWN15:
		case FIXUP3D_CMD_GET_UNKOWN1E:
		case FIXUP3D_CMD_GET_UNKOWN1F:
		case FIXUP3D_CMD_GET_UNKOWN20:
		case FIXUP3D_CMD_GET_UNKOWN21:
		case FIXUP3D_CMD_GET_UNKOWN32:
		{
			ULONG result = *((PUSHORT)buffer);
			log->writeString("[GetUnknown")->writeBinaryBuffer(&command, 2)->writeString("] Result: ")->writeLong(result)->writeString("\r\n");
			break;
		}
		case FIXUP3D_CMD_SET_PREHEAT_TIMER:
		{
			if (argLo == 0) {
				log->writeString("[SetPreheatTimer] Disabled preheating. Result: ")->writeBinaryBuffer(buffer,lengthTransferred)->writeString("\r\n");
			} else {
				log->writeString("[SetPreheatTimer] Duration: ")->writeLong(argLo / 30)->writeString("min Result: ")->writeBinaryBuffer(buffer,lengthTransferred)->writeString("\r\n");
			}
			break;
		}
		case FIXUP3D_CMD_SET_BED_TEMP:
		{
			log->writeString("[SetBedTemp] Result: ")->writeBinaryBuffer(buffer,lengthTransferred)->writeString("\r\n");
			break;
		}
		case FIXUP3D_CMD_SET_NOZZLE1_TEMP:
		{
			log->writeString("[SetNozzle1Temp] Result: ")->writeBinaryBuffer(buffer,lengthTransferred)->writeString("\r\n");
			break;
		}
		case FIXUP3D_CMD_SET_NOZZLE2_TEMP:
		{
			log->writeString("[SetNozzle2Temp] Result: ")->writeBinaryBuffer(buffer,lengthTransferred)->writeString("\r\n");
			break;
		}
		case FIXUP3D_CMD_WRITE_MEM:
		{
			log->writeString("[WriteMem] Result: ")->writeBinaryBuffer(buffer,lengthTransferred)->writeString("\r\n");
			break;
		}
		case FIXUP3D_CMD_NONE:
		{
			// Unknown command
			log->writeString("[NoCmd 0x")->writeBinaryBuffer(&lastWriteCustom,2)
				->writeString("] Args:")->writeBinaryBuffer(&lastWriteCustom+2,2)
				->writeString(" Result: ")->writeBinaryBuffer(buffer,lengthTransferred)->writeString("\r\n");
			break;
		}
		default:
		{
			// Unknown command
			log->writeString("[UnknownCmd 0x")->writeBinaryBuffer(&command,2)
				->writeString("] Args:")->writeBinaryBuffer(&argLong,4)
				->writeString(" Result: ")->writeBinaryBuffer(buffer,lengthTransferred)->writeString("\r\n");
			break;
		}
	}
}

void PrinterIntercept::setNozzle1Temp(ULONG temperature) {
	FixUp3DCustomCommand	cmd;
	cmd.command = FIXUP3D_CMD_SET_NOZZLE1_TEMP;
	cmd.arguments = malloc(4);
	memcpy(cmd.arguments, &temperature, 4);
	cmd.argumentsLength = 4;
	cmd.responseLength = 1;
	addCustomCommand(cmd);
}

} /* namespace Core */
