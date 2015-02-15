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
#define UPCMD_GetArgLo(Value)			(USHORT)( Value & 0xFF )
#define UPCMD_GetArgHi(Value)			(USHORT)( (Value >> 8) & 0xFF )

#define FIXUP3D_CMD_UNKNOWN53			0x53
#define FIXUP3D_CMD_PROGRAM_GO			0x58
#define FIXUP3D_CMD_PROGRAM_NEW			0x63
#define	FIXUP3D_CMD_UNKNOWN6C			0x6C

#define	FIXUP3D_CMD_ERASE_CONFIG		UPCMD(0x77,0x02)
#define	FIXUP3D_CMD_ERASE_ROMPRG		UPCMD(0x77,0x01)

#define	FIXUP3D_CMD_GET_UNKNOWN_STATUS	UPCMD(0x76,0x00)
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
#define	FIXUP3D_CMD_GET_PRINTER_STATUS	UPCMD(0x76,0x10)
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
#define	FIXUP3D_CMD_GET_UNKOWN8E		UPCMD(0x76,0x8E)

#define	FIXUP3D_CMD_SEND_1STBLOCK		UPCMD(0x75,0x01)
#define	FIXUP3D_CMD_SEND_BLOCK			UPCMD(0x75,0x02)

#define	FIXUP3D_CMD_UNKNOWN7330			UPCMD(0x73,0x30)
#define	FIXUP3D_CMD_UNKNOWN7331			UPCMD(0x73,0x31)
#define	FIXUP3D_CMD_UNKNOWN7332			UPCMD(0x73,0x32)
#define	FIXUP3D_CMD_UNKNOWN7333			UPCMD(0x73,0x33)

#define	FIXUP3D_CMD_SET_UNKNOWN0A		UPCMD(0x56,0x0A)
#define	FIXUP3D_CMD_SET_UNKNOWN0B		UPCMD(0x56,0x0B)
#define	FIXUP3D_CMD_SET_PRINTER_STATUS	UPCMD(0x56,0x10)
#define	FIXUP3D_CMD_SET_UNKNOWN14		UPCMD(0x56,0x14)
#define	FIXUP3D_CMD_SET_UNKNOWN16		UPCMD(0x56,0x16)
#define	FIXUP3D_CMD_SET_PREHEAT_TIMER	UPCMD(0x56,0x16)
#define	FIXUP3D_CMD_SET_NOZZLE1_TEMP	UPCMD(0x56,0x39)
#define	FIXUP3D_CMD_SET_NOZZLE2_TEMP	UPCMD(0x56,0x3A)
#define	FIXUP3D_CMD_SET_BED_TEMP		UPCMD(0x56,0x3B)
#define	FIXUP3D_CMD_SET_UNKNOWN4C		UPCMD(0x56,0x4C)
#define	FIXUP3D_CMD_SET_UNKNOWN4D		UPCMD(0x56,0x4D)
#define	FIXUP3D_CMD_SET_UNKNOWN8E		UPCMD(0x56,0x8E)
#define	FIXUP3D_CMD_SET_UNKNOWN94		UPCMD(0x56,0x94)

#define	FIXUP3D_CMD_COMMIT_IDBLK		UPCMD(0x55,0x06)
#define	FIXUP3D_CMD_COMMIT_SETBLK		UPCMD(0x55,0x05)
#define	FIXUP3D_CMD_COMMIT_UNK04		UPCMD(0x55,0x04)
#define	FIXUP3D_CMD_COMMIT_INPORTBLK	UPCMD(0x55,0x03)
#define	FIXUP3D_CMD_COMMIT_OUTPORTBLK	UPCMD(0x55,0x02)
#define	FIXUP3D_CMD_COMMIT_UNK01		UPCMD(0x55,0x01)
#define	FIXUP3D_CMD_COMMIT_UNK00		UPCMD(0x55,0x00)

#define	FIXUP3D_CMD_GET_PRINTERPARAM	UPCMD(0x52,0x00)

#define FIXUP3D_CMD_UNKNOWN4C32			UPCMD(0x4c,0x32)
#define FIXUP3D_CMD_UNKNOWN4C33			UPCMD(0x4c,0x33)
#define FIXUP3D_CMD_UNKNOWN4C35			UPCMD(0x4c,0x35)

#define	FIXUP3D_CMD_WRITE_MEM_1			UPCMD(0x2F,0x01)
#define	FIXUP3D_CMD_WRITE_MEM_2			UPCMD(0x2F,0x02)
#define	FIXUP3D_CMD_WRITE_MEM_3			UPCMD(0x2F,0x03)

#define	FIXUP3D_CMD_GET_CONNECTED		UPCMD(0x01,0x00)

#define FIXUP3D_CMD_NONE				UPCMD(0xFF,0xFF)
#define FIXUP3D_CMD_PAUSE				UPCMD(0xFF,0x01)

