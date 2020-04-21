/*
 * logger.h
 *
 *  Created on: Apr 18, 2020
 *      Author: karsh
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <iostream>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include "boost/log/utility/setup.hpp"

namespace logging = boost::log;
namespace keywords = boost::log::keywords;

#define tracelog BOOST_LOG_TRIVIAL(trace) << "func:"<<__func__<<" [MSG] "
#define debuglog BOOST_LOG_TRIVIAL(debug) << "func:"<< __func__<<" [MSG] "
#define infolog BOOST_LOG_TRIVIAL(info) << "func:"<<__func__<<" [MSG] "
#define warnlog BOOST_LOG_TRIVIAL(warning) << "func:"<< __func__<<" [MSG] "
#define errorlog BOOST_LOG_TRIVIAL(error) << "func:"<< __func__<<" [MSG] "

void init_logging();


#endif /* LOGGER_H_ */
