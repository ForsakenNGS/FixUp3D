/*
 * dll_funcs.h
 *
 *  Created on: 13.01.2015
 *      Author: Forsaken
 */

#ifndef DLL_FUNCS_H_
#define DLL_FUNCS_H_

#include "Winusb.h"

#ifdef __cplusplus
extern "C" {
#endif

void initWinUsbWrapper();

typedef BOOL (WINAPI *WINUSB_ABORT_PIPE)(WINUSB_INTERFACE_HANDLE,UCHAR);
BOOL __stdcall WinUsb_Wrapper_AbortPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID
);

typedef BOOL (WINAPI *WINUSB_CONTROL_TRANSFER)(WINUSB_INTERFACE_HANDLE,WINUSB_SETUP_PACKET,PUCHAR,ULONG,PULONG,LPOVERLAPPED);
BOOL __stdcall WinUsb_Wrapper_ControlTransfer(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		WINUSB_SETUP_PACKET SetupPacket,
		PUCHAR Buffer,
		ULONG BufferLength,
		PULONG LengthTransferred,
		LPOVERLAPPED Overlapped
);

typedef BOOL (WINAPI *WINUSB_FLUSH_PIPE)(WINUSB_INTERFACE_HANDLE,UCHAR);
BOOL __stdcall WinUsb_Wrapper_FlushPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID
);

typedef BOOL (WINAPI *WINUSB_FREE)(WINUSB_INTERFACE_HANDLE);
BOOL __stdcall WinUsb_Wrapper_Free(
		WINUSB_INTERFACE_HANDLE InterfaceHandle
);

typedef BOOL (WINAPI *WINUSB_GET_ASSOCIATED_INTERFACE)(WINUSB_INTERFACE_HANDLE,UCHAR,PWINUSB_INTERFACE_HANDLE);
BOOL __stdcall WinUsb_Wrapper_GetAssociatedInterface(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AssociatedInterfaceIndex,
		PWINUSB_INTERFACE_HANDLE AssociatedInterfaceHandle
);

typedef BOOL (WINAPI *WINUSB_GET_CURRENT_ALTERNATE_SETTING)(WINUSB_INTERFACE_HANDLE,PUCHAR);
BOOL __stdcall WinUsb_Wrapper_GetCurrentAlternateSetting(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		PUCHAR AlternateSetting
);

typedef BOOL (WINAPI *WINUSB_GET_DESCRIPTOR)(WINUSB_INTERFACE_HANDLE,UCHAR,UCHAR,USHORT,PUCHAR,ULONG,PULONG);
BOOL __stdcall WinUsb_Wrapper_GetDescriptor(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR DescriptorType,
		UCHAR Index,
		USHORT LanguageID,
		PUCHAR Buffer,
		ULONG BufferLength,
		PULONG LengthTransferred
);

typedef BOOL (WINAPI *WINUSB_GET_OVERLAPPED_RESULT)(WINUSB_INTERFACE_HANDLE,LPOVERLAPPED,LPDWORD,BOOL);
BOOL __stdcall WinUsb_Wrapper_GetOverlappedResult(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		LPOVERLAPPED lpOverlapped,
		LPDWORD lpNumberOfBytesTransferred,
		BOOL bWait
);

typedef BOOL (WINAPI *WINUSB_GET_PIPE_POLICY)(WINUSB_INTERFACE_HANDLE,UCHAR,ULONG,PULONG,PVOID);
BOOL __stdcall WinUsb_Wrapper_GetPipePolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID,
		ULONG PolicyType,
		PULONG ValueLength,
		PVOID Value
);

typedef BOOL (WINAPI *WINUSB_GET_POWER_POLICY)(WINUSB_INTERFACE_HANDLE,ULONG,PULONG,PVOID);
BOOL __stdcall WinUsb_Wrapper_GetPowerPolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		ULONG PolicyType,
		PULONG ValueLength,
		PVOID Value
);

typedef BOOL (WINAPI *WINUSB_INITIALIZE)(HANDLE,PWINUSB_INTERFACE_HANDLE);
BOOL __stdcall WinUsb_Wrapper_Initialize(
		HANDLE DeviceHandle,
		PWINUSB_INTERFACE_HANDLE InterfaceHandle
);

