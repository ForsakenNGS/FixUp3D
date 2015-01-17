/*
 * dll_funcs.cpp
 *
 *  Created on: 13.01.2015
 *      Author: Forsaken
 */

#define DEBUG_LOG

#include "dll_funcs.h"
#include "PrinterIntercept.h"
#include "PrinterSettings.h"
#include "SimpleLogWriter.h"
#include <iostream>
#include <direct.h>
#include <Shlobj.h>

HMODULE					hWinUsbLib = 0;
WINUSB_INTERFACE_HANDLE hLatestInterfaceHandle = 0;

Core::SimpleLogWriter*	log = NULL;
Core::SimpleLogWriter*	logRaw = NULL;
Core::PrinterIntercept*	printerIntercept = NULL;
ULONG					debugDumpIndex = 0;

void initWinUsbWrapper() {
#ifdef DEBUG_LOG
	if (log == NULL) {
		char	sHomeDir[MAX_PATH];
		char	sFilename[MAX_PATH];
		if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, sHomeDir))) {
			sprintf(sFilename, "%s\\UpUsbIntercept", sHomeDir);
			// Ensure directory exists
			_mkdir(sFilename);
			// Detailed debug.log
			sprintf(sFilename, "%s\\UpUsbIntercept\\debug.log", sHomeDir);
			log = new Core::SimpleLogWriter(sFilename);
			// Raw read/write data
			sprintf(sFilename, "%s\\UpUsbIntercept\\debug_raw.log", sHomeDir);
			logRaw = new Core::SimpleLogWriter(sFilename);
		}
	}
#endif
	if (printerIntercept == NULL) {
		printerIntercept = new Core::PrinterIntercept();
	}
	if (hWinUsbLib == 0) {
		// Load original winusb lib
		hWinUsbLib = LoadLibrary(TEXT("C:\\Windows\\System32\\winusb.dll"));
#ifdef DEBUG_LOG_EX
		log->writeString("Loaded original 'winusb.dll'.\r\n");
#endif
	}
}

BOOL __stdcall WinUsb_Wrapper_AbortPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID
) {
	WINUSB_ABORT_PIPE func = (WINUSB_ABORT_PIPE) GetProcAddress(hWinUsbLib, "WinUsb_AbortPipe");
	return func(InterfaceHandle, PipeID);
}

BOOL __stdcall WinUsb_Wrapper_ControlTransfer(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		WINUSB_SETUP_PACKET SetupPacket,
		PUCHAR Buffer,
		ULONG BufferLength,
		PULONG LengthTransferred,
		LPOVERLAPPED Overlapped
) {
	WINUSB_CONTROL_TRANSFER func = (WINUSB_CONTROL_TRANSFER) GetProcAddress(hWinUsbLib, "WinUsb_ControlTransfer");
	return func(InterfaceHandle, SetupPacket, Buffer, BufferLength, LengthTransferred, Overlapped);
}

BOOL __stdcall WinUsb_Wrapper_FlushPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID
) {
	WINUSB_FLUSH_PIPE func = (WINUSB_FLUSH_PIPE) GetProcAddress(hWinUsbLib, "WinUsb_FlushPipe");
	return func(InterfaceHandle, PipeID);
}

BOOL __stdcall WinUsb_Wrapper_Free(
		WINUSB_INTERFACE_HANDLE InterfaceHandle
) {
	WINUSB_FREE func = (WINUSB_FREE) GetProcAddress(hWinUsbLib, "WinUsb_Free");
	return func(InterfaceHandle);
}

BOOL __stdcall WinUsb_Wrapper_GetAssociatedInterface(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AssociatedInterfaceIndex,
		PWINUSB_INTERFACE_HANDLE AssociatedInterfaceHandle
) {
	WINUSB_GET_ASSOCIATED_INTERFACE func = (WINUSB_GET_ASSOCIATED_INTERFACE) GetProcAddress(hWinUsbLib, "WinUsb_GetAssociatedInterface");
	return func(InterfaceHandle, AssociatedInterfaceIndex, AssociatedInterfaceHandle);
}

BOOL __stdcall WinUsb_Wrapper_GetCurrentAlternateSetting(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		PUCHAR AlternateSetting
) {
	WINUSB_GET_CURRENT_ALTERNATE_SETTING func = (WINUSB_GET_CURRENT_ALTERNATE_SETTING) GetProcAddress(hWinUsbLib, "WinUsb_GetCurrentAlternateSetting");
	return func(InterfaceHandle, AlternateSetting);
}

