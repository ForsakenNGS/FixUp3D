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
#include <stdio.h>
#include <iostream>
#include <direct.h>
#include <Shlobj.h>
#include <tchar.h>

WINUSB_INTERFACE_HANDLE hLatestInterfaceHandle = NULL;

Core::SimpleLogWriter*	log = NULL;
Core::SimpleLogWriter*	logRaw = NULL;
ULONG					debugDumpIndex = 0;

BOOL InitWinUsbWrapper()
{
#ifdef DEBUG_LOG
	if (log == NULL) {
		TCHAR sHomeDir[MAX_PATH];
		TCHAR sFilename[MAX_PATH];
		if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, sHomeDir))) {
			_stprintf(sFilename, TEXT("%s\\UpUsbIntercept"), sHomeDir);
			// Ensure directory exists
			_mkdir(sFilename);
			// Detailed debug.log
			_stprintf(sFilename, TEXT("%s\\UpUsbIntercept\\debug.log"), sHomeDir);
			log = new Core::SimpleLogWriter(sFilename);
			// Raw read/write data
			_stprintf(sFilename, TEXT("%s\\UpUsbIntercept\\debug_raw.log"), sHomeDir);
			logRaw = new Core::SimpleLogWriter(sFilename);
		}
	}
#endif

	// Load original winusb lib
	if( !DynLoadSystemWinUSB() )
		return FALSE;

#ifdef DEBUG_LOG_EX
		log->writeString("Loaded original 'winusb.dll'.\r\n");
#endif

	return TRUE;
}

BOOL __stdcall WinUsb_Wrapper_AbortPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID
)
{
	return WinUsb_AbortPipe( InterfaceHandle, PipeID );
}

BOOL __stdcall WinUsb_Wrapper_ControlTransfer(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		WINUSB_SETUP_PACKET SetupPacket,
		PUCHAR Buffer,
		ULONG BufferLength,
		PULONG LengthTransferred,
		LPOVERLAPPED Overlapped
)
{
	return WinUsb_ControlTransfer( InterfaceHandle, SetupPacket, Buffer, BufferLength, LengthTransferred, Overlapped );
}

BOOL __stdcall WinUsb_Wrapper_FlushPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID
)
{
	return WinUsb_FlushPipe( InterfaceHandle, PipeID );
}

BOOL __stdcall WinUsb_Wrapper_Free(
		WINUSB_INTERFACE_HANDLE InterfaceHandle
)
{
	return WinUsb_Free( InterfaceHandle );
}

BOOL __stdcall WinUsb_Wrapper_GetAssociatedInterface(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AssociatedInterfaceIndex,
		PWINUSB_INTERFACE_HANDLE AssociatedInterfaceHandle
)
{
	return WinUsb_GetAssociatedInterface( InterfaceHandle, AssociatedInterfaceIndex, AssociatedInterfaceHandle );
}

BOOL __stdcall WinUsb_Wrapper_GetCurrentAlternateSetting(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		PUCHAR AlternateSetting
)
{
	return WinUsb_GetCurrentAlternateSetting( InterfaceHandle, AlternateSetting );
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

	if( WinUsb_GetDescriptor( InterfaceHandle, DescriptorType, Index, LanguageID, Buffer, BufferLength, LengthTransferred ) )
	{
#ifdef DEBUG_LOG_EX
		log->writeString(" => Success! Length transferred: ")->writeLong(*LengthTransferred)->writeString("\r\n");
		log->writeString("    Buffer content: 0x")->writeBinaryBuffer(Buffer, *LengthTransferred)->writeString("\r\n");
#endif
		return TRUE;
	}
	else
	{
#ifdef DEBUG_LOG_EX
		DWORD dwError = GetLastError();
		log->writeString(" => Failed! Error code: 0x")->writeBinaryBuffer(&dwError, sizeof(DWORD))->writeString("\r\n");
#endif
		return FALSE;
	}
}

BOOL __stdcall WinUsb_Wrapper_GetOverlappedResult(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		LPOVERLAPPED lpOverlapped,
		LPDWORD lpNumberOfBytesTransferred,
		BOOL bWait
)
{
	return WinUsb_GetOverlappedResult( InterfaceHandle, lpOverlapped, lpNumberOfBytesTransferred, bWait );
}

