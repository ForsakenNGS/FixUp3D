/*
 * LogTarget.h
 *
 *  Created on: 01.08.2012
 *      Author: Unknown
 */

#ifndef LOGTARGET_H_
#define LOGTARGET_H_

#include "Target.h"
#include <sstream>
#include <map>

namespace Logging {

class LogTarget : public std::ostringstream {
private:
	int lvl;
	int sectionCur;
	int sections;
	std::map<std::string,Target*> targets;
public:
	LogTarget(LogTarget& source, int bitSections);
	LogTarget(int bitSections);
	virtual ~LogTarget();

	void addTarget(const std::string &name, Target* target);
	void flush();
	const int getLevel();
	const int getSection();
	const std::map<std::string,Target*> & getTargets();
	Target* getTarget(const std::string &name);
	void setLevel(int lvl);
	void setSection(int bitSection);

};

} /* namespace Logging */

#endif /* LOGTARGET_H_ */
