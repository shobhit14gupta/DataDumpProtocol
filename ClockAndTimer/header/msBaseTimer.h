/*
 * msBaseTimer.h
 *
 *  Created on: Apr 8, 2020
 *      Author: karsh
 */

#ifndef HEADER_MSBASETIMER_H_
#define HEADER_MSBASETIMER_H_

#include <stdbool.h>
#include <iostream>
#include <functional>
#include "Stopwatch.hpp"
#include "pthread.h"
#include <unistd.h>
#include <mutex>
#include <condition_variable>

namespace sw = stopwatch;

class base_timer{
public:
	typedef std::function<void(void* arg)> cb_t;

private:
	sw::Stopwatch* timerHandle;
	uint64_t timeoutms;
	cb_t callback;
	bool timerStatus;
	std::mutex resource_mutex;
	bool expired;
	void* callback_arg;
public:

	base_timer(cb_t callback=NULL,void* callback_arg=NULL):timeoutms(0), callback(callback),timerStatus(false),
	expired(false),callback_arg(callback_arg){
		try{
			timerHandle  = new sw::Stopwatch();
			if(timerHandle==NULL){
				//throw MEMORY_EXCEPTION;
			}
		}
		catch(std::exception& e){

		}
	}


	~base_timer(){
		if(timerHandle!=NULL){
			delete timerHandle;
		}
	}

	uint64_t getTimeoutms() {
		resource_mutex.lock();
		uint64_t _timeoutms_ =timeoutms;
		resource_mutex.unlock();
		return _timeoutms_;
	}

	void setTimeoutms(uint64_t timeoutms) {
		resource_mutex.lock();
		this->timeoutms = timeoutms;
		resource_mutex.unlock();
	}

	bool isTimerRunning() {
		resource_mutex.lock();
		bool _timerStatus_ =timerStatus;
		resource_mutex.unlock();
		return _timerStatus_;
	}

	void setTimerStatus(bool timerStatus) {
		resource_mutex.lock();
		this->timerStatus = timerStatus;
		resource_mutex.unlock();
	}

	void startTimer(){
		resource_mutex.lock();
		timerHandle->start();
		resource_mutex.unlock();
	}

	bool checkTimeOut(){
		bool ret = false;

		resource_mutex.lock();
		if(timerHandle->elapsed<sw::milliseconds>()>=timeoutms){
			ret = true;
		}
		resource_mutex.unlock();

		return ret;
	}

	void notify(){
		resource_mutex.lock();
		cb_t _callback_ = callback;
		void* _callback_arg_ = callback_arg;
		resource_mutex.unlock();

		if(_callback_!=NULL){
			_callback_(_callback_arg_);
		}
	}

	void setExpiredIND(bool ind){
		resource_mutex.lock();
		this->expired = ind;
		resource_mutex.unlock();
	}

	bool getExpiredIND(){
		resource_mutex.lock();
		bool _expired_ = this->expired;
		this->expired = false; //indicator is reset again (very important)
		resource_mutex.unlock();

		return _expired_;
	}

	bool registerCB(cb_t cb, void* cb_arg){
		if(cb==NULL)
			return false;

		resource_mutex.lock();
		callback = cb;
		callback_arg = cb_arg;
		resource_mutex.unlock();

		return true;
	}
};


#endif /* HEADER_MSBASETIMER_H_ */
