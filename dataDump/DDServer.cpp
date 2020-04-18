
#include "DDServer.h"

using namespace DD_PACKET;


/*
 * Server: State timer callback
 */
void DDServerStateTimerCallback(void* arg)
{
	//cout<<"Server: one-shot Timer expired"<<endl;
}



void DDServer::FSMLoop() {

	//**Detect state changes**
	if(this->getNextFSMState() != this->getCurrentFSMState()){
		this->setCurrentFSMState(this->getNextFSMState());
	}

	//**Update server running indicator based on current fsm state**
	if(this->getCurrentFSMState() != fsm_state_e::IDLE){
		this->setModuleStatus(true);
	}


	//FSM States
	switch(this->getCurrentFSMState()){

		//-- . -- . -- . -- . -- . -- . --
		case fsm_state_e::IDLE:
		{
			//** IDLE: state logic **
				//cout<<"Server: state: IDLE"<<endl;
				this->isTransferSuccess = false;
				this->stopStateTransitionTimer();
				this->clientConfig.setAckTimeout(0);
				this->clientConfig.setBlocksPerTransaction(0);
				this->clientConfig.setMaxPacketLength(0);
				this->clientConfig.setTransactionId(0);
				this->internalBufferLen = 0;

				//server is stopped (not running)
				this->setModuleStatus(false);

			//** IDLE: transition logic **

				//stay in this state until new server is started again
		}
		break;

		//-- . -- . -- . -- . -- . -- . --
		case fsm_state_e::REQUEST:
		{
			cout<<"Server: state: REQUEST"<<endl;
			bool stateRet = false;

			//** REQUEST: state logic **
				TransactionPacket request;
				size_t recvLen=0;

				/*receive REQUEST data in bytes
				 * Until non-zero bytes of REQUEST message type are received
				 * or oneshotTimer expires.
				 */
				do{
					recvLen = this->receive(request,msgType_e::REQUEST);
				}while((recvLen==0));

				if(recvLen!=0){
					//Info: valid message is received with matching msgtype and crc ok.

					/*Copy client configurations from the request
					 * 1. ACKTimeout
					 * 2. BlocksPerTransaction
					 * 3. MaxPacketLength
					 * 4. TransactionID
					 * */
					this->clientConfig.setAckTimeout(request.getHeader().getAckTimeout());
					this->clientConfig.setBlocksPerTransaction(request.getHeader().getBlocksPerTransaction());
					this->clientConfig.setMaxPacketLength(request.getHeader().getMaxPacketLength());
					this->clientConfig.setTransactionId(request.getHeader().getTransactionId());

					/** Report success */
					stateRet = true;
				}

			//** transition logic **
				if(stateRet){
					this->setNextFSMState(fsm_state_e::RESPONSE);
					this->startStateTransitionTimer(serverConfig.getAckTimeout());
				}else{
					this->setNextFSMState(fsm_state_e::TERMINATE);
				}
		}
		break;

		//-- . -- . -- . -- . -- . -- . --
		case fsm_state_e::RESPONSE:
		{
			cout<<"Server: state: RESPONSE"<<endl;
			bool stateRet = false;
			TransactionPacket response;

			//** state logic **

			/* Based on recieved Client Configuration, set desired response packet
			 * 1. Trasaction ID should be same as client configuration
			 * 2. Select smallest Block Per Transaction between client and server.
			 * 3. Select smallest Max Packet Length between client and server.
			 * 4. Select largest Timeout between client and server.
			 */
			response.getHeader().setMsgType(msgType_e::RESPONSE);
			response.getHeader().setTransactionId(this->clientConfig.getTransactionId());

			if(this->clientConfig.getBlocksPerTransaction() < this->serverConfig.getBlocksPerTransaction()){
				response.getHeader().setBlocksPerTransaction(this->clientConfig.getBlocksPerTransaction());
			}else{
				response.getHeader().setBlocksPerTransaction(this->serverConfig.getBlocksPerTransaction());
			}

			if(this->clientConfig.getMaxPacketLength() < this->serverConfig.getMaxPacketLength()){
				response.getHeader().setMaxPacketLength(this->clientConfig.getMaxPacketLength());
			}else{
				response.getHeader().setMaxPacketLength(this->serverConfig.getMaxPacketLength());
			}

			if(this->clientConfig.getAckTimeout() > this->serverConfig.getAckTimeout()){
				response.getHeader().setAckTimeout(this->clientConfig.getAckTimeout());
			}else{
				response.getHeader().setAckTimeout(this->serverConfig.getAckTimeout());
			}

			response.generateCRC();


			//update client configurations with new data sent in response
			this->clientConfig.setAckTimeout(response.getHeader().getAckTimeout());
			this->clientConfig.setBlocksPerTransaction(response.getHeader().getBlocksPerTransaction());
			this->clientConfig.setMaxPacketLength(response.getHeader().getMaxPacketLength());
			this->clientConfig.setTransactionId(response.getHeader().getTransactionId());


			/* Read data from storage WITHOUT ERASING and store in it buffer
			 * Data Transfer is only availed when data exists in storage.
			 */
			size_t maxStorageReadSizeReq = maxPacketSize - blockTxRxPacket::headerSize();
			maxStorageReadSizeReq=maxStorageReadSizeReq * clientConfig.getBlocksPerTransaction();
			if(consumerCB!=NULL && this->internalBuffer!=NULL){
				internalBufferLen = consumerCB(internalBuffer,maxStorageReadSizeReq,false);
				if(internalBufferLen!=0){
					stateRet = true;
				}
			}


			//send RESPONSE
			if(stateRet){
				do{
					stateRet = this->send(response,response.size());
					usleep(100);
				}while((stateRet==false) && (this->stateTransitionTimerHandle->isExpired()==false));
			}

			//transistion logic
			if(stateRet){
				this->setNextFSMState(fsm_state_e::INITIATE_BLOCK);
				this->startStateTransitionTimer(this->clientConfig.getAckTimeout());
			}else{
				this->setNextFSMState(fsm_state_e::TERMINATE);
			}
		}
		break;

		//-- . -- . -- . -- . -- . -- . --
		case fsm_state_e::INITIATE_BLOCK:
		{
			cout<<"Server: state: INITIATE_BLOCK"<<endl;
			bool stateRet = false;


			//** state logic **
				TransactionPacket initiate;
				size_t recvLen=0;

				/*receive until non- zero bytes of INITIATE BLOCK packet are received or
				 * oneshot timer is expired.
				 */
				do{
					recvLen= this->receive(initiate,msgType_e::INITIATE_BLOCK_TRANSFER);
				}while((recvLen==0) && (this->stateTransitionTimerHandle->isExpired()==false));

				//Validate the INITIATE BLOCK received.
				if(recvLen!=0){
					if(clientConfig.getTransactionId() == initiate.getHeader().getTransactionId()){
						stateRet = true;
					}
				}

			//** transistion logic **
			if(stateRet){
				this->setNextFSMState(fsm_state_e::BLOCK);
				this->startStateTransitionTimer(clientConfig.getAckTimeout());
				internal_counter = 1; //local counter is used as frame counter, set to zero.
			}else{
				this->setNextFSMState(fsm_state_e::TERMINATE);
			}
		}
		break;

		//-- . -- . -- . -- . -- . -- . --
		case fsm_state_e::BLOCK:
		{
			cout<<"Server: state: BLOCK"<<endl;
			bool stateRet = false;
			//bool timerExpiration = false;

			//** state logic **
			blockTxRxPacket blockDataPacket;
			frameNumber_t _frameCnt_ = internal_counter++;

			blockDataPacket.getHeader().setMsgType(msgType_e::BLOCK_TRANSFER);
			// set frame number
			blockDataPacket.getHeader().setFrameNumber(_frameCnt_);

			//Fill data buffer size and contents from read buffer
			size_t dataSizeCopiedToPacket = maxPacketSize - blockTxRxPacket::headerSize();
			if(internalBufferLen<dataSizeCopiedToPacket){
				dataSizeCopiedToPacket = internalBufferLen;
			}

			blockDataPacket.setBuffer(internalBuffer,dataSizeCopiedToPacket);
			internalBufferLen -= dataSizeCopiedToPacket;// decrement readBufferLen by dataSizeCopiedToPacket
			internalBuffer += dataSizeCopiedToPacket; //Increment readBuffer pointer, by dataSizeCopiedToPacket

			//check if this last frame
			if(_frameCnt_>= this->clientConfig.getBlocksPerTransaction() || internalBufferLen<=0){
				blockDataPacket.getHeader().setContinueBit(continueBit_e::TERMINATE_TRANSFER);
			}else{
				blockDataPacket.getHeader().setContinueBit(continueBit_e::CONTINUE_TRANSFER);
			}

			blockDataPacket.generateCRC();

			//send Initiate block transfer msg
			do{
				stateRet = this->send(blockDataPacket,blockDataPacket.size());
				usleep(100);
			}while((stateRet==false) && (this->stateTransitionTimerHandle->isExpired()==false));

			//IMPORTANT UN-COMMENT IN MICROCONTROLLER
			if(this->stateTransitionTimerHandle->isExpired()==false){
				do{
					usleep(100);
				}while(this->stateTransitionTimerHandle->isExpired()==false);
			}

			cout<<"wait over"<<endl;
			if(stateRet==true){
				cout<<"Server : Block Packet: "<<(int)_frameCnt_<<"sent"<<endl;

				//Check if this is last frame sent.
				if(blockDataPacket.getHeader().getContinueBit()==continueBit_e::TERMINATE_TRANSFER){
					//Info: Last frame is sent, so wait for ACK
					cout<<"Server : Block Packet: "<<(int)_frameCnt_<<" Last Frame sent"<<endl;

					//re-start state transition timer
					//this->startStateTransitionTimer(clientConfig.getAckTimeout());

					//receive until non-zero byte of ACK packet or oneshot timer expires
					TransactionPacket ACKPacket;
					size_t recvLen=0;

					do{
						recvLen= this->receive(ACKPacket,msgType_e::BLOCK_ACK);
						cout<< "waiting for ACK"<<endl;
					}while((recvLen==0) && (this->stateTransitionTimerHandle->isExpired()==false));

					if(recvLen!=0){
						//Info: received ACKOK
						//Check if storageReadCB is not NULL and Validate ACK packet based on trasaction ID.
						//erase the data from storage application
						if((consumerCB!=NULL) &&
								(ACKPacket.getHeader().getTransactionId() == clientConfig.getTransactionId())){
							if(consumerCB(internalBuffer,internalBufferLen,true)==internalBufferLen){
								//Inform app about transfer success
								isTransferSuccess = true;
							}
						}
					}

					//Whether transfer is success or not, we need to terminate here.
					this->setNextFSMState(fsm_state_e::TERMINATE);
				}
				else{
					//Info: This is not last frame, continue in this state
					//start state transition timer and continue to be in this state
					this->setNextFSMState(this->getCurrentFSMState());
					this->startStateTransitionTimer(clientConfig.getAckTimeout());
				}
			}else{
				//something went wrong, send failed, so terminate.
				this->setNextFSMState(fsm_state_e::TERMINATE);
			}
		}
		break;

		//-- . -- . -- . -- . -- . -- . --
		case fsm_state_e::TERMINATE:
		{
			cout<<"Server: state: TERMINATE"<<endl;
			//** Stop state transition timer **
			this->stopStateTransitionTimer();

			//** Notify application about transfer status **
				if(this->notifyCB!=NULL){
					if(isTransferSuccess){
						notifyCB(true);
					}else{
						notifyCB(false);
					}
				}

			//** cleanup **
				this->isTransferSuccess = false;
				this->stopStateTransitionTimer();
				this->clientConfig.setAckTimeout(0);
				this->clientConfig.setBlocksPerTransaction(0);
				this->clientConfig.setMaxPacketLength(0);
				this->clientConfig.setTransactionId(0);
				this->internalBufferLen = 0;


			 //** transistion logic **
				this->setNextFSMState(fsm_state_e::REQUEST);
		}
		break;

		//-- . -- . -- . -- . -- . -- . --
		default:
		{
			this->setNextFSMState(fsm_state_e::TERMINATE);
		}


	}
}



