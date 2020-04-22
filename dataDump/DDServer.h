/*
 * DDprotocol.h
 *
 *  Created on: Mar 29, 2020
 *      Author: karsh
 */

#ifndef COMPONENTS_COMMUNICATION_DATADUMPPROTOCOL_DDPROTOCOL_H_
#define COMPONENTS_COMMUNICATION_DATADUMPPROTOCOL_DDPROTOCOL_H_

#include "DDBase.h"

class DDServer:public DataDump{
public:

private:


public:
	DDServer(string name,timer_loop_t& timerloop,DD_send_cb_t sendcb,DD_receive_cb_t recvcb,
			DD_PACKET::transactionID_t transactionID,
			DD_PACKET::blocksPerTransaction_t blocksPerTransaction,
			DD_PACKET::maxPacketLen_t maxPacketlen,
			DD_PACKET::ACKTimeout_t ackTimeout,
			DD_consumer_cb_t storageRead,
			DD_notify_cb_t appStatusCB);
	~DDServer();
	void FSMLoop();
	bool startServer();
	bool stopServer();

	//friend void DDServerStateTimerCallback(void* arg);
	//friend void DDServerUpdateTimerCallback(void* arg);

};


#endif /* COMPONENTS_COMMUNICATION_DATADUMPPROTOCOL_DDPROTOCOL_H_ */
