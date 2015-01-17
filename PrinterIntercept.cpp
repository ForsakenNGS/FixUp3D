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
}

PrinterIntercept::~PrinterIntercept() {
	// TODO Auto-generated destructor stub
}


void PrinterIntercept::handleUsbRead(WINUSB_INTERFACE_HANDLE InterfaceHandle, UCHAR PipeID, PUCHAR Buffer, ULONG LengthTransferred) {
	switch (lastWriteCommand) {
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
		case FIXUP3D_CMD_PREHEAT:
		{
			USHORT duration = *((PUSHORT)Buffer) * 2;	// Time is stored in 2-minute units
			log->writeString("[Preheat] Duration: ")->writeLong(duration)->writeString("min Result: ")->writeBinaryBuffer(Buffer,LengthTransferred)->writeString("\r\n");
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
		default:
		{
			// Unknown command
			log->writeString("[UnknownCmd 0x")->writeBinaryBuffer(&lastWriteCommand,2)
				->writeString("] Args:")->writeBinaryBuffer(&lastWriteArgumentLong,4)
				->writeString(" Result: ")->writeBinaryBuffer(Buffer,LengthTransferred)->writeString("\r\n");
			break;
		}
	}
}

void PrinterIntercept::handleUsbWrite(WINUSB_INTERFACE_HANDLE InterfaceHandle, UCHAR PipeID, PUCHAR Buffer, ULONG BufferLength) {
	if (BufferLength >= 2) {
		// Get command and argument(s)
		lastWriteCommand = *((PUSHORT)Buffer);
		if (BufferLength >= 4) {
			lastWriteArgumentLo = *((PUSHORT)Buffer+2);
		} else {
			lastWriteArgumentLo = 0;
		}
		if (BufferLength >= 6) {
			lastWriteArgumentHi = *((PUSHORT)Buffer+4);
		} else {
			lastWriteArgumentHi = 0;
		}
		lastWriteArgumentLong = (lastWriteArgumentHi<<16)|lastWriteArgumentLo;
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

void PrinterIntercept::handleUpCmdReply(USHORT Command, USHORT Arg1, USHORT Arg2, ULONG ArgLong) {

}

} /* namespace Core */