BOOL __stdcall WinUsb_Wrapper_GetPipePolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID,
		ULONG PolicyType,
		PULONG ValueLength,
		PVOID Value
)
{
	return WinUsb_GetPipePolicy( InterfaceHandle, PipeID, PolicyType, ValueLength, Value );
}

BOOL __stdcall WinUsb_Wrapper_GetPowerPolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		ULONG PolicyType,
		PULONG ValueLength,
		PVOID Value
)
{
	return WinUsb_GetPowerPolicy( InterfaceHandle, PolicyType, ValueLength, Value );
}

BOOL __stdcall WinUsb_Wrapper_Initialize(
		HANDLE DeviceHandle,
		PWINUSB_INTERFACE_HANDLE InterfaceHandle
)
{
#ifdef DEBUG_LOG_EX
	log->writeString("WinUsb_Initialize(0x")->writeBinaryBuffer(&DeviceHandle, sizeof(HANDLE))->writeString(")\r\n");
#endif
	if( WinUsb_Initialize( DeviceHandle, InterfaceHandle ) )
	{
#ifdef DEBUG_LOG_EX
		log->writeString(" => Success! InterfaceHandle: 0x")->writeBinaryBuffer(InterfaceHandle, sizeof(PWINUSB_INTERFACE_HANDLE))->writeString("\r\n");
#endif
		Core::PrinterSettings::getInstance()->setUsbHandle(*InterfaceHandle);
		return TRUE;
	}
	else
	{
#ifdef DEBUG_LOG_EX
		DWORD dwError = GetLastError();
		log->writeString(" => Failed! Error code: 0x")->writeBinaryBuffer(&dwError, sizeof(DWORD))->writeString("\r\n");
#endif
		return FALSE;
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
)
{
	return WinUsb_ParseConfigurationDescriptor( ConfigurationDescriptor, StartPosition, InterfaceNumber, AlternateSetting, InterfaceClass, InterfaceSubClass, InterfaceProtocol );
}

PUSB_COMMON_DESCRIPTOR __stdcall WinUsb_Wrapper_ParseDescriptors(
		PVOID DescriptorBuffer,
		ULONG TotalLength,
		PVOID StartPosition,
		LONG DescriptorType
)
{
	return WinUsb_ParseDescriptors( DescriptorBuffer, TotalLength, StartPosition, DescriptorType );
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
	if( WinUsb_QueryDeviceInformation( InterfaceHandle, InformationType, BufferLength, Buffer ) )
	{
#ifdef DEBUG_LOG_EX
		log->writeString(" => Success! Length transferred: ")->writeLong(*BufferLength)->writeString("\r\n");
		log->writeString("    Buffer content: 0x")->writeBinaryBuffer(Buffer, *BufferLength)->writeString("\r\n");
#endif
		return TRUE;
	}
	else
	{
#ifdef DEBUG_LOG_EX
		DWORD dwError = GetLastError();
		log->writeString(" => Failed! Error code: 0x")->writeBinaryBuffer(&dwError, sizeof(DWORD))->writeString("\r\n");
#endif
		return FALSE;
	}
}

BOOL __stdcall WinUsb_Wrapper_QueryInterfaceSettings(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AlternateSettingNumber,
		PUSB_INTERFACE_DESCRIPTOR UsbAltInterfaceDescriptor
)
{
	return WinUsb_QueryInterfaceSettings( InterfaceHandle, AlternateSettingNumber, UsbAltInterfaceDescriptor );
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
	if( WinUsb_QueryPipe( InterfaceHandle, AlternateInterfaceNumber, PipeIndex, PipeInformation ) )
	{
#ifdef DEBUG_LOG_EX
		log->writeString(" => Success! Pipe information: ")->writeBinaryBuffer(PipeInformation, sizeof(WINUSB_PIPE_INFORMATION))->writeString("\r\n");
#endif
		return TRUE;
	}
	else
	{
#ifdef DEBUG_LOG_EX
		DWORD dwError = GetLastError();
		log->writeString(" => Failed! Error code: 0x")->writeBinaryBuffer(&dwError, sizeof(DWORD))->writeString("\r\n");
#endif
		return FALSE;
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
#ifdef DEBUG_LOG
	log->writeString("WinUsb_ReadPipe(0x")->writeBinaryBuffer(&InterfaceHandle, sizeof(WINUSB_INTERFACE_HANDLE))
		->writeString(", 0x")->writeBinaryBuffer(&PipeID, 1)->writeString(", [OutBuffer], 0x")->writeLong(BufferLength)
		->writeString(", [OutLen], 0x")->writeBinaryBuffer(&Overlapped, 4)->writeString(")\r\n");
#endif
	if( WinUsb_ReadPipe( InterfaceHandle, PipeID, Buffer, BufferLength, LengthTransferred, Overlapped ) )
	{
#ifdef DEBUG_LOG_EX
		log->writeString(" => Success! Length transferred: ")->writeLong(*LengthTransferred)->writeString("\r\n");
		log->writeString("    Buffer content: 0x")->writeBinaryBuffer(Buffer, *LengthTransferred)->writeString("\r\n");
#endif
#ifdef DEBUG_LOG
		logRaw->writeString("<")->writeBinaryBuffer(Buffer, *LengthTransferred)->writeString("\r\n");
#endif
		Core::PrinterIntercept::getInstance()->handleUsbRead(InterfaceHandle, PipeID, Buffer, *LengthTransferred);
		return TRUE;
	}
	else
	{
#ifdef DEBUG_LOG_EX
		DWORD dwError = GetLastError();
		log->writeString(" => Failed! Error code: 0x")->writeBinaryBuffer(&dwError, sizeof(DWORD))->writeString("\r\n");
#endif
		return FALSE;
	}
}

BOOL __stdcall WinUsb_Wrapper_ResetPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID
)
{
	return WinUsb_ResetPipe( InterfaceHandle, PipeID );
}

BOOL __stdcall WinUsb_Wrapper_SetCurrentAlternateSetting(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AlternateSetting
)
{
	return WinUsb_SetCurrentAlternateSetting( InterfaceHandle, AlternateSetting );
}

BOOL __stdcall WinUsb_Wrapper_SetPipePolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID,
		ULONG PolicyType,
		ULONG ValueLength,
		PVOID Value
)
{
	return WinUsb_SetPipePolicy( InterfaceHandle, PipeID, PolicyType, ValueLength, Value );
}