// Printer status values
#define FIXUP3D_STATUS_UNKNOWN0			0x00
#define FIXUP3D_STATUS_INITIALIZED		0x01
#define FIXUP3D_STATUS_PRINTING			0x02
#define FIXUP3D_STATUS_UNKNOWN3			0x03
#define FIXUP3D_STATUS_UNKNOWN7			0x07
// Preheat states
#define FIXUP3D_PREHEAT_DISABLED		0x00
#define FIXUP3D_PREHEAT_IDLE			0x01
#define FIXUP3D_PREHEAT_STOPPING		0x02
#define FIXUP3D_PREHEAT_HEATING			0x03
#define FIXUP3D_PREHEAT_PRINTING		0x04
// Custom states for the
#define FIXUP3D_REPLY_DONT_INTERCEPT		0x0000
#define FIXUP3D_REPLY_ACKNOWLEDGED			0x0001
#define FIXUP3D_REPLY_CUSTOMPRINTERDATA		0x0002

// Commands that are written as to sdcard
#define FIXUP3D_MEM_CMD_MOVE			0x00000003
#define FIXUP3D_MEM_CMD_EXTRUDER_CTRL	0x00000005
#define FIXUP3D_MEM_CMD_SET_PARAM		0x00000006
// Parameters sent using the "FIXUP3D_MEM_CMD_SET_PARAM" command
#define FIXUP3D_MEM_PARAM_LAYER			0x0000000A
#define FIXUP3D_MEM_PARAM_NOZZLE1_TEMP	0x00000039

struct	FixUp3DCustomCommand {
	USHORT			command;
	USHORT			commandBytes;
	void*			arguments;
	ULONG			argumentsLength;
	ULONG			responseLength;
};

union	FixUp3DMemBlockParams {
	struct	FixUp3DMemBlockFloatParams {
		FLOAT	fParam1;
		FLOAT	fParam2;
		FLOAT	fParam3;
		FLOAT	fParam4;
	} floats;
	struct	FixUp3DMemBlockLongParams {
		ULONG	lParam1;
		ULONG	lParam2;
		ULONG	lParam3;
		ULONG	lParam4;
	} longs;
};

struct	FixUp3DMemBlock {
	ULONG					command;
	FixUp3DMemBlockParams	params;
};

typedef std::queue<FixUp3DCustomCommand> FixUp3dCmdQueue;

class PrinterIntercept {
private:
    static PrinterIntercept	*instance;

	FixUp3dCmdQueue			customCommands;
	BOOL					customCommandsSending;

	ULONG					interceptReply;

	SimpleLogWriter*		log = NULL;

	USHORT					lastWriteCommand;
	USHORT					lastWriteArgumentLo;
	USHORT					lastWriteArgumentHi;
	ULONG					lastWriteArgumentLong;
	ULONG					lastWriteCustom;
	BOOL					lastWriteKeep;

	ULONG					temperatureNozzle1Base;
	ULONG					temperatureNozzle2Base;
	ULONG					temperatureNozzle3Base;

	ULONG					printerStatus;
	ULONG					preheatStatus;
	ULONG					memCurrentLayer;

	void	addCustomCommand(FixUp3DCustomCommand &command);
	void	addCustomCommandDelay(ULONG delayInMs);
	BOOL	sendCustomCommand(WINUSB_INTERFACE_HANDLE interfaceHandle, FixUp3DCustomCommand &command);
	BOOL	handleUpCmdSend(USHORT command, USHORT arg1, USHORT arg2, ULONG argLong, PUCHAR buffer, ULONG bufferLength);
	void	handleUpCmdReply(USHORT command, USHORT arg1, USHORT arg2, ULONG argLong, PUCHAR buffer, ULONG lengthTransferred);
	void	handleUpMemBlock(FixUp3DMemBlock* memBlock);
	void	handleUpMemBlock_SetParam(FixUp3DMemBlockParams& memBlock);
public:
	PrinterIntercept();
	virtual ~PrinterIntercept();
	static PrinterIntercept* getInstance();

	ULONG	handleUsbPreRead(WINUSB_INTERFACE_HANDLE interfaceHandle, UCHAR pipeID, PUCHAR buffer, ULONG bufferLength);
	void	handleUsbRead(WINUSB_INTERFACE_HANDLE interfaceHandle, UCHAR pipeID, PUCHAR buffer, ULONG lengthTransferred);
	BOOL	handleUsbWrite(WINUSB_INTERFACE_HANDLE interfaceHandle, UCHAR pipeID, PUCHAR buffer, ULONG bufferLength);

	void	sendGetConnected();
	void	sendGetUnknownStatus();
	void	sendGetPrinterStatus();
	void	sendGetUnknown8E();
	void	sendProgramNew();
	void	sendProgramGo();
	void	sendUnknown53();
	void	sendUnknown4C32();
	void	sendUnknown4C33();
	void	sendUnknown4C35();
	void	sendUnknown6C(USHORT param);
	void	sendUnknown7330();
	void	sendUnknown7331();
	void	sendUnknown7332();
	void	sendUnknown7333();
	void	setNozzle1Temp(ULONG temperature);
	void	setUnknown10(ULONG value);
	void	setUnknown14(ULONG value);
	void	setUnknown16(ULONG value);
	void	setUnknown8E(ULONG value);
	void	setPreheatTimer(ULONG value);
	void	stopPrint();
	void	printAgain();
};

} /* namespace Core */

#endif /* DEBUG_PRINTERINTERCEPT_H_ */
