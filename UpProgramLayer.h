/*
 * UpProgrammWriter.h
 *
 *  Created on: 17.02.2015
 *      Author: Forsaken
 */

#ifndef UPPROGRAMLAYER_H_
#define UPPROGRAMLAYER_H_

#include <queue>
#include "PrinterIntercept.h"

namespace Core {

class UpProgramLayer {
private:
	std::queue<FixUp3DMemBlock>		commandQueue;
public:
	UpProgramLayer(unsigned int iLayer, unsigned int iTimeRemaining, unsigned int iProgressPercent, float fHeight);
	virtual ~UpProgramLayer();

	void	jumpToZ(float posZ, float speedZ);
	void	jumpToZ(float posZ, float speedZ, float posE, float speedE);
	void	jumpToXY(float posX, float speedX, float posY, float speedY);
	void	jumpToXY(float posX, float speedX, float posY, float speedY, float posE, float speedE);
	void	jumpTo(float posX, float speedX, float posY, float speedY, float posZ, float speedZ, float posE, float speedE);
	void	setParam(unsigned int param, unsigned int value1);
	void	setParam(unsigned int param, unsigned int value1, unsigned int value2);
	void	setParam(unsigned int param, unsigned int value1, unsigned int value2, unsigned int value3);
	void	stop();
	void	unknown5(unsigned int unknownParam);

	void	writeToPrinter();
};

} /* namespace Core */

#endif /* UPPROGRAMLAYER_H_ */