BOOL __stdcall WinUsb_Wrapper_GetDescriptor(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR DescriptorType,
		UCHAR Index,
		USHORT LanguageID,
		PUCHAR Buffer,
		ULONG BufferLength,
		PULONG LengthTransferred
) {
#ifdef DEBUG_LOG_EX
	log->writeString("WinUsb_GetDescriptor(0x")->writeBinaryBuffer(&InterfaceHandle, sizeof(WINUSB_INTERFACE_HANDLE))
		->writeString(", 0x")->writeBinaryBuffer(&DescriptorType, 1)->writeString(", 0x")->writeBinaryBuffer(&Index, 1)
		->writeString(", 0x")->writeBinaryBuffer(&LanguageID, 2)->writeString(", [OutBuffer], ")->writeLong(BufferLength)->writeString(", [OutLen])\r\n");
#endif
	WINUSB_GET_DESCRIPTOR func = (WINUSB_GET_DESCRIPTOR) GetProcAddress(hWinUsbLib, "WinUsb_GetDescriptor");
	if (func(InterfaceHandle, DescriptorType, Index, LanguageID, Buffer, BufferLength, LengthTransferred)) {
#ifdef DEBUG_LOG_EX
		log->writeString(" => Success! Length transferred: ")->writeLong(*LengthTransferred)->writeString("\r\n");
		log->writeString("    Buffer content: 0x")->writeBinaryBuffer(Buffer, *LengthTransferred)->writeString("\r\n");
#endif
		return true;
	} else {
#ifdef DEBUG_LOG_EX
		DWORD dwError = GetLastError();
		log->writeString(" => Failed! Error code: 0x")->writeBinaryBuffer(&dwError, sizeof(DWORD))->writeString("\r\n");
#endif
		return false;
	}
}

BOOL __stdcall WinUsb_Wrapper_GetOverlappedResult(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		LPOVERLAPPED lpOverlapped,
		LPDWORD lpNumberOfBytesTransferred,
		BOOL bWait
) {
	WINUSB_GET_OVERLAPPED_RESULT func = (WINUSB_GET_OVERLAPPED_RESULT) GetProcAddress(hWinUsbLib, "WinUsb_GetOverlappedResult");
	return func(InterfaceHandle, lpOverlapped, lpNumberOfBytesTransferred, bWait);
}

BOOL __stdcall WinUsb_Wrapper_GetPipePolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID,
		ULONG PolicyType,
		PULONG ValueLength,
		PVOID Value
) {
	WINUSB_GET_PIPE_POLICY func = (WINUSB_GET_PIPE_POLICY) GetProcAddress(hWinUsbLib, "WinUsb_GetPipePolicy");
	return func(InterfaceHandle, PipeID, PolicyType, ValueLength, Value);
}

BOOL __stdcall WinUsb_Wrapper_GetPowerPolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		ULONG PolicyType,
		PULONG ValueLength,
		PVOID Value
) {
	WINUSB_GET_POWER_POLICY func = (WINUSB_GET_POWER_POLICY) GetProcAddress(hWinUsbLib, "WinUsb_GetPowerPolicy");
	return func(InterfaceHandle, PolicyType, ValueLength, Value);
}

BOOL __stdcall WinUsb_Wrapper_Initialize(
		HANDLE DeviceHandle,
		PWINUSB_INTERFACE_HANDLE InterfaceHandle
) {
#ifdef DEBUG_LOG_EX
	log->writeString("WinUsb_Initialize(0x")->writeBinaryBuffer(&DeviceHandle, sizeof(HANDLE))->writeString(")\r\n");
#endif
	WINUSB_INITIALIZE func = (WINUSB_INITIALIZE) GetProcAddress(hWinUsbLib, "WinUsb_Initialize");
	if (func(DeviceHandle, InterfaceHandle)) {
#ifdef DEBUG_LOG_EX
		log->writeString(" => Success! InterfaceHandle: 0x")->writeBinaryBuffer(InterfaceHandle, sizeof(PWINUSB_INTERFACE_HANDLE))->writeString("\r\n");
#endif
		Core::PrinterSettings::getInstance()->setUsbHandle(*InterfaceHandle);
		return true;
	} else {
#ifdef DEBUG_LOG_EX
		DWORD dwError = GetLastError();
		log->writeString(" => Failed! Error code: 0x")->writeBinaryBuffer(&dwError, sizeof(DWORD))->writeString("\r\n");
#endif
		return false;
	}
}

