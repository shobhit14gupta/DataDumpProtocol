/*
 * logger.cpp
 *
 *  Created on: Apr 18, 2020
 *      Author: karsh
 */
#include "logger.h"




void init_logging()
{
    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

    // Output message to console
    logging::add_console_log(
        std::cout,
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%",
        keywords::auto_flush = true
    );

    // Output message to file
    logging::add_file_log
    (
        keywords::file_name = "debugLog.txt",
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%"
    );

    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::trace
    );

    logging::add_common_attributes();
}


