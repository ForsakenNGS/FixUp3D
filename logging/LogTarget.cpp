/*
 * LogTarget.cpp
 *
 *  Created on: 01.08.2012
 *      Author: Unknown
 */

#include "LogTarget.h"
#include "../Log.h"
#include <string.h>

using namespace std;

namespace Logging {

/**
 * Custom copy constructor that creates a new stringstream (because it cant be copied)
 */
LogTarget::LogTarget(LogTarget& source) {
	lvl = source.getLevel();
	targets = source.getTargets();
}

LogTarget::LogTarget() {
}

LogTarget::~LogTarget() {
	for (map<string,Target*>::const_iterator it = targets.begin(); it != targets.end(); it++)
	{
		if (it->second != 0)
		{
			delete it->second;
		}
	}
}

void LogTarget::addTarget(const string &name, Target* target) {
	targets[name] = target;
}

void LogTarget::flush() {
	// Push to streams
	string str = this->str();
	const char* data = str.c_str();
	for (map<string,Target*>::const_iterator it = targets.begin(); it != targets.end(); it++) {
		it->second->put(data, lvl);
	}
	ostringstream::str(string());
	ostringstream::clear();
}

const int LogTarget::getLevel() {
	return lvl;
}

const map<string,Target*> & LogTarget::getTargets() {
	return targets;
}

Target* LogTarget::getTarget(const std::string &name) {
	if (targets.count(name) > 0) {
		return targets[name];
	} else {
		return NULL;
	}
}

void LogTarget::setLevel(int lvl){
	this->lvl = lvl;
}

} /* namespace Logging */