DDServer::DDServer(string name,DD_send_cb_t sendcb,DD_receive_cb_t recvcb,DD_PACKET::transactionID_t transactionID,
		DD_PACKET::blocksPerTransaction_t blocksPerTransaction,DD_PACKET::maxPacketLen_t maxPacketlen,
		DD_PACKET::ACKTimeout_t ackTimeout,DD_consumer_cb_t storageRead,DD_notify_cb_t appStatusCB):
		DataDump(mode_e::SERVER,name,sendcb,recvcb,transactionID,blocksPerTransaction,
						maxPacketlen,ackTimeout,storageRead,appStatusCB)
{
	try{
		size_t maxStorageReadSizeReq = maxPacketlen - blockTxRxPacket::headerSize();
		maxStorageReadSizeReq = maxStorageReadSizeReq * serverConfig.getBlocksPerTransaction();
		this->internalBuffer = new uint8_t[maxStorageReadSizeReq];
		if(this->internalBuffer==NULL){
			//throw error
		}

		if(this->stateTransitionTimerHandle->registerCB(DDServerStateTimerCallback,this)==false){
			cout<<"Server one shot timer error"<<endl;
		}
	}
	catch(std::exception &e){

	}


}

DDServer::~DDServer() {
	if(this->internalBuffer!=NULL){
		delete [] this->internalBuffer;
	}
}

bool DDServer::startServer() {

	bool ret = false;
	if(this->isModuleBusy() == false){
		setNextFSMState(fsm_state_e::REQUEST);
		ret = true;
	}

	return ret;
}

bool DDServer::stopServer() {

	bool ret = false;

	//wait till server comes to REQUEST state
	while(getCurrentFSMState()!=fsm_state_e::REQUEST){
		//do nothing, just wait
		usleep(100);
	}

	//push to TERMINATE state
	setNextFSMState(fsm_state_e::IDLE);

	return ret;
}






