/*
 * DDClient.cpp
 *
 *  Created on: Apr 13, 2020
 *      Author: karsh
 */
#include "DDClient.h"

using namespace DD_PACKET;

/*
 * Client: state timer callback
 */
void DDClientStateTimerCallback(void* arg)
{

}


/*
 * Client: update timer callback
 */
void DDClientUpdateTimerCallback(void* arg)
{
	//cout<<"Client: periodic Timer expired"<<endl;
	if(arg!=NULL){
		DDClient* ddInstance = (DDClient*)arg;
		ddInstance->FSMLoop();
	}
}


//============ CLIENT ==================
void DDClient::FSMLoop() {

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
			//cout<<"Client: state: IDLE"<<endl;

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
			cout<<"Client: state: REQUEST"<<endl;
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
					request.getHeader().setMaxPacketLength(clientConfig.getMaxPacketLength());
					request.getHeader().setTransactionId(clientConfig.getTransactionId());
					request.generateCRC();

					do{
						stateRet = this->send(request,request.size());
						usleep(100);
					}while((stateRet==false) && (this->stateTransitionTimerHandle->isExpired()==false));
				}

			//** transition logic **
				if(stateRet){
					this->setNextFSMState(fsm_state_e::RESPONSE);
					this->startStateTransitionTimer(clientConfig.getAckTimeout());
				}else{
					this->setNextFSMState(fsm_state_e::TERMINATE);
				}
		}
		break;

		//-- . -- . -- . -- . -- . -- . --
		case fsm_state_e::RESPONSE:
		{
			cout<<"Client: state: RESPONSE"<<endl;
			bool stateRet = false;

			//** state logic **
				TransactionPacket response;
				size_t recvLen=0;

				/*receive RESPONSE data in bytes
				 * Until non-zero bytes of RESPONSE message type are received
				 * or oneshotTimer expires.
				 */
				cout<<"waiting for Response "<<endl;
				do{
					recvLen= this->receive(response,msgType_e::RESPONSE);
				}while((recvLen==0) && (this->stateTransitionTimerHandle->isExpired()==false));

				cout<<"Response received"<<endl;
				if(recvLen!=0){
					//Info: Valid RESPONSE message with matching msgtype and crc ok is received

					//copy server configuration
					this->serverConfig.setAckTimeout(response.getHeader().getAckTimeout());
					this->serverConfig.setBlocksPerTransaction(response.getHeader().getBlocksPerTransaction());
					this->serverConfig.setMaxPacketLength(response.getHeader().getMaxPacketLength());
					this->serverConfig.setTransactionId(response.getHeader().getTransactionId());
					stateRet = true;
				}


			//** transistion logic **
			if(stateRet){
				this->setNextFSMState(fsm_state_e::INITIATE_BLOCK);
				this->startStateTransitionTimer(this->serverConfig.getAckTimeout());
			}else{
				this->setNextFSMState(fsm_state_e::TERMINATE);
			}
		}
		break;

		//-- . -- . -- . -- . -- . -- . --
		case fsm_state_e::INITIATE_BLOCK:
		{
			cout<<"Client: state: INITIATE_BLOCK"<<endl;
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
					initiate.getHeader().setMaxPacketLength(this->serverConfig.getMaxPacketLength());
					initiate.getHeader().setBlocksPerTransaction(this->serverConfig.getBlocksPerTransaction());
					initiate.getHeader().setTransactionId(this->serverConfig.getTransactionId());
					initiate.generateCRC();
				}
				cout<<"client Initiate block prepared"<<endl;
				//send Initiate block transfer msg
				if(stateRet){
					do{
						stateRet = this->send(initiate,initiate.size());
						usleep(100);
					}while((stateRet==false) && (this->stateTransitionTimerHandle->isExpired()==false));
				}

				cout<<"client Initiate block sent"<<endl;
			//** transistion logic **
				if(stateRet){
					this->setNextFSMState(fsm_state_e::BLOCK);
					this->startStateTransitionTimer(this->serverConfig.getAckTimeout());
				}else{
					this->setNextFSMState(fsm_state_e::TERMINATE);
				}
		}
		break;

		//-- . -- . -- . -- . -- . -- . --
		case fsm_state_e::BLOCK:
		{
			cout<<"Client: state: BLOCK"<<endl;
			bool stateRet = false;


			//** state logic **
				blockTxRxPacket blockDataPacket;
				size_t recvLen=0;

				/*receive until non- zero bytes of BLOCK packet are received or
				 * oneshot timer is expired.
				 */
				do{
					recvLen= this->receive(blockDataPacket,msgType_e::BLOCK_TRANSFER);
				}while((recvLen==0) && (this->stateTransitionTimerHandle->isExpired()==false));


				//copy this data to location pointed by client storage application
				if(recvLen!=0){
					cout<<"Client : Block Packet: "<<"Received"<<endl;
					size_t receivedBlockDataSize = blockDataPacket.getHeader().getDataBufferSize();
					uint8_t* receivedBlockBufferData = blockDataPacket.getBuffer();
					if(internalBuffer!=NULL &&
							receivedBlockBufferData!=NULL &&
							internalBufferLen>=receivedBlockDataSize){
						cout<<"Client : saved the Packet: "<<endl;
						if(memcpy(internalBuffer,receivedBlockBufferData,receivedBlockDataSize)!=NULL){
							internalBuffer += receivedBlockDataSize;
							internalBufferLen -=receivedBlockDataSize;
							stateRet = true;
						}
					}
				}

			//** transition logic **
				if(stateRet){
					//check if this is last frame,
					if(blockDataPacket.getHeader().getContinueBit()==continueBit_e::TERMINATE_TRANSFER){

						cout<<"Client : last frame received: "<<endl;

						//Info: this is last frame
						TransactionPacket ackPacket;
						ackPacket.getHeader().setTransactionId(this->serverConfig.getTransactionId());
						ackPacket.getHeader().setMsgType(msgType_e::BLOCK_ACK);
						ackPacket.generateCRC();

						//start state transition timer
						this->startStateTransitionTimer(this->serverConfig.getAckTimeout());

						do{
							stateRet = this->send(ackPacket,ackPacket.size());
							usleep(100);
						}while((stateRet==false) && (this->stateTransitionTimerHandle->isExpired()==false));

						if(stateRet){
							isTransferSuccess = true;
						}
						//Once BLOCK_ACK is sent, goto terminate state
						this->setNextFSMState(fsm_state_e::TERMINATE);

					}else{

						cout<<"Client : receive more: "<<endl;

						//This is not last frame, the continue in this state
						this->setNextFSMState(this->getCurrentFSMState());
						this->startStateTransitionTimer(this->serverConfig.getAckTimeout());
					}
				}else{
					// something went wrong in block transfer, send BLOCKACKNOT msg to server. and terminate
					TransactionPacket ackPacket;
					ackPacket.getHeader().setTransactionId(this->serverConfig.getTransactionId());
					ackPacket.getHeader().setMsgType(msgType_e::BLOCK_ACKNOT);
					ackPacket.generateCRC();

					//start state transition timer
					this->startStateTransitionTimer(this->serverConfig.getAckTimeout());

					do{
						stateRet = this->send(ackPacket,ackPacket.size());
						usleep(100);
					}while((stateRet==false) && (this->stateTransitionTimerHandle->isExpired()==false));

					//Once BLOCK_ACKNOT is sent, goto terminate state
					this->setNextFSMState(fsm_state_e::TERMINATE);

					cout<<"Client : something went wrong: "<<endl;
				}
		}
		break;

		//-- . -- . -- . -- . -- . -- . --
		case fsm_state_e::TERMINATE:
		{
			cout<<"Client: state: TERMINATE"<<endl;
			//** Stop state transition timer **
			this->stopStateTransitionTimer();

			//** Notify application about transfer status **
				if(notifyCB!=NULL){
					if(isTransferSuccess){
						notifyCB(true);
					}else{
						notifyCB(false);
					}
				}

			//** cleanup **
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
			this->setNextFSMState(fsm_state_e::TERMINATE);
		}


	}
}





DDClient::DDClient(string name, DD_send_cb_t sendcb,DD_receive_cb_t recvcb,
		DD_PACKET::transactionID_t transactionID,
		DD_PACKET::blocksPerTransaction_t blocksPerTransaction,
		DD_PACKET::maxPacketLen_t maxPacketlen,	DD_PACKET::ACKTimeout_t ackTimeout,
		DD_consumer_cb_t storageRead,DD_notify_cb_t appStatusCB):
		DataDump(mode_e::CLIENT,name,sendcb,recvcb,transactionID,blocksPerTransaction,
				maxPacketlen,ackTimeout,storageRead,appStatusCB){

	if(this->stateTransitionTimerHandle->registerCB(DDClientStateTimerCallback,this)==false){
		cout<<"Client one shot timer error"<<endl;
	}
}



DDClient::~DDClient() {
}




bool DDClient::startNewTransfer(uint8_t *buffer, size_t size) {

	bool ret = true;

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
		internalBuffer = buffer;
		internalBufferLen = size;
		//set next state as response
		setNextFSMState(fsm_state_e::REQUEST);
	}

	return ret;
}





