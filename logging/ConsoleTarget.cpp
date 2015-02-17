/*
 * ConsoleTarget.cpp
 *
 *  Created on: 01.08.2012
 *      Author: Unknown
 */

#include "ConsoleTarget.h"
#include "LogLevel.h"
#include <iostream>

using namespace std;

namespace Logging {

ConsoleTarget::ConsoleTarget(const int lvl) : Target(lvl) {
	InitializeCriticalSection(&m_criticalSection);
}

ConsoleTarget::~ConsoleTarget() {
}

void ConsoleTarget::put(const char* str, const int lvl){
	if (lvl <= maxlvl) {
	    EnterCriticalSection(&m_criticalSection);
		switch (lvl) {
		case LogLevel::FATAL:	// Red
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
			break;
		case LogLevel::ERR0R:	// Red
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
			break;
		case LogLevel::WARNING:	// Yellow
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			break;
		case LogLevel::INFO:	// White
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			break;
		case LogLevel::DEBUG:	// Blue
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			break;
		default:				// Grey
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			break;
		}
		cout << str;
		cout.flush();
	    LeaveCriticalSection(&m_criticalSection);
	}
}

} /* namespace Logging */
