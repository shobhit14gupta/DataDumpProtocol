/*
 * baseDD.cpp
 *
 *  Created on: Apr 13, 2020
 *      Author: karsh
 */
#include "DDBase.h"
#include "../Logger/logger.h"


using namespace DD_PACKET;






/*
 *
 *
 */
DataDump::DataDump(mode_e type,string name,
		DD_send_cb_t sendcb,DD_receive_cb_t recvcb,DD_PACKET::transactionID_t transactionID,
		DD_PACKET::blocksPerTransaction_t blocksPerTransaction,
		DD_PACKET::maxPacketLen_t maxPacketlen,
		DD_PACKET::ACKTimeout_t ackTimeout,
		DD_consumer_cb_t storageRead,
		DD_notify_cb_t appStatusCB):
		mode(type),name(name),
		stateTransitionTimerHandle(NULL),
		maxPacketSize(maxPacketlen),
		sendCB(sendcb),recvCB(recvcb),
		internal_counter(1),
		internalBuffer(NULL),
		internalBufferLen(0),
		notifyCB(appStatusCB),
		consumerCB(storageRead),
		isTransferSuccess(false),
		isBusy(false),
		currentState(fsm_state_e::IDLE),
		nextState(fsm_state_e::IDLE)
{
		if(mode==DataDump::mode_e::CLIENT){
			clientConfig.setTransactionId(transactionID);
			clientConfig.setBlocksPerTransaction(blocksPerTransaction);
			clientConfig.setMaxPacketLength(maxPacketlen);
			clientConfig.setAckTimeout(ackTimeout);

		}else{
			serverConfig.setTransactionId(transactionID);
			serverConfig.setBlocksPerTransaction(blocksPerTransaction);
			serverConfig.setMaxPacketLength(maxPacketlen);
			serverConfig.setAckTimeout(ackTimeout);
		}
		//Create oneshot timer.
		stateTransitionTimerHandle = new btimer(string(name+"ST"),btimer::ONE_SHOT);
		if(stateTransitionTimerHandle==NULL){
			throw EMU_ERR_OUT_OF_MEMORY;
		}
}

DataDump::~DataDump() {

	//Stop if timer is still running
	stateTransitionTimerHandle->stop();

}




bool DataDump::send(TransactionPacket& packet, uint8_t length) {
	bool ret = false;
		if(length >maxPacketSize)
			return false;

		//serialize the packet
		uint8_t buffer[maxPacketSize]={0};
		packet.serialize(buffer,length);

		//copy to send buffer
		if(sendCB!=NULL && length<=maxPacketSize){
			ret = sendCB(buffer,length);
		}

		return ret;

}



bool DataDump::send(blockTxRxPacket& packet, uint8_t length) {
	bool ret = false;

		if(length >maxPacketSize)
			return false;

		//serialize the packet
		uint8_t buffer[maxPacketSize]={0};

		if(packet.getBuffer()!=NULL)
		{
			uint8_t* tempBuf =packet.getBuffer();
			size_t tempSize = packet.getHeader().getDataBufferSize();
		}


		uint8_t len =packet.serialize(buffer,maxPacketSize);

		//copy to send buffer
		if(sendCB!=NULL && len<=maxPacketSize){
			ret = sendCB(buffer,len);
		}
		return ret;

}




size_t DataDump::receive(TransactionPacket& packet,msgType_e msgType) {
	size_t len =0;
	uint8_t recvByteBuffer[maxPacketSize]={0};


		if(this->recvCB!=NULL){
			//receive data using recvCB. RecvCB is expected to be non-blocking call.
			 len=recvCB(recvByteBuffer,maxPacketSize);
		}


		if(len>0){
			//check msg type of received message to be matching in arg
			if(recvByteBuffer[0]!=(uint8_t)msgType){
				tracelog<<"DDBase:"<<this->name<<"Wrong msgType"<<endl;
				return 0;
			}

			//serialize the packet and verify the CRC.
			packet.deserialize(recvByteBuffer,len);
			if(packet.getHeader().getCrc()!=CRC16::generate(recvByteBuffer[0],len)){
				tracelog<<"DDBase:"<<this->name<<"Wrong crc"<<endl;
				return 0;
			}
		}

		return len;

}


size_t DataDump::receive(blockTxRxPacket& packet,msgType_e msgType) {
	size_t len =0;
	uint8_t recvByteBuffer[maxPacketSize]={0};

		if(recvCB!=NULL){
			//receive data using recvCB. RecvCB is expected to be non-blocking call.
			 len=recvCB(recvByteBuffer,maxPacketSize);
		}

		if(len>0){
			//check msg type of received message to be matching in arg
			if(recvByteBuffer[0]!=(uint8_t)msgType){
				return 0;
			}

			//serialize the packet and verify the CRC.
			packet.deserialize(recvByteBuffer,len);
			if(packet.getHeader().getCrc()!=CRC16::generate(recvByteBuffer[0],len)){
				return 0;
			}
		}

		return len;

}



bool DataDump::startStateTransitionTimer(ACKTimeout_t timeout) {

		if(this->stateTransitionTimerHandle->start(timeout*100)==false){
			cout<<"fail to start timer"<<endl;
			return false;
		}
		return true;

}



bool DataDump::stopStateTransitionTimer() {

		if(this->stateTransitionTimerHandle->stop()==false){
				return false;
		}
		return true;

}


void DataDump::setCurrentFSMState(fsm_state_e state){
	//===== Critical section <
	std::lock_guard<std::mutex> lck (cur_mutex);
	currentState = state;
	//===== Critical section >
}


void DataDump::setNextFSMState(fsm_state_e state){
	//===== Critical section <
	{
		std::lock_guard<std::mutex> lck (nxt_mutex);
		nextState = state;
	}
	//===== Critical section >
}


DataDump::fsm_state_e DataDump::getCurrentFSMState(){
	fsm_state_e temp;
	//===== Critical section <
	{
		std::lock_guard<std::mutex> lck (cur_mutex);
		temp = currentState;
	}
	//===== Critical section >
	return temp;
}


DataDump::fsm_state_e DataDump::getNextFSMState(){
	fsm_state_e temp;
	//===== Critical section <
	{
		std::lock_guard<std::mutex> lck (nxt_mutex);
		temp = nextState;
	}
	//===== Critical section >
	return temp;
}

bool DataDump::isModuleBusy() const {
	return isBusy;
}

void DataDump::setModuleStatus(bool serverStatus) {
	this->isBusy = serverStatus;
}