BOOL __stdcall WinUsb_Wrapper_SetPowerPolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		ULONG PolicyType,
		ULONG ValueLength,
		PVOID Value
)
{
	return WinUsb_SetPowerPolicy( InterfaceHandle, PolicyType, ValueLength, Value );
}

BOOL __stdcall WinUsb_Wrapper_WritePipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID,
		PUCHAR Buffer,
		ULONG BufferLength,
		PULONG LengthTransferred,
		LPOVERLAPPED Overlapped
) {
#ifdef DEBUG_LOG
	log->writeString("WinUsb_WritePipe(0x")->writeBinaryBuffer(&InterfaceHandle, sizeof(WINUSB_INTERFACE_HANDLE))
		->writeString(", 0x")->writeBinaryBuffer(&PipeID, 1)->writeString(", [OutBuffer], 0x")->writeLong(BufferLength)
		->writeString(", [OutLen], 0x")->writeBinaryBuffer(&Overlapped, 4)->writeString(")\r\n");
#endif
	Core::PrinterIntercept::getInstance()->handleUsbWrite(InterfaceHandle, PipeID, Buffer, BufferLength);
	if( WinUsb_WritePipe( InterfaceHandle, PipeID, Buffer, BufferLength, LengthTransferred, Overlapped ) )
	{
#ifdef DEBUG_LOG_EX
		log->writeString(" => Success! Length transferred: ")->writeLong(*LengthTransferred)->writeString("\r\n");
		//log->writeString("    Buffer sent: 0x")->writeBinaryBuffer(Buffer, *LengthTransferred)->writeString("\r\n");
#endif
		return true;
	}
	else
	{
#ifdef DEBUG_LOG_EX
		DWORD dwError = GetLastError();
		log->writeString(" => Failed! Error code: 0x")->writeBinaryBuffer(&dwError, sizeof(DWORD))->writeString("\r\n");
#endif
		return false;
	}
}