PUSB_INTERFACE_DESCRIPTOR __stdcall WinUsb_Wrapper_ParseConfigurationDescriptor(
		PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
		PVOID StartPosition,
		LONG InterfaceNumber,
		LONG AlternateSetting,
		LONG InterfaceClass,
		LONG InterfaceSubClass,
		LONG InterfaceProtocol
) {
	WINUSB_PARSE_CONFIGURATION_DESCRIPTOR func = (WINUSB_PARSE_CONFIGURATION_DESCRIPTOR) GetProcAddress(hWinUsbLib, "WinUsb_ParseConfigurationDescriptor");
	return func(ConfigurationDescriptor, StartPosition, InterfaceNumber, AlternateSetting, InterfaceClass, InterfaceSubClass, InterfaceProtocol);
}

PUSB_COMMON_DESCRIPTOR __stdcall WinUsb_Wrapper_ParseDescriptors(
		PVOID DescriptorBuffer,
		ULONG TotalLength,
		PVOID StartPosition,
		LONG DescriptorType
) {
	WINUSB_PARSE_DESCRIPTORS func = (WINUSB_PARSE_DESCRIPTORS) GetProcAddress(hWinUsbLib, "WinUsb_ParseDescriptors");
	return func(DescriptorBuffer, TotalLength, StartPosition, DescriptorType);
}

BOOL __stdcall WinUsb_Wrapper_QueryDeviceInformation(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		ULONG InformationType,
		PULONG BufferLength,
		PVOID Buffer
) {
#ifdef DEBUG_LOG_EX
	log->writeString("WinUsb_QueryDeviceInformation(0x")->writeBinaryBuffer(&InterfaceHandle, sizeof(WINUSB_INTERFACE_HANDLE))
		->writeString(", 0x")->writeBinaryBuffer(&InformationType, 4)->writeString(", ")->writeLong(*BufferLength)->writeString(", [OutBuffer])\r\n");
#endif
	WINUSB_QUERY_DEVICE_INFORMATION func = (WINUSB_QUERY_DEVICE_INFORMATION) GetProcAddress(hWinUsbLib, "WinUsb_QueryDeviceInformation");
	if (func(InterfaceHandle, InformationType, BufferLength, Buffer)) {
#ifdef DEBUG_LOG_EX
		log->writeString(" => Success! Length transferred: ")->writeLong(*BufferLength)->writeString("\r\n");
		log->writeString("    Buffer content: 0x")->writeBinaryBuffer(Buffer, *BufferLength)->writeString("\r\n");
#endif
		return true;
	} else {
#ifdef DEBUG_LOG_EX
		DWORD dwError = GetLastError();
		log->writeString(" => Failed! Error code: 0x")->writeBinaryBuffer(&dwError, sizeof(DWORD))->writeString("\r\n");
#endif
		return false;
	}
}

BOOL __stdcall WinUsb_Wrapper_QueryInterfaceSettings(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AlternateSettingNumber,
		PUSB_INTERFACE_DESCRIPTOR UsbAltInterfaceDescriptor
) {
	WINUSB_QUERY_INTERFACE_SETTINGS func = (WINUSB_QUERY_INTERFACE_SETTINGS) GetProcAddress(hWinUsbLib, "WinUsb_QueryInterfaceSettings");
	return func(InterfaceHandle, AlternateSettingNumber, UsbAltInterfaceDescriptor);
}

BOOL __stdcall WinUsb_Wrapper_QueryPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AlternateInterfaceNumber,
		UCHAR PipeIndex,
		PWINUSB_PIPE_INFORMATION PipeInformation
) {
#ifdef DEBUG_LOG_EX
	log->writeString("WinUsb_QueryPipe(0x")->writeBinaryBuffer(&InterfaceHandle, sizeof(WINUSB_INTERFACE_HANDLE))
		->writeString(", 0x")->writeBinaryBuffer(&AlternateInterfaceNumber, 1)->writeString(", 0x")->writeBinaryBuffer(&PipeIndex, 1)->writeString(", [OutBuffer])\r\n");
#endif
	WINUSB_QUERY_PIPE func = (WINUSB_QUERY_PIPE) GetProcAddress(hWinUsbLib, "WinUsb_QueryPipe");
	if (func(InterfaceHandle, AlternateInterfaceNumber, PipeIndex, PipeInformation)) {
#ifdef DEBUG_LOG_EX
		log->writeString(" => Success! Pipe information: ")->writeBinaryBuffer(PipeInformation, sizeof(WINUSB_PIPE_INFORMATION))->writeString("\r\n");
#endif
		return true;
	} else {
#ifdef DEBUG_LOG_EX
		DWORD dwError = GetLastError();
		log->writeString(" => Failed! Error code: 0x")->writeBinaryBuffer(&dwError, sizeof(DWORD))->writeString("\r\n");
#endif
		return false;
	}
}

