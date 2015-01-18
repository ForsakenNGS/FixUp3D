/*
 * dll_funcs.h
 *
 *  Created on: 13.01.2015
 *      Author: Forsaken
 */

#ifndef DLL_FUNCS_H_
#define DLL_FUNCS_H_

#include "winusbhelper.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL InitWinUsbWrapper();

BOOL __stdcall WinUsb_Wrapper_AbortPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID
);

BOOL __stdcall WinUsb_Wrapper_ControlTransfer(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		WINUSB_SETUP_PACKET SetupPacket,
		PUCHAR Buffer,
		ULONG BufferLength,
		PULONG LengthTransferred,
		LPOVERLAPPED Overlapped
);

BOOL __stdcall WinUsb_Wrapper_FlushPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID
);

BOOL __stdcall WinUsb_Wrapper_Free(
		WINUSB_INTERFACE_HANDLE InterfaceHandle
);

BOOL __stdcall WinUsb_Wrapper_GetAssociatedInterface(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AssociatedInterfaceIndex,
		PWINUSB_INTERFACE_HANDLE AssociatedInterfaceHandle
);

BOOL __stdcall WinUsb_Wrapper_GetCurrentAlternateSetting(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		PUCHAR AlternateSetting
);

BOOL __stdcall WinUsb_Wrapper_GetDescriptor(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR DescriptorType,
		UCHAR Index,
		USHORT LanguageID,
		PUCHAR Buffer,
		ULONG BufferLength,
		PULONG LengthTransferred
);

BOOL __stdcall WinUsb_Wrapper_GetOverlappedResult(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		LPOVERLAPPED lpOverlapped,
		LPDWORD lpNumberOfBytesTransferred,
		BOOL bWait
);

BOOL __stdcall WinUsb_Wrapper_GetPipePolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID,
		ULONG PolicyType,
		PULONG ValueLength,
		PVOID Value
);

BOOL __stdcall WinUsb_Wrapper_GetPowerPolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		ULONG PolicyType,
		PULONG ValueLength,
		PVOID Value
);

BOOL __stdcall WinUsb_Wrapper_Initialize(
		HANDLE DeviceHandle,
		PWINUSB_INTERFACE_HANDLE InterfaceHandle
);

PUSB_INTERFACE_DESCRIPTOR __stdcall WinUsb_Wrapper_ParseConfigurationDescriptor(
		PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
		PVOID StartPosition,
		LONG InterfaceNumber,
		LONG AlternateSetting,
		LONG InterfaceClass,
		LONG InterfaceSubClass,
		LONG InterfaceProtocol
);

PUSB_COMMON_DESCRIPTOR __stdcall WinUsb_Wrapper_ParseDescriptors(
		PVOID DescriptorBuffer,
		ULONG TotalLength,
		PVOID StartPosition,
		LONG DescriptorType
);

BOOL __stdcall WinUsb_Wrapper_QueryDeviceInformation(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		ULONG InformationType,
		PULONG BufferLength,
		PVOID Buffer
);

BOOL __stdcall WinUsb_Wrapper_QueryInterfaceSettings(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AlternateSettingNumber,
		PUSB_INTERFACE_DESCRIPTOR UsbAltInterfaceDescriptor
);

BOOL __stdcall WinUsb_Wrapper_QueryPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AlternateInterfaceNumber,
		UCHAR PipeIndex,
		PWINUSB_PIPE_INFORMATION PipeInformation
);

BOOL __stdcall WinUsb_Wrapper_ReadPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID,
		PUCHAR Buffer,
		ULONG BufferLength,
		PULONG LengthTransferred,
		LPOVERLAPPED Overlapped
);

BOOL __stdcall WinUsb_Wrapper_ResetPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID
);

BOOL __stdcall WinUsb_Wrapper_SetCurrentAlternateSetting(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AlternateSetting
);

BOOL __stdcall WinUsb_Wrapper_SetPipePolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID,
		ULONG PolicyType,
		ULONG ValueLength,
		PVOID Value
);

BOOL __stdcall WinUsb_Wrapper_SetPowerPolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		ULONG PolicyType,
		ULONG ValueLength,
		PVOID Value
);

BOOL __stdcall WinUsb_Wrapper_WritePipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID,
		PUCHAR Buffer,
		ULONG BufferLength,
		PULONG LengthTransferred,
		LPOVERLAPPED Overlapped
);

#ifdef __cplusplus
}
#endif

#endif /* DLL_FUNCS_H_ */
