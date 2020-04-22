/*
 * bTimer.cpp
 *
 *  Created on: Apr 17, 2020
 *      Author: karsh
 */
#include "bTimer.h"
#include "../Logger/logger.h"
#include "../ErrorHandler/errorHandler.h"


using namespace std;
void timeoutCB(btimer* arg) noexcept;


btimer::btimer(string name,boost::asio::io_service& service,mode_e mode,cb_t callback,void* callback_arg):
		name(name),mode(mode),handle(NULL),duration(1),notifyCB(callback),notifyCBarg(callback_arg),
		threadID(0),timerExpiry(false){
}


btimer::~btimer(){
	stop();
};


bool btimer::start(uint64_t intervalms) noexcept{
	bool ret = false;

	stop();

	//=== Critical section start=======
	{
		std::unique_lock<std::timed_mutex> lck (resProtect,std::defer_lock);
		if(lck.try_lock_for(std::chrono::microseconds(100))){

			//set expiry time
			duration = intervalms;
			timerExpiry = false;

			//create deadline timer instance and also set timeout
			handle = new boost::asio::deadline_timer(service,boost::posix_time::milliseconds(duration));
			if(handle!=NULL){
				//attach timeout with async wait
				handle->async_wait(boost::bind(timeoutCB,this));
				ret = true;
			}

		}
	}
	//=== Critical section end=======
	return ret;
}



bool btimer::restart() noexcept{
	bool ret = false;

	//=== Critical section start=======
	{
		std::unique_lock<std::timed_mutex> lck (resProtect,std::defer_lock);
		if(lck.try_lock_for(std::chrono::microseconds(100))){

			if(handle!=NULL){
				if(this->mode == btimer::mode_e::PERIODIC){
					handle->expires_from_now(boost::posix_time::milliseconds(duration));
					handle->async_wait(boost::bind(timeoutCB,this));
				}
				ret = true;
			}

		}
	}
	//=== Critical section end=======

	return ret;
}



bool btimer::stop() noexcept{

	bool ret = false;
	//=== Critical section start=======
	{
		std::unique_lock<std::timed_mutex> lck (resProtect,std::defer_lock);
		if(lck.try_lock_for(std::chrono::microseconds(100))){

			//Cancel any asynchronous operations that are waiting on the timer
			if(handle!=NULL){
				bool didTimeoutOccur =(handle->expires_at() <= boost::asio::deadline_timer::traits_type::now())?true:false;
				if(didTimeoutOccur==false){
					// timer is running, so it sensible to cancel it
					handle->cancel();
				}

				delete handle;
				handle=NULL;
				ret = true;
			}

		}

	}
	//=== Critical section end=======
	return ret;
}




bool btimer::isExpired() noexcept{
	bool temp = false;
	//=== Critical section start=======
	{
		std::unique_lock<std::timed_mutex> lck (resProtect,std::defer_lock);
		if(lck.try_lock_for(std::chrono::microseconds(100))){
			temp = timerExpiry;
			timerExpiry = false;
		}
	}
	//=== Critical section end=======

	return temp;
}



bool btimer::registerCB(cb_t callback, void *callback_arg) noexcept{
	bool ret = false;

	//=== Critical section start=======
	{
		std::unique_lock<std::timed_mutex> lck (resProtect,std::defer_lock);
		if(lck.try_lock_for(std::chrono::microseconds(100))){
			if(callback!=NULL){
				this->notifyCB = callback;
				this->notifyCBarg = callback_arg;
				ret = true;
			}
		}
	}
	//=== Critical section end=======

	return ret;
}

const std::string& btimer::getName() const {
	return name;
}

void timeoutCB(btimer* arg) noexcept{

	bool ret = false;
	bool didTimeoutOccur = false;

	//pointer check
	if(arg!=NULL){
		//===== Critical section <
		{
			std::unique_lock<std::timed_mutex> lck (arg->resProtect,std::defer_lock);
			if(lck.try_lock_for(std::chrono::microseconds(100))){
				if(arg->handle!=NULL){
					didTimeoutOccur =(arg->handle->expires_at() <= boost::asio::deadline_timer::traits_type::now())?true:false;
					arg->timerExpiry = didTimeoutOccur;
					ret = true;
				}
			}
		}
		//===== Critical section >
	}

	if(ret){
		if (didTimeoutOccur==true){
			if(arg->notifyCB!=NULL){
				arg->notifyCB(arg->notifyCBarg);
			}
			ret = arg->restart();
		}
	}
}

