/*
 * bTimer.cpp
 *
 *  Created on: Apr 17, 2020
 *      Author: karsh
 */
#include "bTimer.h"
#include "../Logger/logger.h"


using namespace std;

//======== friend functions ==============
void* serviceTaskLoop(void* arg){

	bool ret = true;
	btimer* instance = NULL;



	//arg check
	if(arg==NULL){
		ret = false;
	}

	//instance pointer check
	if(ret){
		instance = (btimer*)arg;
		//===== Critical section <
		{
			std::lock_guard<std::mutex> lck (instance->resProtect);
			if((instance->service==NULL) || (instance->handle==NULL)){
				ret = false;
			}
		}
		//===== Critical section >
	}


	//log message
	tracelog<<instance->getName()<<": Type: "<<((instance->mode==0)?"ONESHOT":"PERIODIC")<<": service initiated";

	//blocking call, so while(true) is not required here
	if(ret){
		instance->service->run();
	}

	//log message
	tracelog<<instance->getName()<<": Type: "<< ((instance->mode==0)?"ONESHOT":"PERIODIC")<<": service terminated";

	//exit thread gracefully
	pthread_exit(NULL);

    return NULL;
}

const std::string& btimer::getName() const {
	return name;
}

void timeoutCB(btimer* arg){

	bool ret = true;
	bool didTimeoutOccur = false;
	//arg check
	if(arg==NULL){
		ret = false;
	}

	//pointer check
	if(ret){
		//===== Critical section <
		{
			std::lock_guard<std::mutex> lck (arg->resProtect);
			if(arg->handle==NULL){
				ret = false;
			}

			if(ret){
				if (arg->handle->expires_at() <= boost::asio::deadline_timer::traits_type::now()){
					didTimeoutOccur = true;
				}
			}
		}
		//===== Critical section >
	}


	if(ret){
		if (didTimeoutOccur==true){


			//===== Critical section <
			{
				std::lock_guard<std::mutex> lck (arg->resProtect);

				tracelog<<arg->getName()<<"timeout called"<<endl;
				arg->timerExpiry = true;

				if(arg->notifyCB!=NULL){
					arg->notifyCB(arg->notifyCBarg);
				}
			}
			//===== Critical section >

			ret = arg->restart();
		}
	}


}

//=======================================================================



btimer::btimer(string name,mode_e mode,cb_t callback,void* callback_arg):
		name(name),mode(mode),handle(NULL),service(NULL),duration(1),notifyCB(callback),notifyCBarg(callback_arg),threadID(0),timerExpiry(false){


}


btimer::~btimer(){
	//===== Critical section <
	{
		std::lock_guard<std::mutex> lck (resProtect);

		//check if service is stopped
		if(service!=NULL){
			if(service->stopped()==false){
				service->reset();
			}

			//wait for service thread to terminate/
			if(threadID!=0){
				pthread_join(threadID,NULL);
			}

			//delete service
			delete service;
		}

		//delete allocated timer
		if(handle!=NULL){
			delete handle;
		}
	}
	//===== Critical section >

};


bool btimer::start(uint64_t intervalms){
	bool ret = true;

	//cout<<"In start"<<endl;
	stop();

	//=== Critical section start=======
	{
		std::lock_guard<std::mutex> lck (resProtect);

		if(ret){
			//set expiry time
			duration = intervalms;

			timerExpiry = false;

			//create service
			service = new boost::asio::io_service;
			if(service==NULL){
				errorlog<<this->getName()<<": service creation error"<<endl;
				ret = false;
			}
		}

		if(ret){
			//create deadline timer instance and also set timeout
			handle = new boost::asio::deadline_timer(*service,boost::posix_time::milliseconds(duration));
			if(handle==NULL){
				errorlog<<this->getName()<<": Timer creation error"<<endl;
				ret = false;
			}
		}

		if(ret){
			//attach timeout with async wait
			if(handle!=NULL && notifyCB!=NULL){
				handle->async_wait(boost::bind(timeoutCB,this));
			}

			//create a thread to run io service.
			pthread_create(&threadID,NULL,serviceTaskLoop,(void*)this);
			if(threadID==0){
				errorlog<<this->getName()<<": Thread creation error"<<endl;
				ret = false;
			}
		}
	}
	//=== Critical section end=======

	//cout<<"out start"<<endl;
	return ret;
}

bool btimer::restart(){
	bool ret = true;
	//cout<<"in restart"<<endl;
	//=== Critical section start=======
	{
		std::lock_guard<std::mutex> lck (resProtect);

		//pointer check
		if(ret){
			if((service==NULL) || (handle==NULL)){
				ret = false;
			}
		}


		if(ret){
			if(this->mode == btimer::mode_e::PERIODIC){
				timerExpiry = false;

				handle->expires_from_now(boost::posix_time::milliseconds(duration));

				if(handle!=NULL && notifyCB!=NULL){
					handle->async_wait(boost::bind(timeoutCB,this));
				}
			}
		}
	}
	//=== Critical section end=======
	//cout<<"out restart"<<endl;
	return ret;
}



bool btimer::stop(){

	bool ret = true;
	bool resetDone = false;
	//cout<<"in stop"<<endl;
	//=== Critical section start=======
	{
		std::lock_guard<std::mutex> lck (resProtect);

		//pointer check
		if(ret){
			if((service==NULL) || (handle==NULL)){
				ret = false;
			}
		}

		if(ret){
			if(service->stopped()==false){
				service->stop();
				resetDone = true;
			}
		}
	}
	//=== Critical section end=======

	if(resetDone){
		pthread_join(threadID,NULL);
	}


	//....service termination initiated....
	if(ret){
		//===== Critical section <
		{
			std::lock_guard<std::mutex> lck (resProtect);

			//check if service is stopped
			if(service!=NULL){
				delete service;
				service=NULL;
			}

			//delete timer
			if(handle!=NULL){
				delete handle;
				handle =NULL;
			}
		}
		//===== Critical section >
	}
	//cout<<"out restart"<<endl;
	return true;
}

bool btimer::isExpired(){
	bool temp = false;

	//=== Critical section start=======
	{
		std::lock_guard<std::mutex> lck (resProtect);
		temp = timerExpiry;
	}
	//=== Critical section end=======

	return temp;
}



bool btimer::registerCB(cb_t callback, void *callback_arg) {
	bool ret = false;

	//=== Critical section start=======
	{
		std::lock_guard<std::mutex> lck (resProtect);
		if(callback!=NULL){
			this->notifyCB = callback;
			this->notifyCBarg = callback_arg;
			ret = true;
		}
	}
	//=== Critical section end=======

	return ret;

}

