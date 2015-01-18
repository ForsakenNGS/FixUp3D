/*
 * winusbhelper.cpp
 *
 *  Created on: 18.01.2015
 *      Author: stohn
 */

#include "winusbhelper.h"

WINUSB_ABORT_PIPE WinUsb_AbortPipe;
WINUSB_CONTROL_TRANSFER WinUsb_ControlTransfer;
WINUSB_FLUSH_PIPE WinUsb_FlushPipe;
WINUSB_FREE WinUsb_Free;
WINUSB_GET_ASSOCIATED_INTERFACE WinUsb_GetAssociatedInterface;
WINUSB_GET_CURRENT_ALTERNATE_SETTING WinUsb_GetCurrentAlternateSetting;
WINUSB_GET_DESCRIPTOR WinUsb_GetDescriptor;
WINUSB_GET_OVERLAPPED_RESULT WinUsb_GetOverlappedResult;
WINUSB_GET_PIPE_POLICY WinUsb_GetPipePolicy;
WINUSB_GET_POWER_POLICY WinUsb_GetPowerPolicy;
WINUSB_INITIALIZE WinUsb_Initialize;
WINUSB_PARSE_CONFIGURATION_DESCRIPTOR WinUsb_ParseConfigurationDescriptor;
WINUSB_PARSE_DESCRIPTORS WinUsb_ParseDescriptors;
WINUSB_QUERY_DEVICE_INFORMATION WinUsb_QueryDeviceInformation;
WINUSB_QUERY_INTERFACE_SETTINGS WinUsb_QueryInterfaceSettings;
WINUSB_QUERY_PIPE WinUsb_QueryPipe;
WINUSB_READ_PIPE WinUsb_ReadPipe;
WINUSB_RESET_PIPE WinUsb_ResetPipe;
WINUSB_SET_CURRENT_ALTERNATE_SETTING WinUsb_SetCurrentAlternateSetting;
WINUSB_SET_PIPE_POLICY WinUsb_SetPipePolicy;
WINUSB_SET_POWER_POLICY WinUsb_SetPowerPolicy;
WINUSB_WRITE_PIPE WinUsb_WritePipe;

static HMODULE _hWinUsbDll = NULL;

