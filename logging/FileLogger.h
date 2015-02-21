/*
 * FileLogger.h
 *
 *  Created on: 01.08.2012
 *      Author: Unknown
 */

#ifndef FILELOGGER_H_
#define FILELOGGER_H_

#include "Target.h"
#include <windows.h>
#include <fstream>

namespace Logging {


class FileLogger: public Target {
private:
    CRITICAL_SECTION 		m_criticalSection;
	std::ofstream outf;
public:
	FileLogger(const char* filename, const int lvl, const int bitSection);
	FileLogger(const char* filename, const std::ios_base::openmode mode, const int lvl, const int bitSection);
	virtual ~FileLogger();

	virtual void put(const char* str, const int lvl, const int bitSection);
};

} /* namespace Logging */

#endif /* FILELOGGER_H_ */
