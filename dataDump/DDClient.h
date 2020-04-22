/*
 * DDClient.h
 *
 *  Created on: Apr 13, 2020
 *      Author: karsh
 */

#ifndef DDCLIENT_H_
#define DDCLIENT_H_
#include "DDBase.h"

class DDClient:public DataDump{
private:


public:
	DDClient(string name, timer_loop_t& timerloop,DD_send_cb_t sendcb,DD_receive_cb_t recvcb,
			DD_PACKET::transactionID_t transactionID,
			DD_PACKET::blocksPerTransaction_t blocksPerTransaction,
			DD_PACKET::maxPacketLen_t maxPacketlen,	DD_PACKET::ACKTimeout_t ackTimeout,
			DD_consumer_cb_t storageRead,DD_notify_cb_t appStatusCB);
	~DDClient();
	void FSMLoop();
	bool startNewTransfer(uint8_t* buffer,size_t size);

	//friend void DDClientStateTimerCallback(void* arg);
	//friend void DDClientUpdateTimerCallback(void* arg);

};



#endif /* DDCLIENT_H_ */
