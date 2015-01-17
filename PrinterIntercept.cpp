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

PrinterIntercept::PrinterIntercept() {
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


void PrinterIntercept::handleUsbRead(WINUSB_INTERFACE_HANDLE InterfaceHandle, UCHAR PipeID, PUCHAR Buffer, ULONG LengthTransferred) {
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

void PrinterIntercept::handleUsbWrite(WINUSB_INTERFACE_HANDLE InterfaceHandle, UCHAR PipeID, PUCHAR Buffer, ULONG BufferLength) {
	if (BufferLength >= 2) {
		// Get command and argument(s)
		lastWriteCommand = *((PUSHORT)Buffer);
		if (BufferLength >= 4) {
			lastWriteArgumentLo = *((PUSHORT)(Buffer+2));
		} else {
			lastWriteArgumentLo = 0;
		}
		if (BufferLength >= 6) {
			lastWriteArgumentHi = *((PUSHORT)(Buffer+4));
		} else {
			lastWriteArgumentHi = 0;
		}
		lastWriteArgumentLong = (lastWriteArgumentHi<<16)|lastWriteArgumentLo;
		lastWriteCustom = 0;
		handleUpCmdSend(lastWriteCommand, lastWriteArgumentLo, lastWriteArgumentHi, lastWriteArgumentLong, Buffer, BufferLength);
	}
}

void PrinterIntercept::handleUpCmdSend(USHORT Command, USHORT ArgLo, USHORT ArgHi, ULONG ArgLong, PUCHAR Buffer, ULONG BufferLength) {
	PrinterSettings*	settings = PrinterSettings::getInstance();
	switch (lastWriteCommand) {
		case FIXUP3D_CMD_SET_NOZZLE1_TEMP:
		{
			settings->setHeaterTemperature(ArgLong, false);
			ULONG TargetTemperature = settings->getHeaterTemperature();
			if (TargetTemperature != ArgLong) {
				// Override temperature!
				log->writeString("[SetNozzle1Temp] Overriding Temperature: ")->writeLong(ArgLong)->writeString("°C > ")->writeLong(TargetTemperature)->writeString("°C\r\n");
				UPCMD_SetArgLong(Buffer, TargetTemperature);
			}
			break;
		}
		case FIXUP3D_CMD_WRITE_MEM:
		{
			// Write to memory
			ULONG MemoryOffset = *((PULONG)(Buffer+6));
			log->writeString("[WriteMem] Arg: ")->writeBinaryBuffer(&ArgLong, 4)->writeString(" Data: ")->writeBinaryBuffer(Buffer+10, BufferLength-10)->writeString("\r\n");
			if (MemoryOffset == 0x00000039) {
				struct memSettings39 {
					ULONG	temperature;
					ULONG	unknown1;
					ULONG	unknown2;
				} *memSettings = ((memSettings39*)(Buffer+10));
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

void PrinterIntercept::handleUpCmdReply(USHORT Command, USHORT ArgLo, USHORT ArgHi, ULONG ArgLong, PUCHAR Buffer, ULONG LengthTransferred) {
	switch (Command) {
		case FIXUP3D_CMD_GET_BED_TEMP:
		{
			FLOAT temperature = *((PFLOAT)Buffer);
			log->writeString("[GetBedTemp] Result: ")->writeFloat(temperature)->writeString("°C\r\n");
			break;
		}
		case FIXUP3D_CMD_GET_NOZZLE1_TEMP:
		{
			FLOAT temperature = *((PFLOAT)Buffer);
			log->writeString("[GetNozzle1Temp] Result: ")->writeFloat(temperature)->writeString("°C\r\n");
			break;
		}
		case FIXUP3D_CMD_GET_NOZZLE2_TEMP:
		{
			FLOAT temperature = *((PFLOAT)Buffer);
			log->writeString("[GetNozzle2Temp] Result: ")->writeFloat(temperature)->writeString("°C\r\n");
			break;
		}
		case FIXUP3D_CMD_GET_POSITION:
		{
			if (LengthTransferred == 5) {
				ULONG result = *((PULONG)Buffer);
				if (result == 1) {
					// Next read also belongs to this command
					lastWriteKeep = true;
				}
				log->writeString("[GetPosition] Read 1 / Result: ")->writeLong(result)->writeString("\r\n");
			} else if (LengthTransferred >= 49) {
				// Cmd: 0x768C /  Example:
				/*
				 * Offset   0        4          9        13         18       22         27       31         36       40
				 * Hex      0000dcc2 0000000000 00007042 0000000000 0000e6c2 0000000000 00f89745 0011123111 23111230 0000220030000000000000001000000 06
				 * Decim    -110                60                  -115                4863
				 * Desc     PosX     ???        PosY     ???        PosZ     ???        Nozzle	 ???        ???      ???                             END
				 */
				FLOAT posX = *((PFLOAT)Buffer);
				FLOAT posY = *((PFLOAT)(Buffer+9));
				FLOAT posZ = *((PFLOAT)(Buffer+18));
				FLOAT nozzle = *((PFLOAT)(Buffer+27));
				log->writeString("[GetPosition] Read 2 / PosX: ")->writeFloat(posX)->writeString(" PosY: ")->writeFloat(posY)->writeString(" PosZ: ")->writeFloat(posZ)
						->writeString(" Nozzle: ")->writeFloat(nozzle)->writeString(" Debug: ")->writeLong(LengthTransferred)->writeString("\r\n");
			}
			break;
		}
		case FIXUP3D_CMD_GET_PREHEAT_TIMER:
		{
			ULONG result = *((PUSHORT)Buffer);	// Time is stored in 2-minute units
			log->writeString("[GetPreheatTimer] Result: ")->writeLong(result)->writeString("\r\n");
			break;
		}
		// Yet unknown simple get commands
		case FIXUP3D_CMD_GET_UNKOWN00:
		case FIXUP3D_CMD_GET_UNKOWN01:
		case FIXUP3D_CMD_GET_UNKOWN02:
		case FIXUP3D_CMD_GET_UNKOWN03:
		case FIXUP3D_CMD_GET_UNKOWN04:
		case FIXUP3D_CMD_GET_UNKOWN05:
		case FIXUP3D_CMD_GET_UNKOWN0A:
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
			ULONG result = *((PUSHORT)Buffer);	// Time is stored in 2-minute units
			log->writeString("[GetUnknown")->writeBinaryBuffer(&Command, 2)->writeString("] Result: ")->writeLong(result)->writeString("\r\n");
			break;
		}
		case FIXUP3D_CMD_SET_PREHEAT_TIMER:
		{
			if (ArgLo == 0) {
				log->writeString("[SetPreheatTimer] Disabled preheating. Result: ")->writeBinaryBuffer(Buffer,LengthTransferred)->writeString("\r\n");
			} else {
				log->writeString("[SetPreheatTimer] Duration: ")->writeLong(ArgLo / 30)->writeString("min Result: ")->writeBinaryBuffer(Buffer,LengthTransferred)->writeString("\r\n");
			}
			break;
		}
		case FIXUP3D_CMD_SET_BED_TEMP:
		{
			log->writeString("[SetBedTemp] Result: ")->writeBinaryBuffer(Buffer,LengthTransferred)->writeString("\r\n");
			break;
		}
		case FIXUP3D_CMD_SET_NOZZLE1_TEMP:
		{
			log->writeString("[SetNozzle1Temp] Result: ")->writeBinaryBuffer(Buffer,LengthTransferred)->writeString("\r\n");
			break;
		}
		case FIXUP3D_CMD_SET_NOZZLE2_TEMP:
		{
			log->writeString("[SetNozzle2Temp] Result: ")->writeBinaryBuffer(Buffer,LengthTransferred)->writeString("\r\n");
			break;
		}
		case FIXUP3D_CMD_WRITE_MEM:
		{
			log->writeString("[WriteMem] Result: ")->writeBinaryBuffer(Buffer,LengthTransferred)->writeString("\r\n");
			break;
		}
		case FIXUP3D_CMD_NONE:
		{
			// Unknown command
			log->writeString("[NoCmd 0x")->writeBinaryBuffer(&lastWriteCustom,2)
				->writeString("] Args:")->writeBinaryBuffer(&lastWriteCustom+2,2)
				->writeString(" Result: ")->writeBinaryBuffer(Buffer,LengthTransferred)->writeString("\r\n");
			break;
		}
		default:
		{
			// Unknown command
			log->writeString("[UnknownCmd 0x")->writeBinaryBuffer(&Command,2)
				->writeString("] Args:")->writeBinaryBuffer(&ArgLong,4)
				->writeString(" Result: ")->writeBinaryBuffer(Buffer,LengthTransferred)->writeString("\r\n");
			break;
		}
	}
}

} /* namespace Core */
