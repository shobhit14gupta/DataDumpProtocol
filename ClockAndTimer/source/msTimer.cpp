/*
 * msTimer.cpp
 *
 *  Created on: Apr 8, 2020
 *      Author: karsh
 */
#include "../header/msTimer.h"
#include "bits/pthreadtypes.h"

using namespace std;

msTimer::msTimer(mode_e mode,cb_t callback,void* callback_arg):base_timer(callback,callback_arg),mode(mode),threadID(0)
{

}

bool msTimer::create(){
	try{
		//create thread to run timer loop
		pthread_create( &threadID, NULL, evaluationLoop, (void*)this);
		if(threadID==0){
			throw -1;
			cout<<"thread failed to create"<<endl;
		}
		return true;
	}
	catch (const std::exception& e){

		return false;
	}
}




msTimer::~msTimer(){
	cout<< "msTimer destructor called"<<endl;
	pthread_cancel(threadID);
	pthread_join(threadID,NULL);
}




bool msTimer::start(uint64_t timeout_ms){
	bool ret = false;


	//check if timer is running or not
	if(this->isTimerRunning()==false && timeout_ms>0){
		//timer is not running, so we can start timer
		this->setTimeoutms(timeout_ms);
		this->setTimerStatus(true);
		this->startTimer();

		trigger_condVar.notify_one();

		ret = true;
	}else if(timeout_ms == 0){
		this->setTimeoutms(timeout_ms);
		this->setTimerStatus(false);
		ret = true;
	}


	return ret;
}


bool msTimer::stop(){
	bool ret = false;
	if(this->isTimerRunning()==true){
		this->setTimerStatus(false);
		this->setTimeoutms(0);
		ret = true;
	}
	return ret;
}

bool msTimer::isExpired(){
	return this->getExpiredIND();
}

void* msTimer::evaluationLoop(void* arg){

	//thread cancellation deferred until test cancel is executed
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    msTimer* instance = (msTimer*)arg;

	while(1){

		std::unique_lock<std::mutex> lck(instance->trigger_mutex);
		instance->trigger_condVar.wait(lck);

		while(instance->isTimerRunning()==true){
			if(instance->checkTimeOut()==true){
				//based on the mode of the time, restart or stop
				if(instance->mode==msTimer::PERIODIC){
					instance->start(instance->getTimeoutms());
				}else{
					instance->stop();
				}

				//set expiration indicator
				instance->setExpiredIND(true);

				//notify
				instance->notify();
			}
		pthread_testcancel();
		usleep(10000);
		}
	}
	return NULL;
}
