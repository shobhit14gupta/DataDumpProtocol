
#include "DDServer.h"
#include "../Logger/logger.h"

using namespace DD_PACKET;



void DDServer::FSMLoop() {
	try{
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
					tracelog<<"Server: state: IDLE"<<endl;

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
				tracelog<<"Server: state: REQUEST"<<endl;
				bool stateRet = false;

				//** REQUEST: state logic **
					TransactionPacket request;
					size_t recvLen=0;

					/*receive REQUEST data in bytes
					 * Until non-zero bytes of REQUEST message type are received
					 * or oneshotTimer expires.
					 */
					tracelog<<"Server: REQUEST: waiting for request"<<endl;
					do{
						recvLen = this->receive(request,msgType_e::REQUEST);
						usleep(10);
					}while((recvLen==0));

					tracelog<<"Server: REQUEST: request received"<<endl;

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
						this->clientConfig.setMaxPacketLength(request.getHeader().getPacketLength());
						this->clientConfig.setTransactionId(request.getHeader().getTransactionId());

						/** Report success */
						stateRet = true;
					}

				//** transition logic **
					if(stateRet){
						tracelog<<"Server: REQUEST: client configuration received and saved"<<endl;
						this->setNextFSMState(fsm_state_e::RESPONSE);
						this->startStateTransitionTimer(serverConfig.getAckTimeout());
					}else{
						warnlog<<"Server: REQUEST: Fail to receive or save client configuration"<<endl;
						this->setNextFSMState(fsm_state_e::TERMINATE);
					}
			}
			break;

			//-- . -- . -- . -- . -- . -- . --
			case fsm_state_e::RESPONSE:
			{
				tracelog<<"Server: state: RESPONSE"<<endl;
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

				if(this->clientConfig.getPacketLength() < this->serverConfig.getPacketLength()){
					response.getHeader().setMaxPacketLength(this->clientConfig.getPacketLength());
				}else{
					response.getHeader().setMaxPacketLength(this->serverConfig.getPacketLength());
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
				this->clientConfig.setMaxPacketLength(response.getHeader().getPacketLength());
				this->clientConfig.setTransactionId(response.getHeader().getTransactionId());


				/* Read data from storage WITHOUT ERASING and store in it buffer
				 * Data Transfer is only availed when data exists in storage.
				 */

				size_t maxStorageReadSizeReq = maxPacketSize - blockTxRxPacket::getHeaderSize();
				maxStorageReadSizeReq=maxStorageReadSizeReq * clientConfig.getBlocksPerTransaction();
				if(consumerCB!=NULL && this->internalBuffer!=NULL){
					internalBufferLen = consumerCB(internalBuffer,maxStorageReadSizeReq,false);
					tracelog<<"Server: RESPONSE: "<< internalBufferLen<<"(Bytes) read to be dumped"<<endl;
					if(internalBufferLen!=0){
						stateRet = true;
					}
				}


				//send RESPONSE
				if(stateRet){
					tracelog<<"Server: RESPONSE: sending response packet"<<endl;
					do{
						stateRet = this->send(response,response.getPacketSize());
						usleep(10);
					}while((stateRet==false) && (this->stateTransitionTimerHandle->isExpired()==false));
				}

				//transistion logic
				if(stateRet){
					tracelog<<"Server: RESPONSE: Response packet sent"<<endl;
					this->setNextFSMState(fsm_state_e::INITIATE_BLOCK);
					this->startStateTransitionTimer(this->clientConfig.getAckTimeout());
				}else{
					warnlog<<"Server: RESPONSE: Fail to send Response packet"<<endl;
					this->setNextFSMState(fsm_state_e::TERMINATE);
				}
			}
			break;

			//-- . -- . -- . -- . -- . -- . --
			case fsm_state_e::INITIATE_BLOCK:
			{
				tracelog<<"Server: state: INITIATE_BLOCK"<<endl;
				bool stateRet = false;


				//** state logic **
					TransactionPacket initiate;
					size_t recvLen=0;

					/*receive until non- zero bytes of INITIATE BLOCK packet are received or
					 * oneshot timer is expired.
					 */
					tracelog<<"Server: INITIATE_BLOCK: waiting for initiate packet"<<endl;
					do{
						recvLen= this->receive(initiate,msgType_e::INITIATE_BLOCK_TRANSFER);
						usleep(10);
					}while((recvLen==0) && (this->stateTransitionTimerHandle->isExpired()==false));

					//Validate the INITIATE BLOCK received.
					if(recvLen!=0){
						if(clientConfig.getTransactionId() == initiate.getHeader().getTransactionId()){
							tracelog<<"Server: INITIATE_BLOCK: received initiate packet, terms agreed"<<endl;
							stateRet = true;
						}
					}

				//** transistion logic **
				if(stateRet){
					tracelog<<"Server: INITIATE_BLOCK: Initiate received, Transaction ACCEPTED"<<endl;
					this->setNextFSMState(fsm_state_e::BLOCK);
					this->startStateTransitionTimer(clientConfig.getAckTimeout());
					internal_counter = 1; //local counter is used as frame counter, set to zero.
				}else{
					warnlog<<"Server: INITIATE_BLOCK: Fail to receive Initiate or Transaction DECLINED"<<endl;
					this->setNextFSMState(fsm_state_e::TERMINATE);
				}
			}
			break;

			//-- . -- . -- . -- . -- . -- . --
			case fsm_state_e::BLOCK:
			{
				tracelog<<"Server: state: BLOCK"<<endl;
				bool stateRet = false;
				//bool timerExpiration = false;

				//** state logic **
				blockTxRxPacket blockDataPacket;
				frameNumber_t _frameCnt_ = internal_counter++;

				blockDataPacket.getHeader().setMsgType(msgType_e::BLOCK_TRANSFER);
				// set frame number
				blockDataPacket.getHeader().setFrameNumber(_frameCnt_);

				//Fill data buffer size and contents from read buffer
				size_t dataSizeCopiedToPacket = maxPacketSize - blockTxRxPacket::getHeaderSize();
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

				tracelog<<"Server: BLOCK: Sending Block packets: framecount Number="<<(int)_frameCnt_<<endl;
				//send Initiate block transfer msg
				stateRet = false;
				do{
					if(stateRet==false){
						stateRet = this->send(blockDataPacket,blockDataPacket.getPacketSize());
					}
					usleep(1);
				}while(stateRet==false && this->stateTransitionTimerHandle->isExpired()==false);


				if(stateRet){
					tracelog<<"Server : BLOCK: "<<(int)_frameCnt_<<"sent"<<endl;

					//Check if this is last frame sent.
					if(blockDataPacket.getHeader().getContinueBit()==continueBit_e::TERMINATE_TRANSFER){
						//Info: Last frame is sent, so wait for ACK
						tracelog<<"Server : BLOCK: "<<(int)_frameCnt_<<" all frames sent"<<endl;

						//re-start state transition timer
						//this->startStateTransitionTimer(clientConfig.getAckTimeout());

						//receive until non-zero byte of ACK packet or oneshot timer expires
						TransactionPacket ACKPacket;
						size_t recvLen=0;

						do{
							recvLen= this->receive(ACKPacket,msgType_e::BLOCK_ACK);
							tracelog<< "Server: BLOCK: waiting for ACK packet"<<endl;
							usleep(10);
						}while((recvLen==0) && (this->stateTransitionTimerHandle->isExpired()==false));

						if(recvLen!=0){
							tracelog<< "Server: BLOCK: ACK packet received"<<endl;
							//Info: received ACKOK
							//Check if storageReadCB is not NULL and Validate ACK packet based on trasaction ID.
							//erase the data from storage application
							if((consumerCB!=NULL) &&
									(ACKPacket.getHeader().getTransactionId() == clientConfig.getTransactionId())){

								tracelog<< "Server: BLOCK: Erasing sent data from storage device"<<endl;
								if(consumerCB(internalBuffer,internalBufferLen,true)==internalBufferLen){
									//Inform app about transfer success
									isTransferSuccess = true;
								}
							}
						}else{
							warnlog<< "Server: BLOCK: Fail to receive ACK packet"<<endl;
						}

						//Whether transfer is success or not, we need to terminate here.
						this->setNextFSMState(fsm_state_e::TERMINATE);
					}
					else{
						//Info: This is not last frame, continue in this state
						//start state transition timer and continue to be in this state
						tracelog<< "Server: BLOCK: More packet to be sent"<<endl;
						this->setNextFSMState(this->getCurrentFSMState());
						this->startStateTransitionTimer(clientConfig.getAckTimeout());
					}
				}else{
					//something went wrong, send failed, so terminate.
					warnlog<< "Server: BLOCK: Something went wrong"<<endl;
					this->setNextFSMState(fsm_state_e::TERMINATE);
				}
			}
			break;

			//-- . -- . -- . -- . -- . -- . --
			case fsm_state_e::TERMINATE:
			{
				tracelog<<"Server: state: TERMINATE"<<endl;
				//** Stop state transition timer **
				this->stopStateTransitionTimer();

				//** Notify application about transfer status **
					if(this->notifyCB!=NULL){
						tracelog<<"Server: TERMINATE: Notified application"<<endl;
						if(isTransferSuccess){
							notifyCB(true);
						}else{
							notifyCB(false);
						}
					}

				//** cleanup **
					tracelog<<"Server: TERMINATE: cleanup"<<endl;
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
				tracelog<<"Server: UNKNOWN STATE"<<endl;
				this->setNextFSMState(fsm_state_e::TERMINATE);
			}


		}
	}
	catch(std::exception& e){
		errorlog<<e.what()<<endl;
	}
	catch(emu_err_t& e){
		exceptlog;
		errorlog<<endl;
		return ;
	}
}