typedef PUSB_INTERFACE_DESCRIPTOR (WINAPI *WINUSB_PARSE_CONFIGURATION_DESCRIPTOR)(PUSB_CONFIGURATION_DESCRIPTOR, PVOID, LONG, LONG, LONG, LONG, LONG);
PUSB_INTERFACE_DESCRIPTOR __stdcall WinUsb_Wrapper_ParseConfigurationDescriptor(
		PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
		PVOID StartPosition,
		LONG InterfaceNumber,
		LONG AlternateSetting,
		LONG InterfaceClass,
		LONG InterfaceSubClass,
		LONG InterfaceProtocol
);

typedef PUSB_COMMON_DESCRIPTOR (WINAPI *WINUSB_PARSE_DESCRIPTORS)(PVOID, ULONG, PVOID, LONG);
PUSB_COMMON_DESCRIPTOR __stdcall WinUsb_Wrapper_ParseDescriptors(
		PVOID DescriptorBuffer,
		ULONG TotalLength,
		PVOID StartPosition,
		LONG DescriptorType
);

typedef BOOL (WINAPI *WINUSB_QUERY_DEVICE_INFORMATION)(WINUSB_INTERFACE_HANDLE,ULONG,PULONG,PVOID);
BOOL __stdcall WinUsb_Wrapper_QueryDeviceInformation(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		ULONG InformationType,
		PULONG BufferLength,
		PVOID Buffer
);

typedef BOOL (WINAPI *WINUSB_QUERY_INTERFACE_SETTINGS)(WINUSB_INTERFACE_HANDLE,UCHAR,PUSB_INTERFACE_DESCRIPTOR);
BOOL __stdcall WinUsb_Wrapper_QueryInterfaceSettings(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AlternateSettingNumber,
		PUSB_INTERFACE_DESCRIPTOR UsbAltInterfaceDescriptor
);

typedef BOOL (WINAPI *WINUSB_QUERY_PIPE)(WINUSB_INTERFACE_HANDLE,UCHAR,UCHAR,PWINUSB_PIPE_INFORMATION);
BOOL __stdcall WinUsb_Wrapper_QueryPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AlternateInterfaceNumber,
		UCHAR PipeIndex,
		PWINUSB_PIPE_INFORMATION PipeInformation
);

typedef BOOL (WINAPI *WINUSB_READ_PIPE)(WINUSB_INTERFACE_HANDLE,UCHAR,PUCHAR,ULONG,PULONG,LPOVERLAPPED);
BOOL __stdcall WinUsb_Wrapper_ReadPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID,
		PUCHAR Buffer,
		ULONG BufferLength,
		PULONG LengthTransferred,
		LPOVERLAPPED Overlapped
);

typedef BOOL (WINAPI *WINUSB_RESET_PIPE)(WINUSB_INTERFACE_HANDLE,UCHAR);
BOOL __stdcall WinUsb_Wrapper_ResetPipe(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID
);

typedef BOOL (WINAPI *WINUSB_SET_CURRENT_ALTERNATE_SETTING)(WINUSB_INTERFACE_HANDLE,UCHAR);
BOOL __stdcall WinUsb_Wrapper_SetCurrentAlternateSetting(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR AlternateSetting
);

typedef BOOL (WINAPI *WINUSB_SET_PIPE_POLICY)(WINUSB_INTERFACE_HANDLE,UCHAR,ULONG,ULONG,PVOID);
BOOL __stdcall WinUsb_Wrapper_SetPipePolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		UCHAR PipeID,
		ULONG PolicyType,
		ULONG ValueLength,
		PVOID Value
);

typedef BOOL (WINAPI *WINUSB_SET_POWER_POLICY)(WINUSB_INTERFACE_HANDLE,ULONG,ULONG,PVOID);
BOOL __stdcall WinUsb_Wrapper_SetPowerPolicy(
		WINUSB_INTERFACE_HANDLE InterfaceHandle,
		ULONG PolicyType,
		ULONG ValueLength,
		PVOID Value
);

typedef BOOL (WINAPI *WINUSB_WRITE_PIPE)(WINUSB_INTERFACE_HANDLE,UCHAR,PUCHAR,ULONG,PULONG,LPOVERLAPPED);
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
