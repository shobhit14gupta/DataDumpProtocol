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
#include <boost/log/utility/setup.hpp>


namespace logging = boost::log;
namespace keywords = boost::log::keywords;


#define marklog BOOST_LOG_TRIVIAL(trace) << "{FIL} "<< __FILE__<<" {FUN} "<<__func__<<" {LIN} "<<__LINE__<<endl;
#define tracelog BOOST_LOG_TRIVIAL(trace) << "{FUN} "<<__func__<<" {MSG} "
#define debuglog BOOST_LOG_TRIVIAL(debug) << "{FUN} "<< __func__<<" {MSG} "
#define infolog BOOST_LOG_TRIVIAL(info) << "{FUN} "<<__func__<<" {MSG} "
#define warnlog BOOST_LOG_TRIVIAL(warning) << "{FUN} "<< __func__<<" {MSG} "
#define errorlog BOOST_LOG_TRIVIAL(error) << "{FUN} "<< __func__<<" {MSG} "
#define exceptlog BOOST_LOG_TRIVIAL(error) << "{FIL} "<< __FILE__<<" {FUN} "<<__func__<<" {LIN} "<<__LINE__<<endl;


void init_logging();


#endif /* LOGGER_H_ */
