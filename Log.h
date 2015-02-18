/*
 * Logger.h
 *
 *  Created on: 01.08.2012
 *      Author: Forsaken
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include "logging/LogTarget.h"
#include "logging/LogLevel.h"
#include <string.h>
#include <map>
#include <iomanip>

class Log {
private:
	static Logging::LogTarget* 						logTarget;
public:
	bool bWritename;
	int iLevel;
	const char* sName;

	Log(const char* name);
	virtual ~Log();
	Log& operator<<(const char* val) {
		if (bWritename) {
			Log::getTarget().setLevel(iLevel);
			writeName();
		}
		Log::getTarget() << val;
		if (strchr(val, '\n')) {
			bWritename = true;
			Log::getTarget().flush();
		}
		return *this;
	};

	Log& get(const int level);
	void writeName();
	void writeBinaryAsHex(const int level, void* buffer, unsigned int bufferLength);

	static void addTarget(const std::string &name, Logging::Target* target);
	static Logging::LogTarget& getTarget();
	static Logging::Target* getTarget(const std::string &name);
};

template<typename T> Log& operator<<(Log& instance, T val) {
	Log::getTarget() << val;
	return instance;
};

#endif /* LOGGER_H_ */
