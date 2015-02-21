/*
 * FileLogger.cpp
 *
 *  Created on: 01.08.2012
 *      Author: Unknown
 */

#include "FileLogger.h"

namespace Logging {

FileLogger::FileLogger(const char* filename, const int lvl, const int bitSection) : Target(lvl, bitSection), outf(filename, std::ios_base::app | std::ios_base::out) {
	InitializeCriticalSection(&m_criticalSection);
}

FileLogger::FileLogger(const char* filename, const std::ios_base::openmode mode, const int lvl, const int bitSection) : Target(lvl, bitSection), outf(filename, mode) {
	InitializeCriticalSection(&m_criticalSection);
}

FileLogger::~FileLogger() {
	// Auto-generated destructor stub
}

void FileLogger::put(const char* str, const int lvl, const int bitSection){
	if ((lvl <= maxlvl) && ((section & bitSection) > 0)) {
	    EnterCriticalSection(&m_criticalSection);
		outf << str;
		outf.flush();
	    LeaveCriticalSection(&m_criticalSection);
	}
}

} /* namespace Logging */
