/*
 * msTimer.h
 *
 *  Created on: Apr 8, 2020
 *      Author: karsh
 */
#ifndef HEADER_MSTIMER_H_
#define HEADER_MSTIMER_H_

#include "msBaseTimer.h"

class msTimer:public base_timer{
public:
	typedef enum{
		ONE_SHOT=0,
		PERIODIC=1
	}mode_e;

	msTimer(mode_e mode,cb_t callback=NULL,void* callback_arg=NULL);
	~msTimer();
	bool create();
	bool start(uint64_t timeout_ms);
	bool stop();
	bool isExpired();
private:
	mode_e mode;
	pthread_t threadID;
	std::mutex trigger_mutex ;
	std::condition_variable  trigger_condVar ;
	static void* evaluationLoop(void*);
};




#endif /* HEADER_MSTIMER_H_ */
