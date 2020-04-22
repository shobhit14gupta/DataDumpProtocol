/*
 * baseDD.h
 *
 *  Created on: Apr 13, 2020
 *      Author: karsh
 */

#ifndef DDBASE_H_
#define DDBASE_H_

//#include "../ClockAndTimer/header/msTimer.h"
#include "../bTimer/bTimer.h"
#include "functional"
#include "string"
#include "string.h"
#include "../ErrorHandler/errorHandler.h"
#include "DDPacket.h"
#include <mutex>
#include <condition_variable>
#include "boost/asio.hpp"
#include <thread>


using namespace std;

class DataDump{

public:
	using DD_timer_cb_t = std::function<void(void* arg)>;
	using DD_send_cb_t = std::function<bool(uint8_t* arg,size_t length)>;
	using DD_receive_cb_t = std::function<size_t (uint8_t* arg,size_t maxLength)>;
	using DD_consumer_cb_t = std::function<size_t(uint8_t* bufptr,size_t maxSize,bool eraseDataRead)>;
	using DD_notify_cb_t = std::function<void(bool transferStatus)>;
	typedef boost::asio::io_service timer_loop_t;

	typedef enum{
		SERVER=0,
		CLIENT
	}mode_e;


	typedef enum {
		IDLE=0,
		REQUEST,
		RESPONSE,
		INITIATE_BLOCK,
		BLOCK,
		TERMINATE
	}fsm_state_e;


protected:
	/* module info*/
	mode_e mode;
	string name;

	/*timeout timer handle*/
	btimer* stateTransitionTimerHandle;

	/* transport layer callback */
	size_t maxPacketSize;
	DD_send_cb_t sendCB;
	DD_receive_cb_t recvCB;

	/* transaction variables */
	uint32_t internal_counter;

	DD_PACKET::TranscationHeader serverConfig;
	DD_PACKET::TranscationHeader clientConfig;

	uint8_t* internalBuffer;
	size_t internalBufferLen;

	DD_consumer_cb_t consumerCB;
	DD_notify_cb_t notifyCB;

	bool isTransferSuccess;

private:

	bool isBusy;

	fsm_state_e currentState;
	fsm_state_e nextState;

	timed_mutex cur_mutex;
	timed_mutex nxt_mutex;

public:
	DataDump(mode_e type,string name,timer_loop_t& timer_loop,
			DD_send_cb_t sendcb,DD_receive_cb_t recvcb,DD_PACKET::transactionID_t transactionID,
			DD_PACKET::blocksPerTransaction_t blocksPerTransaction,
			DD_PACKET::maxPacketLen_t maxPacketlen,
			DD_PACKET::ACKTimeout_t ackTimeout,
			DD_consumer_cb_t storageRead,
			DD_notify_cb_t appStatusCB);

	virtual ~DataDump();

	/* copies the data to sendBuffer.
	 * The data from sendBuffer is delievered to transmitting protocol or layer.
	 * Lock need to implemented to make use of transmitting layer safely
	 */
	bool send(DD_PACKET::TransactionPacket& packet ,uint8_t length);
	bool send(DD_PACKET::blockTxRxPacket& packet,uint8_t length);

	/* copies the data to recvBuffer.
	 * The data from recvBuffer is delievered to fsm.
	 * Lock need to implemented to make use of recv function safely
	 */
	size_t receive(DD_PACKET::TransactionPacket& packet,DD_PACKET::msgType_e msgType);
	size_t receive(DD_PACKET::blockTxRxPacket& packet,DD_PACKET::msgType_e msgType);

	/* starts timer with a timer rate.
	 * @param timer tick rate in Hertz
	 * @return true on success and false on failure
	 */
	bool startStateTransitionTimer(DD_PACKET::ACKTimeout_t timeout);

	/* stops timer
	 * @return true on success and false on failure
	 */
	bool stopStateTransitionTimer();


	/* starts timer with a timer rate.
	 * @param timer tick rate in Hertz
	 * @return true on success and false on failure
	 */
	bool startFSMTimer();

	/* stops timer
	 * @return true on success and false on failure
	 */
	bool stopFSMTimer();

	void setCurrentFSMState(fsm_state_e state) noexcept;
	void setNextFSMState(fsm_state_e state) noexcept;

	fsm_state_e getCurrentFSMState() noexcept;
	fsm_state_e getNextFSMState() noexcept;

	bool isModuleBusy() const;
	void setModuleStatus(bool Status);

	virtual void FSMLoop()=0;
};



#endif /* DDBASE_H_ */
