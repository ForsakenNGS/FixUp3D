/*
 * FileLogger.cpp
 *
 *  Created on: 01.08.2012
 *      Author: Unknown
 */

#include "FileLogger.h"

namespace Logging {

FileLogger::FileLogger(const char* filename, const int lvl) : Target(lvl), outf(filename, std::ios_base::app | std::ios_base::out) {
	long size = outf.tellp();
	if (size > MAX_LOG_FILE_SIZE) {
		outf.seekp(0);
	}
	InitializeCriticalSection(&m_criticalSection);
}

FileLogger::~FileLogger() {
	// Auto-generated destructor stub
}

void FileLogger::put(const char* str, const int lvl){
	if (lvl <= maxlvl){
	    EnterCriticalSection(&m_criticalSection);
		outf << str;
		outf.flush();
	    LeaveCriticalSection(&m_criticalSection);
	}
}

} /* namespace Logging */
