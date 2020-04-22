/*
 * DDClient.cpp
 *
 *  Created on: Apr 13, 2020
 *      Author: karsh
 */
#include "DDClient.h"
#include "../Logger/logger.h"

using namespace DD_PACKET;


//============ CLIENT ==================
void DDClient::FSMLoop() {

	try{
		//**Detect state changes**
		if(this->getNextFSMState() != this->getCurrentFSMState()){
			this->setCurrentFSMState(this->getNextFSMState());
		}


		//**Update Client running indicator based on current fsm state**
		if(this->getCurrentFSMState() != fsm_state_e::IDLE){
			this->setModuleStatus(true);
		}

		switch(this->getCurrentFSMState()){

			//-- . -- . -- . -- . -- . -- . --
			case fsm_state_e::IDLE:
			{
				//tracelog<<"Client: state: IDLE"<<endl;

				//** IDLE: state logic **

					//set client as available (not busy)
					this->setModuleStatus(false);

				//** IDLE: transition logic **
					//stay in this state until new transfer is initiated

			}
			break;

			//-- . -- . -- . -- . -- . -- . --
			case fsm_state_e::REQUEST:
			{
				tracelog<<"Client: state: REQUEST"<<endl;
				bool stateRet = true;

				//** REQUEST: state logic **

					//start state timer
					if(stateRet){
						stateRet = this->startStateTransitionTimer(200);
					}

					//update transaction ID
	//				if(stateRet){
	//					clientConfig.setTransactionId(localCounter);
	//					localCounter++;
	//				}

					/* create REQUEST packet from following data
					 * 1. ACKTimeout
					 * 2. BlocksPerTransaction
					 * 3. MaxPacketLength
					 * 4. TransactionID
					 * 5. CRC
					 *
					 * Send the REQUEST Packet, also update stateRet
					 */
					if(stateRet){
						TransactionPacket request;
						request.getHeader().setMsgType(msgType_e::REQUEST);
						request.getHeader().setAckTimeout(clientConfig.getAckTimeout());
						request.getHeader().setBlocksPerTransaction(clientConfig.getBlocksPerTransaction());
						request.getHeader().setMaxPacketLength(clientConfig.getPacketLength());
						request.getHeader().setTransactionId(clientConfig.getTransactionId());
						request.generateCRC();

						do{
							stateRet = this->send(request,request.getPacketSize());
							usleep(10);
						}while((stateRet==false) && (this->stateTransitionTimerHandle->isExpired()==false));
					}


				//** transition logic **
					if(stateRet){
						tracelog<<"Client: REQUEST: Request sent"<<endl;
						this->setNextFSMState(fsm_state_e::RESPONSE);
						this->startStateTransitionTimer(clientConfig.getAckTimeout());
					}else{
						warnlog<<"Client: REQUEST: Failed to send"<<endl;
						this->setNextFSMState(fsm_state_e::TERMINATE);
					}
			}
			break;

			//-- . -- . -- . -- . -- . -- . --
			case fsm_state_e::RESPONSE:
			{
				tracelog<<"Client: state: RESPONSE"<<endl;
				bool stateRet = false;

				//** state logic **
					TransactionPacket response;
					size_t recvLen=0;

					/*receive RESPONSE data in bytes
					 * Until non-zero bytes of RESPONSE message type are received
					 * or oneshotTimer expires.
					 */
					tracelog<<"Client: RESPONSE: waiting for Response: ACKTimeout="<<(int)clientConfig.getAckTimeout() <<endl;
					do{
						tracelog<<"Client: RESPONSE: trying to receive"<<endl;
						recvLen= this->receive(response,msgType_e::RESPONSE);
						usleep(10);
					}while((recvLen==0) && (this->stateTransitionTimerHandle->isExpired()==false));


					if(recvLen>0){
						tracelog<<"Client: RESPONSE: Response received"<<endl;
						//Info: Valid RESPONSE message with matching msgtype and crc ok is received

						//copy server configuration
						this->serverConfig.setAckTimeout(response.getHeader().getAckTimeout());
						this->serverConfig.setBlocksPerTransaction(response.getHeader().getBlocksPerTransaction());
						this->serverConfig.setMaxPacketLength(response.getHeader().getPacketLength());
						this->serverConfig.setTransactionId(response.getHeader().getTransactionId());
						stateRet = true;
					}


				//** transistion logic **
				if(stateRet){
					tracelog<<"Client: RESPONSE: server config received"<<endl;
					this->setNextFSMState(fsm_state_e::INITIATE_BLOCK);
					this->startStateTransitionTimer(this->serverConfig.getAckTimeout());
				}else{
					warnlog<<"Client: RESPONSE: failure, recv packet or saving server config"<<endl;
					this->setNextFSMState(fsm_state_e::TERMINATE);
				}
			}
			break;

			//-- . -- . -- . -- . -- . -- . --
			case fsm_state_e::INITIATE_BLOCK:
			{
				tracelog<<"Client: RESPONSE: INITIATE_BLOCK"<<endl;
				bool stateRet = false;
				TransactionPacket initiate;

				//** state logic **

					//Validate Server config, If not agreeable then terminate.
					if(this->serverConfig.getTransactionId()!=0){
						stateRet = true;
					}

					/* Create INITIATE BLOCK TRANSFER PACKET, Following are the configurations
					 * which are agreed by client and is same as configuration sent by server
					 * in RESPONSE Packet
					 *
					 * 1. Transaction ID
					 * 2. Block Per Transaction
					 * 3. Max Packet Length
					 * 4. ACK Timeout
					 */
					if(stateRet){
						initiate.getHeader().setMsgType(msgType_e::INITIATE_BLOCK_TRANSFER);
						initiate.getHeader().setAckTimeout(this->serverConfig.getAckTimeout());
						initiate.getHeader().setMaxPacketLength(this->serverConfig.getPacketLength());
						initiate.getHeader().setBlocksPerTransaction(this->serverConfig.getBlocksPerTransaction());
						initiate.getHeader().setTransactionId(this->serverConfig.getTransactionId());
						initiate.generateCRC();
					}
					//send Initiate block transfer msg
					if(stateRet){
						do{
							stateRet = this->send(initiate,initiate.getPacketSize());
							usleep(10);
						}while((stateRet==false) && (this->stateTransitionTimerHandle->isExpired()==false));
					}


				//** transistion logic **
					if(stateRet){
						tracelog<<"Client: INITIATE_BLOCK: Initiate block sent"<<endl;
						this->setNextFSMState(fsm_state_e::BLOCK);
						this->startStateTransitionTimer(this->serverConfig.getAckTimeout());
					}else{
						warnlog<<"Client: INITIATE_BLOCK: Failed to send Initiate block"<<endl;
						this->setNextFSMState(fsm_state_e::TERMINATE);
					}
			}
			break;

			//-- . -- . -- . -- . -- . -- . --
			case fsm_state_e::BLOCK:
			{
				tracelog<<"Client: state: BLOCK"<<endl;
				bool stateRet = false;


				//** state logic **
					blockTxRxPacket blockDataPacket;
					size_t recvLen=0;

					/*receive until non- zero bytes of BLOCK packet are received or
					 * oneshot timer is expired.
					 */
					do{
						recvLen= this->receive(blockDataPacket,msgType_e::BLOCK_TRANSFER);
						usleep(1);
					}while((recvLen==0) && (this->stateTransitionTimerHandle->isExpired()==false));

					if(recvLen==0 && (this->stateTransitionTimerHandle->isExpired()==true)){
						tracelog<<"Client: BLOCK: Timer Expired, Nothing recved within timeout"<<endl;
					}else if(recvLen==0 && (this->stateTransitionTimerHandle->isExpired()==false)){
						warnlog<<"Client: BLOCK: should never come here"<<endl;
					}

					//copy this data to location pointed by client storage application
					if(recvLen>0){
						tracelog<<"Client : BLOCK: "<<"Packet Received"<<endl;
						size_t receivedBlockDataSize = blockDataPacket.getHeader().getDataBufferSize();
						uint8_t* receivedBlockBufferData = blockDataPacket.getBuffer();
						if(internalBuffer!=NULL &&
								receivedBlockBufferData!=NULL &&
								internalBufferLen>=receivedBlockDataSize){
							tracelog<<"Client : BLOCK: storing the Packet: "<<endl;
							if(memcpy(internalBuffer,receivedBlockBufferData,receivedBlockDataSize)!=NULL){
								internalBuffer += receivedBlockDataSize;
								internalBufferLen -=receivedBlockDataSize;
								tracelog<<"Client : BLOCK: Packet saved successfully: "<<endl;
								stateRet = true;
							}
						}
					}

				//** transition logic **
					if(stateRet){
						//check if this is last frame,
						if(blockDataPacket.getHeader().getContinueBit()==continueBit_e::TERMINATE_TRANSFER){

							tracelog<<"Client : BLOCK: last frame received: "<<endl;

							//Info: this is last frame
							TransactionPacket ackPacket;
							ackPacket.getHeader().setTransactionId(this->serverConfig.getTransactionId());
							ackPacket.getHeader().setMsgType(msgType_e::BLOCK_ACK);
							ackPacket.generateCRC();

							//start state transition timer
							this->startStateTransitionTimer(this->serverConfig.getAckTimeout());

							do{
								stateRet = this->send(ackPacket,ackPacket.getPacketSize());
								usleep(10);
							}while((stateRet==false) && (this->stateTransitionTimerHandle->isExpired()==false));

							if(stateRet){
								isTransferSuccess = true;
								tracelog<<"Client : BLOCK: ACK sent: "<<endl;
							}else{
								warnlog<<"Client : BLOCK: Failed to send ACK: "<<endl;
							}

							//Once BLOCK_ACK is sent, goto terminate state
							this->setNextFSMState(fsm_state_e::TERMINATE);

						}else{
							tracelog<<"Client : BLOCK: receive more: "<<endl;
							//This is not last frame, the continue in this state
							this->setNextFSMState(this->getCurrentFSMState());
							this->startStateTransitionTimer(this->serverConfig.getAckTimeout());
						}
					}else{
						// something went wrong in block transfer, send BLOCKACKNOT msg to server. and terminate
						warnlog<<"Client : BLOCK: something went wrong: "<<endl;
						tracelog<<"Client : BLOCK: sending NOT_ACK: "<<endl;
						TransactionPacket ackPacket;
						ackPacket.getHeader().setTransactionId(this->serverConfig.getTransactionId());
						ackPacket.getHeader().setMsgType(msgType_e::BLOCK_ACKNOT);
						ackPacket.generateCRC();

						//start state transition timer
						this->startStateTransitionTimer(this->serverConfig.getAckTimeout());

						do{
							stateRet = this->send(ackPacket,ackPacket.getPacketSize());
							usleep(10);
						}while((stateRet==false) && (this->stateTransitionTimerHandle->isExpired()==false));


						if(stateRet){
							tracelog<<"Client : BLOCK: ACK_NOT sent: "<<endl;
						}else{
							warnlog<<"Client : BLOCK: Failed to send ACK_NOT: "<<endl;
						}

						//Once BLOCK_ACKNOT is sent, goto terminate state
						this->setNextFSMState(fsm_state_e::TERMINATE);


					}
			}
			break;

			//-- . -- . -- . -- . -- . -- . --
			case fsm_state_e::TERMINATE:
			{
				tracelog<<"Client: state: TERMINATE"<<endl;

				//** Stop state transition timer **
				this->stopStateTransitionTimer();

				//** Notify application about transfer status **
					if(notifyCB!=NULL){
						tracelog<<"Client: TERMINATE: Notifying application on transfer status"<<endl;
						if(isTransferSuccess){
							notifyCB(true);
						}else{
							notifyCB(false);
						}
					}

				//** cleanup **
					tracelog<<"Client: TERMINATE: Cleanup"<<endl;
					this->isTransferSuccess = false;
					this->serverConfig.setAckTimeout(0);
					this->serverConfig.setBlocksPerTransaction(0);
					this->serverConfig.setMaxPacketLength(0);
					this->serverConfig.setTransactionId(0);
					this->internalBuffer = NULL;
					this->internalBufferLen = 0;

				//** transistion logic **
					this->setNextFSMState(fsm_state_e::IDLE);
			}
			break;

			//-- . -- . -- . -- . -- . -- . --
			default:
			{
				warnlog<<"Client: UNKNOWN STATE"<<endl;
				this->setNextFSMState(fsm_state_e::TERMINATE);
			}
		}
	}
	catch(std::exception& e){
		exceptlog;
		errorlog<<e.what()<<endl;
		return ;
	}
	catch(emu_err_t& e){
		exceptlog;
		errorlog<<endl;
		return ;
	}
}