BOOL DynLoadSystemWinUSB()
{
	if( !_hWinUsbDll )
	{
		// Load original winusb lib
		_hWinUsbDll = LoadLibrary(TEXT("C:\\Windows\\System32\\winusb.dll"));

		WinUsb_AbortPipe                    = (WINUSB_ABORT_PIPE)                     GetProcAddress(_hWinUsbDll, TEXT("WinUsb_AbortPipe"));
		WinUsb_ControlTransfer              = (WINUSB_CONTROL_TRANSFER)               GetProcAddress(_hWinUsbDll, TEXT("WinUsb_ControlTransfer"));
		WinUsb_FlushPipe                    = (WINUSB_FLUSH_PIPE)                     GetProcAddress(_hWinUsbDll, TEXT("WinUsb_FlushPipe"));
		WinUsb_Free                         = (WINUSB_FREE)                           GetProcAddress(_hWinUsbDll, TEXT("WinUsb_Free"));
		WinUsb_GetAssociatedInterface       = (WINUSB_GET_ASSOCIATED_INTERFACE)       GetProcAddress(_hWinUsbDll, TEXT("WinUsb_GetAssociatedInterface"));
		WinUsb_GetCurrentAlternateSetting   = (WINUSB_GET_CURRENT_ALTERNATE_SETTING)  GetProcAddress(_hWinUsbDll, TEXT("WinUsb_GetCurrentAlternateSetting"));
		WinUsb_GetDescriptor                = (WINUSB_GET_DESCRIPTOR)                 GetProcAddress(_hWinUsbDll, TEXT("WinUsb_GetDescriptor"));
		WinUsb_GetOverlappedResult          = (WINUSB_GET_OVERLAPPED_RESULT)          GetProcAddress(_hWinUsbDll, TEXT("WinUsb_GetOverlappedResult"));
		WinUsb_GetPipePolicy                = (WINUSB_GET_PIPE_POLICY)                GetProcAddress(_hWinUsbDll, TEXT("WinUsb_GetPipePolicy"));
		WinUsb_GetPowerPolicy               = (WINUSB_GET_POWER_POLICY)               GetProcAddress(_hWinUsbDll, TEXT("WinUsb_GetPowerPolicy"));
		WinUsb_Initialize                   = (WINUSB_INITIALIZE)                     GetProcAddress(_hWinUsbDll, TEXT("WinUsb_Initialize"));
		WinUsb_ParseConfigurationDescriptor = (WINUSB_PARSE_CONFIGURATION_DESCRIPTOR) GetProcAddress(_hWinUsbDll, TEXT("WinUsb_ParseConfigurationDescriptor"));
		WinUsb_ParseDescriptors             = (WINUSB_PARSE_DESCRIPTORS)              GetProcAddress(_hWinUsbDll, TEXT("WinUsb_ParseDescriptors"));
		WinUsb_QueryDeviceInformation       = (WINUSB_QUERY_DEVICE_INFORMATION)       GetProcAddress(_hWinUsbDll, TEXT("WinUsb_QueryDeviceInformation"));
		WinUsb_QueryInterfaceSettings       = (WINUSB_QUERY_INTERFACE_SETTINGS)       GetProcAddress(_hWinUsbDll, TEXT("WinUsb_QueryInterfaceSettings"));
		WinUsb_QueryPipe                    = (WINUSB_QUERY_PIPE)                     GetProcAddress(_hWinUsbDll, TEXT("WinUsb_QueryPipe"));
		WinUsb_ReadPipe                     = (WINUSB_READ_PIPE)                      GetProcAddress(_hWinUsbDll, TEXT("WinUsb_ReadPipe"));
		WinUsb_ResetPipe                    = (WINUSB_RESET_PIPE)                     GetProcAddress(_hWinUsbDll, TEXT("WinUsb_ResetPipe"));
		WinUsb_SetCurrentAlternateSetting   = (WINUSB_SET_CURRENT_ALTERNATE_SETTING)  GetProcAddress(_hWinUsbDll, TEXT("WinUsb_SetCurrentAlternateSetting"));
		WinUsb_SetPipePolicy                = (WINUSB_SET_PIPE_POLICY)                GetProcAddress(_hWinUsbDll, TEXT("WinUsb_SetPipePolicy"));
		WinUsb_SetPowerPolicy               = (WINUSB_SET_POWER_POLICY)               GetProcAddress(_hWinUsbDll, TEXT("WinUsb_SetPowerPolicy"));
		WinUsb_WritePipe                    = (WINUSB_WRITE_PIPE)                     GetProcAddress(_hWinUsbDll, TEXT("WinUsb_WritePipe"));

		if( !WinUsb_AbortPipe || !WinUsb_ControlTransfer || !WinUsb_FlushPipe || !WinUsb_Free ||
			!WinUsb_GetAssociatedInterface || !WinUsb_GetCurrentAlternateSetting || !WinUsb_GetDescriptor ||
			!WinUsb_GetOverlappedResult || !WinUsb_GetPipePolicy || !WinUsb_GetPowerPolicy ||
			!WinUsb_Initialize || !WinUsb_ParseConfigurationDescriptor || !WinUsb_ParseDescriptors ||
			!WinUsb_QueryDeviceInformation || !WinUsb_QueryInterfaceSettings || !WinUsb_QueryPipe ||
			!WinUsb_ReadPipe || !WinUsb_ResetPipe || !WinUsb_SetCurrentAlternateSetting ||
			!WinUsb_SetPipePolicy || !WinUsb_SetPowerPolicy || !WinUsb_WritePipe
		  )
		{
			return FALSE;
		}

	}

	return TRUE;
}

BOOL DynUnloadWinUSB()
{
	if( _hWinUsbDll )
		FreeLibrary( _hWinUsbDll );

	_hWinUsbDll = NULL;

	return TRUE;
}
