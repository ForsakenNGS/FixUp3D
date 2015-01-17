/*
 * PrinterIntercept.h
 *
 *  Created on: 17.01.2015
 *      Author: Forsaken
 */

#ifndef DEBUG_PRINTERINTERCEPT_H_
#define DEBUG_PRINTERINTERCEPT_H_

#include <winusb.h>
#include "SimpleLogWriter.h"

namespace Core {

#define UPCMD(A,B) 						( (B<<8)|A )
#define UPCMD_SetCmd(Buffer,Value)		*((PUSHORT)Buffer) = Value
#define UPCMD_SetArgLo(Buffer,Value)	*((PUSHORT)Buffer+2) = Value
#define UPCMD_SetArgHi(Buffer,Value)	*((PUSHORT)Buffer+4) = Value
#define UPCMD_SetArgLong(Buffer,Value)	*((PULONG)Buffer+2) = Value

#define	FIXUP3D_CMD_GET_NOZZLE1_TEMP	UPCMD(0x76,0x06)
#define	FIXUP3D_CMD_GET_NOZZLE2_TEMP	UPCMD(0x76,0x07)
#define	FIXUP3D_CMD_GET_BED_TEMP		UPCMD(0x76,0x08)
#define	FIXUP3D_CMD_PREHEAT				UPCMD(0x56,0x16)
#define	FIXUP3D_CMD_SET_NOZZLE1_TEMP	UPCMD(0x56,0x39)
#define	FIXUP3D_CMD_SET_NOZZLE2_TEMP	UPCMD(0x56,0x3A)
#define	FIXUP3D_CMD_SET_BED_TEMP		UPCMD(0x56,0x3B)
#define	FIXUP3D_CMD_WRITE_MEM			UPCMD(0x2F,0x01)

class PrinterIntercept {
private:
	SimpleLogWriter*	log = NULL;

	USHORT				lastWriteCommand;
	USHORT				lastWriteArgumentLo;
	USHORT				lastWriteArgumentHi;
	ULONG				lastWriteArgumentLong;
	ULONG				lastWriteCustom;
public:
	PrinterIntercept();
	virtual ~PrinterIntercept();

	void	handleUsbRead(WINUSB_INTERFACE_HANDLE InterfaceHandle, UCHAR PipeID, PUCHAR Buffer, ULONG LengthTransferred);
	void	handleUsbWrite(WINUSB_INTERFACE_HANDLE InterfaceHandle, UCHAR PipeID, PUCHAR Buffer, ULONG BufferLength);
	void	handleUpCmdSend(USHORT Command, USHORT Arg1, USHORT Arg2, ULONG ArgLong, PUCHAR Buffer, ULONG BufferLength);
	void	handleUpCmdReply(USHORT Command, USHORT Arg1, USHORT Arg2, ULONG ArgLong);
};

} /* namespace Core */

#endif /* DEBUG_PRINTERINTERCEPT_H_ */
