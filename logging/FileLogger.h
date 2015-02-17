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

// 1 MB
#define MAX_LOG_FILE_SIZE		1048576

namespace Logging {


class FileLogger: public Target {
private:
    CRITICAL_SECTION 		m_criticalSection;
	std::ofstream outf;
public:
	FileLogger(const char* filename, const int lvl);
	virtual ~FileLogger();

	virtual void put(const char* str, const int lvl);
};

} /* namespace Logging */

#endif /* FILELOGGER_H_ */
