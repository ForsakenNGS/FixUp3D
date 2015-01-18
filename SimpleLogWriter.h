/*
 * SimpleLogWriter.h
 *
 *  Created on: 17.01.2015
 *      Author: Forsaken
 */

#ifndef SIMPLELOGWRITER_H_
#define SIMPLELOGWRITER_H_

#include <windows.h>
#include <iostream>
#include <TCHAR.H>

namespace Core {

class SimpleLogWriter {
private:
	HANDLE		hFile;
public:
	SimpleLogWriter(const TCHAR* debugFilename);
	virtual ~SimpleLogWriter();

	SimpleLogWriter*	writeLong(ULONG number);
	SimpleLogWriter*	writeFloat(FLOAT number);
	SimpleLogWriter*	writeString(const char* text);
	SimpleLogWriter*	writeBinaryBuffer(PVOID buffer, ULONG bufferLength);
};

} /* namespace Core */

#endif /* SIMPLELOGWRITER_H_ */
