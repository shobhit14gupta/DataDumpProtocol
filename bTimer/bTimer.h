/*
 * bTimer.h
 *
 *  Created on: Apr 17, 2020
 *      Author: karsh
 */

#ifndef BTIMER_H_
#define BTIMER_H_

#include <string>
#include <string.h>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <mutex>
#include<thread>
#include <chrono>

class btimer{
public:
	typedef std::function<void(void* arg)> cb_t;
	typedef enum{
		ONE_SHOT=0,
		PERIODIC=1
	}mode_e;

private:
	std::string name;
	mode_e mode;
	boost::asio::deadline_timer* handle;
	boost::asio::io_service service;
	uint64_t duration;//changing
	cb_t notifyCB;
	void* notifyCBarg;
	pthread_t threadID;
	std::timed_mutex resProtect;
	bool timerExpiry;

public:
	btimer(const btimer& ref) = delete;
	btimer& operator=(const btimer& ref) = delete;

	btimer(std::string name,boost::asio::io_service& service,mode_e mode,cb_t callback=NULL,void* callback_arg=NULL);
	~btimer();
	bool start(uint64_t msInterval) noexcept;
	bool stop() noexcept;
	bool isExpired() noexcept;
	bool registerCB(cb_t callback,void* callback_arg) noexcept;
	const std::string& getName() const;

private:
	bool restart() noexcept;
	friend void* serviceTaskLoop(void* arg);
	friend void timeoutCB(btimer* arg) noexcept;

};



#endif /* BTIMER_H_ */
