/*
 * UpProgrammWriter.h
 *
 *  Created on: 17.02.2015
 *      Author: Forsaken
 */

#ifndef UPPROGRAMMLAYER_H_
#define UPPROGRAMMLAYER_H_

#include <map>
#include "PrinterIntercept.h"
#include "UpProgramLayer.h"

namespace Core {

class UpProgram {
private:
	LONG 							nozzleTemp1, nozzleTemp2, bedTemp;
	std::map<ULONG,UpProgramLayer*>	layers;
public:
	UpProgram();
	UpProgram(ULONG nozzleTemp1, ULONG nozzleTemp2, ULONG bedTemp);
	virtual ~UpProgram();

	UpProgramLayer&	addLayer(ULONG iLayer, ULONG iTimeRemaining, ULONG iProgressPercent, float fHeight);
	void 			clearLayers();
	UpProgramLayer*	getLayer(ULONG iLayer);

	void	writeToPrinter();
};

} /* namespace Core */

#endif /* UPPROGRAMMLAYER_H_ */