DDClient::DDClient(string name,timer_loop_t& timerloop, DD_send_cb_t sendcb,DD_receive_cb_t recvcb,
		DD_PACKET::transactionID_t transactionID,
		DD_PACKET::blocksPerTransaction_t blocksPerTransaction,
		DD_PACKET::maxPacketLen_t maxPacketlen,	DD_PACKET::ACKTimeout_t ackTimeout,
		DD_consumer_cb_t storageRead,DD_notify_cb_t appStatusCB):
		DataDump(mode_e::CLIENT,name,timerloop,sendcb,recvcb,transactionID,blocksPerTransaction,
				maxPacketlen,ackTimeout,storageRead,appStatusCB){

	try{
//		if(this->stateTransitionTimerHandle->registerCB(DDClientStateTimerCallback,this)==false){
//			cout<<"Client one shot timer error"<<endl;
//		}
	}
	catch(std::exception& e){
		exceptlog;
		errorlog<<e.what()<<endl;
		return ;
	}
	catch(emu_err_t& e){
		exceptlog;
		errorlog<<endl;
		return ;
	}
}



DDClient::~DDClient() {
}




bool DDClient::startNewTransfer(uint8_t *buffer, size_t size) {

	bool ret = true;
	marklog;

	try{
		if(ret){
			if(buffer==NULL || size<=0){
				ret = false;
			}
		}

		//Check if client is busy
		if(ret){
			if(isModuleBusy()==true){
				ret = false;
			}
		}

		//client is free, so update next state and start fsm update timer.
		if(ret){
			tracelog<< "data dump transaction initiated"<<endl;
			internalBuffer = buffer;
			internalBufferLen = size;
			//set next state as response
			setNextFSMState(fsm_state_e::REQUEST);
		}

		return ret;
	}
	catch(std::exception& e){
		exceptlog;
		errorlog<<e.what()<<endl;
		return false;
	}
	catch(emu_err_t& e){
		exceptlog;
		errorlog<<endl;
		return false;
	}
}





