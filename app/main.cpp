//============================================================================
// Name        : Basic.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================


#include "../ClockAndTimer/TestNdoc/test.h"
#include "unistd.h"
#include "../dataDump/DDTest.h"
#include <iostream>
#include "../bTimer/btimerTest.h"
#include "../Logger/logger.h"




int main() {
	init_logging();
	//StopWatchTest();
	//msTimerUnitTest();
	DDTest();
	//btimerTest(NULL);
}
