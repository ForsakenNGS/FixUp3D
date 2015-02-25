/*
 * UpProgrammWriter.cpp
 *
 *  Created on: 17.02.2015
 *      Author: Forsaken
 */

#include "PrinterIntercept.h"
#include "UpProgramLayer.h"

namespace Core {

UpProgramLayer::UpProgramLayer(unsigned int iLayer, unsigned int iTimeRemaining, unsigned int iProgressPercent, float fHeight) {
	FixUp3DMemBlock			memBlock;
	// - SET_PARAM: Report time remaining
	memBlock.command = FIXUP3D_MEM_CMD_SET_PARAM;
	memBlock.params.longs.lParam1 = FIXUP3D_MEM_PARAM_TIME_REMAINING;
	memBlock.params.longs.lParam2 = iTimeRemaining;
	commandQueue.push(memBlock);
	// - SET_PARAM: Report time gone
	memBlock.command = FIXUP3D_MEM_CMD_SET_PARAM;
	memBlock.params.longs.lParam1 = FIXUP3D_MEM_PARAM_TIME_PERCENT;
	memBlock.params.longs.lParam2 = iProgressPercent;
	commandQueue.push(memBlock);
	// - SET_PARAM: Current layer
	memBlock.command = FIXUP3D_MEM_CMD_SET_PARAM;
	memBlock.params.longs.lParam1 = FIXUP3D_MEM_PARAM_LAYER;
	memBlock.params.longs.lParam2 = iLayer;
	commandQueue.push(memBlock);
	// - SET_PARAM: Report height
	memBlock.command = FIXUP3D_MEM_CMD_SET_PARAM;
	memBlock.params.longs.lParam1 = FIXUP3D_MEM_PARAM_HEIGHT;
	memBlock.params.floats.fParam2 = fHeight;
	commandQueue.push(memBlock);
}

UpProgramLayer::~UpProgramLayer() {
	// TODO Auto-generated destructor stub
}

void UpProgramLayer::jumpToZ(float posZ, float speedZ) {
	jumpTo(0.0f, 0.0f, 0.0f, 0.0f, posZ, speedZ, -10.0f, 10000.0f);
}

void UpProgramLayer::jumpToZ(float posZ, float speedZ, float posE, float speedE) {
	jumpTo(0.0f, 0.0f, 0.0f, 0.0f, posZ, speedZ, posE, speedE);
}

void UpProgramLayer::jumpToXY(float posX, float speedX, float posY, float speedY) {
	jumpTo(posX, speedX, posY, speedY, 0.0f, 0.0f, 0.0f, 10000.0f);
}

void UpProgramLayer::jumpToXY(float posX, float speedX, float posY, float speedY, float posE, float speedE) {
	jumpTo(posX, speedX, posY, speedY, 0.0f, 0.0f, posE, speedE);
}

void UpProgramLayer::jumpTo(float posX, float speedX, float posY, float speedY, float posZ, float speedZ, float posE, float speedE) {
	FixUp3DMemBlock	memMoveA, memMoveB;
	memMoveA.command = FIXUP3D_MEM_CMD_MOVE_FLOAT;
	memMoveA.params.floats.fParam1 = posX;
	memMoveA.params.floats.fParam2 = speedX;
	memMoveA.params.floats.fParam3 = posY;
	memMoveA.params.floats.fParam4 = speedY;
	memMoveB.command = FIXUP3D_MEM_CMD_MOVE_FLOAT;
	memMoveB.params.floats.fParam1 = posZ;
	memMoveB.params.floats.fParam2 = speedZ;
	memMoveB.params.floats.fParam3 = posE;
	memMoveB.params.floats.fParam4 = speedE;
	commandQueue.push(memMoveA);
	commandQueue.push(memMoveB);
}

void UpProgramLayer::setParam(unsigned int param, unsigned int value1) {
	setParam(param, value1, 0);
}

void UpProgramLayer::setParam(unsigned int param, unsigned int value1, unsigned int value2) {
	setParam(param, value1, value2, 0);
}

void UpProgramLayer::setParam(unsigned int param, unsigned int value1, unsigned int value2, unsigned int value3) {
	FixUp3DMemBlock memParam;
	memParam.command = FIXUP3D_MEM_CMD_SET_PARAM;
	memParam.params.longs.lParam1 = param;
	memParam.params.longs.lParam2 = value1;
	memParam.params.longs.lParam3 = value1;
	memParam.params.longs.lParam4 = value1;
	commandQueue.push(memParam);
}

void UpProgramLayer::stop() {
	FixUp3DMemBlock memStop;
	memStop.command = FIXUP3D_MEM_CMD_STOP;
	memStop.params.longs.lParam1 = 0;
	memStop.params.longs.lParam2 = 0;
	memStop.params.longs.lParam3 = 0;
	memStop.params.longs.lParam4 = 0;
	commandQueue.push(memStop);
	commandQueue.push(memStop);
	commandQueue.push(memStop);
}

void UpProgramLayer::unknown5(unsigned int unknownParam) {
	FixUp3DMemBlock memUnkown5;
	memUnkown5.command = FIXUP3D_MEM_CMD_UNKNOWN5;
	memUnkown5.params.longs.lParam1 = unknownParam;
	memUnkown5.params.longs.lParam2 = 0;
	memUnkown5.params.longs.lParam3 = 0;
	memUnkown5.params.longs.lParam4 = 0;
	commandQueue.push(memUnkown5);
}

/**
 * Write layer data
 */
void UpProgramLayer::writeToPrinter() {
	PrinterIntercept* 		intercept = PrinterIntercept::getInstance();
	intercept->sendProgramCmdsFree();				// UNKNOWN_46			46
	intercept->writeMemory(commandQueue);
}

} /* namespace Core */