BOOL __stdcall WinUsb_Wrapper_ReadPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID,
		PUCHAR Buffer,
		ULONG BufferLength,
		PULONG LengthTransferred,
		LPOVERLAPPED Overlapped
) {
#ifdef DEBUG_LOG_EX
	log->writeString("WinUsb_ReadPipe(0x")->writeBinaryBuffer(&InterfaceHandle, sizeof(WINUSB_INTERFACE_HANDLE))
		->writeString(", 0x")->writeBinaryBuffer(&PipeID, 1)->writeString(", [OutBuffer], 0x")->writeLong(BufferLength)
		->writeString(", [OutLen], 0x")->writeBinaryBuffer(&Overlapped, 4)->writeString(")\r\n");
#endif
	WINUSB_READ_PIPE func = (WINUSB_READ_PIPE) GetProcAddress(hWinUsbLib, "WinUsb_ReadPipe");
	if (func(InterfaceHandle, PipeID, Buffer, BufferLength, LengthTransferred, Overlapped)) {
#ifdef DEBUG_LOG_EX
		log->writeString(" => Success! Length transferred: ")->writeLong(*LengthTransferred)->writeString("\r\n");
		log->writeString("    Buffer content: 0x")->writeBinaryBuffer(Buffer, *LengthTransferred)->writeString("\r\n");
#endif
#ifdef DEBUG_LOG
		logRaw->writeString("<")->writeBinaryBuffer(Buffer, *LengthTransferred)->writeString("\r\n");
#endif
		printerIntercept->handleUsbRead(InterfaceHandle, PipeID, Buffer, *LengthTransferred);
		return true;
	} else {
#ifdef DEBUG_LOG_EX
		DWORD dwError = GetLastError();
		log->writeString(" => Failed! Error code: 0x")->writeBinaryBuffer(&dwError, sizeof(DWORD))->writeString("\r\n");
#endif
		return false;
	}
}

BOOL __stdcall WinUsb_Wrapper_ResetPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID
) {
	WINUSB_RESET_PIPE func = (WINUSB_RESET_PIPE) GetProcAddress(hWinUsbLib, "WinUsb_ResetPipe");
	return func(InterfaceHandle, PipeID);
}

BOOL __stdcall WinUsb_Wrapper_SetCurrentAlternateSetting(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AlternateSetting
) {
	WINUSB_SET_CURRENT_ALTERNATE_SETTING func = (WINUSB_SET_CURRENT_ALTERNATE_SETTING) GetProcAddress(hWinUsbLib, "WinUsb_SetCurrentAlternateSetting");
	return func(InterfaceHandle, AlternateSetting);
}

BOOL __stdcall WinUsb_Wrapper_SetPipePolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID,
		ULONG PolicyType,
		ULONG ValueLength,
		PVOID Value
) {
	WINUSB_SET_PIPE_POLICY func = (WINUSB_SET_PIPE_POLICY) GetProcAddress(hWinUsbLib, "WinUsb_SetPipePolicy");
	return func(InterfaceHandle, PipeID, PolicyType, ValueLength, Value);
}

BOOL __stdcall WinUsb_Wrapper_SetPowerPolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		ULONG PolicyType,
		ULONG ValueLength,
		PVOID Value
) {
	WINUSB_SET_POWER_POLICY func = (WINUSB_SET_POWER_POLICY) GetProcAddress(hWinUsbLib, "WinUsb_SetPowerPolicy");
	return func(InterfaceHandle, PolicyType, ValueLength, Value);
}

BOOL __stdcall WinUsb_Wrapper_WritePipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID,
		PUCHAR Buffer,
		ULONG BufferLength,
		PULONG LengthTransferred,
		LPOVERLAPPED Overlapped
) {
	printerIntercept->handleUsbWrite(InterfaceHandle, PipeID, Buffer, BufferLength);
	WINUSB_WRITE_PIPE func = (WINUSB_WRITE_PIPE) GetProcAddress(hWinUsbLib, "WinUsb_WritePipe");
	if (func(InterfaceHandle, PipeID, Buffer, BufferLength, LengthTransferred, Overlapped)) {
#ifdef DEBUG_LOG_EX
		log->writeString(" => Success! Length transferred: ")->writeLong(*LengthTransferred)->writeString("\r\n");
		//log->writeString("    Buffer sent: 0x")->writeBinaryBuffer(Buffer, *LengthTransferred)->writeString("\r\n");
#endif
		return true;
	} else {
#ifdef DEBUG_LOG_EX
		DWORD dwError = GetLastError();
		log->writeString(" => Failed! Error code: 0x")->writeBinaryBuffer(&dwError, sizeof(DWORD))->writeString("\r\n");
#endif
		return false;
	}
}

