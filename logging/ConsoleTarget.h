/*
 * ConsoleTarget.h
 *
 *  Created on: 01.08.2012
 *      Author: Unknown
 */

#ifndef CONSOLETARGET_H_
#define CONSOLETARGET_H_

#include "Target.h"
#include <windows.h>

namespace Logging {

class ConsoleTarget : public Target {
private:
    CRITICAL_SECTION 		m_criticalSection;
public:
	ConsoleTarget(const int lvl, const int bitSection);
	virtual ~ConsoleTarget();

	void put(const char* str, const int lvl, const int bitSection);
};

} /* namespace Logging */
#endif /* CONSOLETARGET_H_ */