DDServer::DDServer(string name,timer_loop_t& timerloop,DD_send_cb_t sendcb,DD_receive_cb_t recvcb,DD_PACKET::transactionID_t transactionID,
		DD_PACKET::blocksPerTransaction_t blocksPerTransaction,DD_PACKET::maxPacketLen_t maxPacketlen,
		DD_PACKET::ACKTimeout_t ackTimeout,DD_consumer_cb_t storageRead,DD_notify_cb_t appStatusCB):
		DataDump(mode_e::SERVER,name,timerloop,sendcb,recvcb,transactionID,blocksPerTransaction,
						maxPacketlen,ackTimeout,storageRead,appStatusCB)
{
	try{
		size_t maxStorageReadSizeReq = maxPacketlen - blockTxRxPacket::getHeaderSize();
		maxStorageReadSizeReq = maxStorageReadSizeReq * serverConfig.getBlocksPerTransaction();
		this->internalBuffer = new uint8_t[maxStorageReadSizeReq];
		if(this->internalBuffer==NULL){
			//throw error
		}

//		if(this->stateTransitionTimerHandle->registerCB(DDServerStateTimerCallback,this)==false){
//			cout<<"Server one shot timer error"<<endl;
//		}
	}
	catch(std::exception& e){
		errorlog<<e.what()<<endl;
	}
	catch(emu_err_t& e){
		exceptlog;
		errorlog<<endl;
		return ;
	}
}

DDServer::~DDServer() {
	if(this->internalBuffer!=NULL){
		delete [] this->internalBuffer;
	}
}

bool DDServer::startServer() {
	try{
		bool ret = false;
		if(this->isModuleBusy() == false){
			setNextFSMState(fsm_state_e::REQUEST);
			ret = true;
		}

		return ret;
	}
	catch(std::exception& e){
		errorlog<<e.what()<<endl;
		return false;
	}
	catch(emu_err_t& e){
		exceptlog;
		errorlog<<endl;
		return false;
	}
}

bool DDServer::stopServer() {
	try{
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
	catch(std::exception& e){
		errorlog<<e.what()<<endl;
		return false;
	}
	catch(emu_err_t& e){
		exceptlog;
		errorlog<<endl;
		return false;
	}
}






