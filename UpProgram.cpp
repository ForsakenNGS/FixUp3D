/*
 * UpProgrammWriter.cpp
 *
 *  Created on: 17.02.2015
 *      Author: Forsaken
 */

#include "PrinterIntercept.h"
#include "UpProgram.h"

namespace Core {

UpProgram::UpProgram() {
	nozzleTemp1 = 280.0f;
	nozzleTemp2 = 30.0f;
	bedTemp = 105.0f;
}

UpProgram::UpProgram(ULONG fNozzleTemp1, ULONG fNozzleTemp2, ULONG fBedTemp) {
	nozzleTemp1 = fNozzleTemp1;
	nozzleTemp2 = fNozzleTemp2;
	bedTemp = fBedTemp;
}

UpProgram::~UpProgram() {
}

UpProgramLayer& UpProgram::addLayer(ULONG iLayer, ULONG iTimeRemaining, ULONG iProgressPercent, float fHeight) {
	if (layers.find(iLayer) == layers.end()) {
		layers[iLayer] = new UpProgramLayer(iLayer, iTimeRemaining, iProgressPercent, fHeight);
	}
	return *layers[iLayer];
}

void UpProgram::clearLayers() {
	for (std::map<ULONG,UpProgramLayer*>::iterator it = layers.begin(); it != layers.end(); ++it) {
		// Free memory
		delete it->second;
		it->second = NULL;
	}
	layers.clear();
}

UpProgramLayer*	UpProgram::getLayer(ULONG iLayer) {
	if (layers.find(iLayer) != layers.end()) {
		return layers[iLayer];
	} else {
		return NULL;
	}
}

void UpProgram::writeToPrinter() {
	PrinterIntercept* 		intercept = PrinterIntercept::getInstance();
	FixUp3DMemBlock			memBlock;

	// Start a new program
	intercept->sendProgramNew();			// PROGRAM_NEW				63
	intercept->setUnknown0A(0);				// SET_UNKNOWN_0A			560A	00000000
	intercept->setUnknown0B(0);				// SET_UNKNOWN_0B			560B	00000000
	intercept->sendUnknown6C(0x0109);		// UNKNOWN_6C				6C		0901
	intercept->setPrinterStatus(0x13);		// SET_PRINTER_STATUS		5610	13000000

	// Set params for layer 0
	memBlock.command = FIXUP3D_MEM_CMD_SET_PARAM;
	// - Nozzle 1 Temp
	memBlock.params.longs.lParam1 = FIXUP3D_MEM_PARAM_NOZZLE1_TEMP;
	memBlock.params.longs.lParam2 = nozzleTemp1;
	intercept->writeMemory1(memBlock);
	// - Nozzle 2 Temp
	memBlock.params.longs.lParam1 = FIXUP3D_MEM_PARAM_NOZZLE2_TEMP;
	memBlock.params.longs.lParam2 = nozzleTemp2;
	intercept->writeMemory1(memBlock);
	// - Bed Temp
	memBlock.params.longs.lParam1 = FIXUP3D_MEM_PARAM_BED_TEMP;
	memBlock.params.longs.lParam2 = bedTemp;
	intercept->writeMemory1(memBlock);
	intercept->sendProgramCommitHead();		// PROGRAM_COMMIT_HEAD		4C32

	// Enable bed heating and do some stuff....  (enable nozzle heater?)
	intercept->setUnknown14(1);				// SET_UNKNOWN_0A			5614	01000000
	intercept->setPreheatTimer(1);			// SET_PREHEAT_TIMER		5616	01000000
	intercept->setUnknown8E(0xF);			// SET_UNKNOWN_8E			568E	0F000000
	intercept->setUnknown4D(7);				// SET_UNKNOWN_4D			564D	07000000
	intercept->setUnknown4C(0);				// SET_UNKNOWN_4C			564C	00000000
	// Get commands relevant? GET_PRINTER_STATUS, GET_LAYER, GET_UNKOWN0B, GET_NOZZLE1_TEMP, GET_NOZZLE2_TEMP, GET_BED_TEMP

	// LAYERS
	for (std::map<ULONG,UpProgramLayer*>::iterator it = layers.begin(); it != layers.end(); ++it) {
		it->second->writeToPrinter();
	}
	intercept->sendProgramCommitLayers();	// PROGRAM_COMMIT_LAYERS	4C35
	// Get commands relevant? GET_PRINTER_STATUS, GET_LAYER, GET_UNKOWN0B, GET_NOZZLE1_TEMP, GET_NOZZLE2_TEMP, GET_BED_TEMP
	intercept->sendUnknown46();				// UNKNOWN_46				46

	// Stop!
	memBlock.command = FIXUP3D_MEM_CMD_STOP;
	intercept->writeMemory3(memBlock, memBlock, memBlock);
	// Get commands relevant? GET_PRINTER_STATUS, GET_LAYER, GET_UNKOWN0B, GET_NOZZLE1_TEMP, GET_NOZZLE2_TEMP, GET_BED_TEMP

	intercept->sendUnknown46();				// UNKNOWN_46				46
	// Get commands relevant? GET_PRINTER_STATUS, GET_LAYER, GET_UNKOWN0B, GET_NOZZLE1_TEMP, GET_NOZZLE2_TEMP, GET_BED_TEMP

	intercept->sendProgramNew();			// PROGRAM_NEW				63
	intercept->sendUnknown6C(0x0009);		// UNKNOWN_6C				6C		0900
	intercept->sendProgramGo();				// PROGRAM_GO				58
}

} /* namespace Core */
