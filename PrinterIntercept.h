/*
 * PrinterIntercept.h
 *
 *  Created on: 17.01.2015
 *      Author: Forsaken
 */

#ifndef DEBUG_PRINTERINTERCEPT_H_
#define DEBUG_PRINTERINTERCEPT_H_

#include "winusbhelper.h"
#include <queue>
#include "SimpleLogWriter.h"

namespace Core {

#define UPCMD(A,B) 						( (B<<8)|A )
#define UPCMD_SetCmd(Buffer,Value)		*((PUSHORT)Buffer) = Value
#define UPCMD_SetArgLo(Buffer,Value)	*((PUSHORT)(Buffer+2)) = Value
#define UPCMD_SetArgHi(Buffer,Value)	*((PUSHORT)(Buffer+4)) = Value
#define UPCMD_SetArgLong(Buffer,Value)	*((PULONG)(Buffer+2)) = Value

#define	FIXUP3D_CMD_GET_UNKOWN00		UPCMD(0x76,0x00)
#define	FIXUP3D_CMD_GET_UNKOWN01		UPCMD(0x76,0x01)
#define	FIXUP3D_CMD_GET_UNKOWN02		UPCMD(0x76,0x02)
#define	FIXUP3D_CMD_GET_UNKOWN03		UPCMD(0x76,0x03)
#define	FIXUP3D_CMD_GET_UNKOWN04		UPCMD(0x76,0x04)
#define	FIXUP3D_CMD_GET_UNKOWN05		UPCMD(0x76,0x05)
#define	FIXUP3D_CMD_GET_NOZZLE1_TEMP	UPCMD(0x76,0x06)
#define	FIXUP3D_CMD_GET_NOZZLE2_TEMP	UPCMD(0x76,0x07)
#define	FIXUP3D_CMD_GET_BED_TEMP		UPCMD(0x76,0x08)
#define	FIXUP3D_CMD_GET_LAYER			UPCMD(0x76,0x0A)
#define	FIXUP3D_CMD_GET_UNKOWN0B		UPCMD(0x76,0x0B)
#define	FIXUP3D_CMD_GET_UNKOWN0F		UPCMD(0x76,0x0F)
#define	FIXUP3D_CMD_GET_UNKOWN10		UPCMD(0x76,0x10)
#define	FIXUP3D_CMD_GET_UNKOWN14		UPCMD(0x76,0x14)
#define	FIXUP3D_CMD_GET_UNKOWN15		UPCMD(0x76,0x15)
#define	FIXUP3D_CMD_GET_PREHEAT_TIMER	UPCMD(0x76,0x16)
#define	FIXUP3D_CMD_GET_UNKOWN1E		UPCMD(0x76,0x1E)
#define	FIXUP3D_CMD_GET_UNKOWN1F		UPCMD(0x76,0x1F)
#define	FIXUP3D_CMD_GET_UNKOWN20		UPCMD(0x76,0x20)
#define	FIXUP3D_CMD_GET_UNKOWN21		UPCMD(0x76,0x21)
#define	FIXUP3D_CMD_GET_UNKOWN2A		UPCMD(0x76,0x2A)
#define	FIXUP3D_CMD_GET_UNKOWN2B		UPCMD(0x76,0x2B)
#define	FIXUP3D_CMD_GET_UNKOWN32		UPCMD(0x76,0x32)
#define	FIXUP3D_CMD_GET_UNKOWN36		UPCMD(0x76,0x36)
#define	FIXUP3D_CMD_GET_UNKOWN3E		UPCMD(0x76,0x3E)
#define	FIXUP3D_CMD_GET_POSITION		UPCMD(0x76,0x8C)
#define	FIXUP3D_CMD_SET_UNKNOWN0A		UPCMD(0x56,0x0A)
#define	FIXUP3D_CMD_SET_UNKNOWN0B		UPCMD(0x56,0x0B)
#define	FIXUP3D_CMD_SET_UNKNOWN10		UPCMD(0x56,0x10)
#define	FIXUP3D_CMD_SET_UNKNOWN14		UPCMD(0x56,0x14)
#define	FIXUP3D_CMD_SET_PREHEAT_TIMER	UPCMD(0x56,0x16)
#define	FIXUP3D_CMD_SET_NOZZLE1_TEMP	UPCMD(0x56,0x39)
#define	FIXUP3D_CMD_SET_NOZZLE2_TEMP	UPCMD(0x56,0x3A)
#define	FIXUP3D_CMD_SET_BED_TEMP		UPCMD(0x56,0x3B)
#define	FIXUP3D_CMD_SET_UNKNOWN4C		UPCMD(0x56,0x4C)
#define	FIXUP3D_CMD_SET_UNKNOWN4D		UPCMD(0x56,0x4D)
#define	FIXUP3D_CMD_SET_UNKNOWN8E		UPCMD(0x56,0x8E)
#define	FIXUP3D_CMD_SET_UNKNOWN94		UPCMD(0x56,0x94)
#define	FIXUP3D_CMD_WRITE_MEM			UPCMD(0x2F,0x01)
#define FIXUP3D_CMD_NONE				UPCMD(0xFF,0xFF)

struct	FixUp3DCustomCommand {
	USHORT	command;
	void*	arguments;
	ULONG	argumentsLength;
	ULONG	responseLength;
};

typedef std::queue<FixUp3DCustomCommand> FixUp3dCmdQueue;

class PrinterIntercept {
private:
    static PrinterIntercept	*instance;

	FixUp3dCmdQueue			customCommands;
	BOOL					customCommandsSending;

	SimpleLogWriter*		log = NULL;

	USHORT					lastWriteCommand;
	USHORT					lastWriteArgumentLo;
	USHORT					lastWriteArgumentHi;
	ULONG					lastWriteArgumentLong;
	ULONG					lastWriteCustom;
	BOOL					lastWriteKeep;

	void	addCustomCommand(FixUp3DCustomCommand &command);
	void	sendCustomCommand(WINUSB_INTERFACE_HANDLE interfaceHandle, FixUp3DCustomCommand &command);
	void	handleUpCmdSend(USHORT command, USHORT arg1, USHORT arg2, ULONG argLong, PUCHAR buffer, ULONG bufferLength);
	void	handleUpCmdReply(USHORT command, USHORT arg1, USHORT arg2, ULONG argLong, PUCHAR buffer, ULONG lengthTransferred);
public:
	PrinterIntercept();
	virtual ~PrinterIntercept();
	static PrinterIntercept* getInstance();

	void	setNozzle1Temp(ULONG temperature);

	void	handleUsbRead(WINUSB_INTERFACE_HANDLE interfaceHandle, UCHAR pipeID, PUCHAR buffer, ULONG lengthTransferred);
	void	handleUsbWrite(WINUSB_INTERFACE_HANDLE interfaceHandle, UCHAR pipeID, PUCHAR buffer, ULONG bufferLength);
};

} /* namespace Core */

#endif /* DEBUG_PRINTERINTERCEPT_H_ */
