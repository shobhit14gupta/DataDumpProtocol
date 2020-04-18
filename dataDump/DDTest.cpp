/*
 * DDTest.cpp
 *
 *  Created on: Apr 13, 2020
 *      Author: karsh
 */


#include "DDTest.h"
#include <pthread.h>
#include <semaphore.h>

using namespace std;

#define TRANSFER_SIZE 256
/*
 * Server related
 */
uint8_t serverTXChannel[20]={0};
uint8_t serverRXChannel[20]={0};

// Declaration of thread condition variable
condition_variable TXcond ;
condition_variable RXcond ;

mutex TXlock ;
mutex RXlock ;

pthread_t serverThread;
pthread_t clientThread;

uint8_t serverStorage[TRANSFER_SIZE]={0};
uint8_t* serverStoragePtr = serverStorage;
uint8_t serverIncStorage = 0;

uint8_t clientStorage[TRANSFER_SIZE]={0};

DDServer* server = NULL;
DDClient* client = NULL;



bool serverSend(uint8_t* arg,size_t length){

	memcpy(serverTXChannel,arg,length);
	cout<< "server sent"<<endl;
	TXcond.notify_all();

	return true;
}

size_t clientReceive(uint8_t* arg,size_t maxLength){
	std::unique_lock<std::mutex> lck(TXlock);
	if(TXcond.wait_for(lck, std::chrono::milliseconds(10))==std::cv_status::timeout){
		return 0;
	}
	cout<< "Client received"<<endl;
	memcpy(arg,serverTXChannel,20);

	return 20;
}



size_t serverReceive(uint8_t* arg,size_t maxLength){

	std::unique_lock<std::mutex> lck(RXlock);
	if(RXcond.wait_for(lck,std::chrono::milliseconds(10))==std::cv_status::timeout){
		return 0;
	}
	cout<< "server received"<<endl;
	memcpy(arg,serverRXChannel,20);
	return 20;
}

bool clientSend(uint8_t* arg,size_t length){
	memcpy(serverRXChannel,arg,length);
	cout<< "client sent"<<endl;
	RXcond.notify_all();
	return true;
}





size_t serverStorageReadcb(uint8_t* bufptr,size_t maxSize,bool eraseDataRead){

	if(maxSize>TRANSFER_SIZE){
		maxSize = TRANSFER_SIZE;
	}
	memcpy(bufptr,serverStorage,maxSize);
	return maxSize;
}




void serverNotify(bool transferStatus){
	if(transferStatus==true){
		cout<<"Server transfer success"<<endl;

	}else{
		cout<<"Server transfer failed"<<endl;
	}
}

void clientNotify(bool transferStatus){
	if(transferStatus==true){
		cout<<"Client transfer success"<<endl;
		for(auto i=0;i<TRANSFER_SIZE;i++){
			printf("**[%d]=%d",i,clientStorage[i]);
		}
	}else{
		cout<<"Client transfer failed"<<endl;
	}
}



DDServer test_server("server1",serverSend,serverReceive,0x0000,35,20,250,serverStorageReadcb,serverNotify);
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


void DDTest(){
	//fill storage buffer with data.
	for(uint16_t i=0;i<TRANSFER_SIZE;i++){
		serverStorage[i]=i;
	}

	pthread_create(&serverThread,NULL,serverTaskloop,NULL);
	pthread_create(&clientThread,NULL,clientTaskloop,NULL);

	if(test_server.startServer()==false){
		cout<<"server fail to start"<<endl;
	}

	cout<< "server started OK"<<endl;


	if(test_client.startNewTransfer(clientStorage,TRANSFER_SIZE)==false){
		cout<< "client fail to start"<<endl;
	}

	cout<< "client started OK"<<endl;

	while(1){
		sleep(100);
	}
}







