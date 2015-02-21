/*
 * Logger.cpp
 *
 *  Created on: 01.08.2012
 *      Author: Forsaken
 */

#include "Log.h"
#include "logging/ConsoleTarget.h"
#include <time.h>

#include <iostream>
#include <iomanip>

using namespace std;
using namespace Logging;

LogTarget* Log::logTarget = NULL;

Log::Log(const char* name) : sName(name) {
	iLevel = 0;
	iSections = LogSections::SECTION_ANY;
	bWritename = true;
}

Log::Log(const char* name, int bitSections) : sName(name) {
	iLevel = 0;
	iSections = bitSections;
	bWritename = true;
}

Log::~Log() {
	// TODO Auto-generated destructor stub
}

Log& Log::get(const int level) {
	iLevel = level;
	return *this;
}

void Log::writeName() {
	bWritename = false;
	// Output timestamp
	time_t now;
	time(&now);
	struct tm *nowLocal = localtime ( &now );
	Log::getTarget() << dec << setfill('0') << "[" << setw(2) << nowLocal->tm_hour << ":" << setw(2) << nowLocal->tm_min << ":" << setw(2) << nowLocal->tm_sec << "]" << " ";
	// Output log level and category
	switch (iLevel) {
	case LogLevel::FATAL:
		Log::getTarget() << "[FATAL]\t";
		break;
	case LogLevel::ERR0R:
		Log::getTarget() << "[ERROR]\t";
		break;
	case LogLevel::WARNING:
		Log::getTarget() << "[WARN]\t";
		break;
	case LogLevel::INFO:
		Log::getTarget() << "[INFO]\t";
		break;
	case LogLevel::DEBUG:
		Log::getTarget() << "[DEBUG]\t";
		break;
	default:
		Log::getTarget() << "[UNDEF]\t";
		break;
	}
	Log::getTarget() << sName << ": ";
}

void Log::writeBinaryAsHex(const int level, void* buffer, unsigned int bufferLength) {
	char	sBuffer[2];
	Log&	logTarget = this->get(level);
	for (ULONG i = 0; i < bufferLength; i++) {
		sprintf(sBuffer, "%02x", *((PUCHAR)buffer + i));
		logTarget << sBuffer;
	}
}

void Log::addTarget(const string &name, Target* target) {
	getTarget().addTarget(name, target);
}

LogTarget& Log::getTarget() {
	if (logTarget == NULL) {
		logTarget = new LogTarget(LogSections::SECTION_ANY);
		// TODO: Dynamic log level
		logTarget->addTarget("console", new ConsoleTarget(LogLevel::DEBUG, LogSections::SECTION_ANY));
	}
	return *logTarget;
}

Target* Log::getTarget(const string &name) {
	return getTarget().getTarget(name);
}
