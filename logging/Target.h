/*
 * Target.h
 *
 *  Created on: 01.08.2012
 *      Author: Unknown
 */

#ifndef TARGET_H_
#define TARGET_H_

namespace Logging {

class Target {
protected:
	int maxlvl;
	int	section;
public:
	Target(int lvl, int bitSection);
	virtual ~Target();

	virtual void put(const char* str, int lvl, const int bitSection) = 0;
	virtual const char* getBuffer();
};

} /* namespace Logging */

#endif /* TARGET_H_ */
