/*
 * Target.cpp
 *
 *  Created on: 01.08.2012
 *      Author: Unknown
 */

#include "Target.h"

namespace Logging {

Target::Target(int lvl) : maxlvl(lvl) {

}

Target::~Target() {
	// Auto-generated destructor stub
}

const char* Target::getBuffer() {
	return 0;
}

} /* namespace Logging */
