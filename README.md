# DataDumpProtocol
#Data Dump Protocol (DDP)

###Project Status: under development
##Description
DDP is a high-level (physical layer independent) data transfer protocol. 
DataDump protocol can be used for large file tranfer such as music, pictures etc.
DDP involves 2 players
1. server (containing {sensor} data)
2. client (requestion {sensor} data)

Features included
1. Frame format
2. Message segmentation
3. Server-client transaction flow

Refer Power point file to get more insight in the protocol


##Dependencies

  Includes
  1. Timerlibrary (a wrapper around boost.asio timers)
  2. DataDump protocol
    2.1 Packet (message frame format serialization/deserialization mplementation)
    2.2 baseDataDump class (Includes packets)
    2.3 server derieved from baseDataDump
    2.4 client derieved from baseDataDump

  compiled libraries:
    -lpthread
    -lboost_system
    -lboost_thread
    -lboost_log
    -lboost_log_setup

  Predefined Symbol
    -DBOOST_LOG_DYN_LINK=1
  
 

