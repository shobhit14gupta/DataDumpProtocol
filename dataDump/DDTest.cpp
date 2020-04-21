/*
 * DDTest.cpp
 *
 *  Created on: Apr 13, 2020
 *      Author: karsh
 */


#include "DDTest.h"
#include <pthread.h>
#include <semaphore.h>
#include "vector"
#include "queue"
#include "../Logger/logger.h"

using namespace std;

#define TRANSFER_SIZE 256

/*
 * Server related
 */
queue <vector<uint8_t>> serverInChannel;
queue <vector<uint8_t>> serverOutChannel;


// Declaration of thread condition variable
//condition_variable TXcond ;
//condition_variable RXcond ;

mutex inChannel_mutex ;
mutex outChannel_mutex ;

bool startNextTransfer = true;
int TransactionCount = 0;

//Memory storage emulation
uint8_t serverStorage[TRANSFER_SIZE]={0};
uint8_t* serverStoragePtr = serverStorage;
uint8_t serverIncStorage = 0;
uint8_t clientStorage[TRANSFER_SIZE]={0};





//===================================================================================
bool serverSend(uint8_t* arg,size_t length){

	bool ret = false;

	if(arg!=NULL && length !=0){

		//===== Critical section <
		{
			std::lock_guard<std::mutex> lck (outChannel_mutex);
			if(serverOutChannel.size()<20){
				//tracelog<< "Server: server Out channel sent"<<endl;
				serverOutChannel.emplace(arg,arg+length);
				ret = true;
			}
		}
		//===== Critical section >
	}
	return ret;
}

size_t serverReceive(uint8_t* arg,size_t maxLength){

	vector<uint8_t> _msg_;
	size_t ret = 0;

	if(arg!=NULL && maxLength !=0){

		//===== Critical section <
		{
			std::lock_guard<std::mutex> lck (inChannel_mutex);
			if(serverInChannel.size()!=0){
				//tracelog<< "Server: server In channel contains msg"<<endl;
				_msg_ = serverInChannel.front();
				serverInChannel.pop();
				if(_msg_.size()<=maxLength){
					std::copy(_msg_.begin(), _msg_.end(), arg);
					tracelog<< "Server: server In channel msg copied size:"<<_msg_.size()<<endl;
//					for(auto i=0;i<_msg_.size();i++){
//						tracelog<<"["<<dec<<i<<"]= 0x"<<dec<<(int)arg[i]<<" : ";
//					}
//					tracelog<<endl;
					ret = _msg_.size();
				}
			}
		}
		//===== Critical section >
	}
	return ret;
}


//===================================================================================
size_t clientReceive(uint8_t* arg,size_t maxLength){

	vector<uint8_t> _msg_;
	size_t ret = 0;


	if(arg!=NULL && maxLength !=0){

		tracelog<< "Client: server Out channel check for msg"<<endl;
		//===== Critical section <
		{
			std::lock_guard<std::mutex> lck (outChannel_mutex);
			if(serverOutChannel.size()!=0){
				tracelog<< "Client: server Out channel contains msg"<<endl;
				_msg_ = serverOutChannel.front();
				serverOutChannel.pop();
				if(_msg_.size()<=maxLength){
					std::copy(_msg_.begin(), _msg_.end(), arg);
					tracelog<< "Client: server Out channel msg copied, size:"<<_msg_.size()<<endl;
//					for(auto i=0;i<_msg_.size();i++){
//						tracelog<<"["<<dec<<i<<"]= 0x"<<dec<<(int)arg[i]<<" : ";
//					}
//					tracelog<<endl;
					ret = _msg_.size();
				}
			}
		}
		//===== Critical section >
	}else{
		errorlog<< "Client: Invalid arg"<<endl;
	}
	return ret;
}


bool clientSend(uint8_t* arg,size_t length){

	bool ret = false;

	if(arg!=NULL && length !=0){

		//===== Critical section <
		{
			std::lock_guard<std::mutex> lck (inChannel_mutex);
			if(serverInChannel.size()<20){
				//tracelog<< "Client: server In channel sent"<<endl;
				serverInChannel.emplace(arg,arg+length);
				ret = true;
			}
		}
		//===== Critical section >
	}
	return ret;
}


void DDTXRXTest()
{
	uint8_t sendBuffer[210]={0};
	uint8_t recvBuffer[210]={0};
	for(auto i=0;i<210;i++){
		sendBuffer[i]=i;
	}

	for(auto j=0;j<10;j++){
		tracelog<< "\nAttempt: "<<j<<endl;
		clientSend(&sendBuffer[j*20],20);
		serverReceive(&recvBuffer[j*20],20);
	}

	while(1){
		sleep(1);
	}
//	for(auto i=0;i<210;i++){
//		printf("[%d]=%d\n",i,recvBuffer[i]);
//	}
}

//================================================================================


size_t serverStorageReadcb(uint8_t* bufptr,size_t maxSize,bool eraseDataRead){

	if(maxSize>TRANSFER_SIZE){
		maxSize = TRANSFER_SIZE;
	}
	memcpy(bufptr,serverStorage,maxSize);
	return maxSize;
}


//================================================================================

void serverNotify(bool transferStatus){
	if(transferStatus==true){
		infolog<<"Server: transfer success"<<endl;

	}else{
		warnlog<<"Server: transfer failed"<<endl;
	}
}

void clientNotify(bool transferStatus){
	if(transferStatus==true){
		infolog<<"Client: Transfer success"<<endl;
		startNextTransfer = true;
//		//waiting for server thread to finish printing
//		sleep(1);
//		infolog<<"Client: Transfer dump  ==== >>  "<<endl;
//		for(auto i=0;i<TRANSFER_SIZE;i++){
//			cout<<"["<<(int)i<<"]="<<(int)clientStorage[i];
//			if(i%10==0)
//				cout<<endl;
//		}
//		cout<<"\n=============<< "<<endl;
	}else{
		warnlog<<"Client: Transfer failed"<<endl;
	}
}






//=================================================================
DDServer test_server("Server1",serverSend,serverReceive,0x0000,35,20,250,serverStorageReadcb,serverNotify);
DDClient test_client("Client1",clientSend,clientReceive,0x1234,35,20,250,NULL,clientNotify);


void* serverTaskloop(void* arg){
	while(1){
	test_server.FSMLoop();
	usleep(100);
	}
	return NULL;
}


void* clientTaskloop(void* arg){
	while(1){
	test_client.FSMLoop();
	usleep(100);
	}
	return NULL;
}

//================================================
pthread_t serverThread;
pthread_t clientThread;


void DDTest(){
//	DDTXRXTest();
	//fill storage buffer with data.
	for(uint16_t i=0;i<TRANSFER_SIZE;i++){
		serverStorage[i]=i;
	}

	pthread_create(&serverThread,NULL,serverTaskloop,NULL);
	pthread_create(&clientThread,NULL,clientTaskloop,NULL);

	if(test_server.startServer()==false){
		warnlog<<"Server: Fail to start"<<endl;
	}

	tracelog<< "Server: Started OK"<<endl;

	do{
		if(startNextTransfer){
			infolog<<"===================="<<"transaction counter"<<TransactionCount<<"==========================================="<<endl;

			if(test_client.startNewTransfer(clientStorage,TRANSFER_SIZE)==true){
				TransactionCount++;
				startNextTransfer= false;
			}else{
				warnlog<< "Client: Busy Can't start now"<<endl;
			}
		}
		sleep(10);
	}while(TransactionCount<10);

	tracelog<< "Client: Started OK"<<endl;

	pthread_join(serverThread,NULL);
}







