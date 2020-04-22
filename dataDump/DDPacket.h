/*
 * packet.h
 *
 *  Created on: Mar 29, 2020
 *      Author: karsh
 */

#ifndef COMPONENTS_COMMUNICATION_DATADUMPPROTOCOL_PACKET_H_
#define COMPONENTS_COMMUNICATION_DATADUMPPROTOCOL_PACKET_H_

/*
 * DDprotocol.cpp
 *
 *  Created on: Mar 29, 2020
 *      Author: karsh
 */
#include "stdio.h"
#include "crc16.h"

namespace DD_PACKET{
/* Message type is describes metadata in the message.
 * 	UNASSIGNED: Message that is not valid for transaction
 * 	REQUEST: Message type associated with transaction request messages (Client sends --> server receives)
 * 	RESPONSE: Message type associated with transaction response message (Server sends --> client receives)
 * 	BLOCK_DATA: Message type associated with block data message transcation (server sends ---> client receives)
 * 	BLOCK_ACK: Message type associated with validating reception of block data message (client sends ---> server receives)
 */
typedef enum{
	UNASSIGNED=0,
	REQUEST,
	RESPONSE,
	INITIATE_BLOCK_TRANSFER,
	BLOCK_TRANSFER,
	BLOCK_ACK,
	BLOCK_ACKNOT

}msgType_e;

/* Block data message contains 1 bit field describing
 * 	TERMINATE: indicating termination of transfer
 *  CONTINUE: indicating more block data message to be transferred.
 */
typedef enum{
	TERMINATE_TRANSFER=0,
	CONTINUE_TRANSFER=1
}continueBit_e;


/* Typedef of different fields used in message header.
 * This allows to flexibility to adapt later
 *
 */
typedef uint8_t msgType_t;
typedef uint32_t transactionID_t;
typedef uint8_t blocksPerTransaction_t; //min 1 and max 127
typedef uint8_t maxPacketLen_t; //in bytes
typedef uint8_t ACKTimeout_t; //10 ms per lsb change
typedef uint16_t crc_t;
typedef uint8_t blockMetaData_t;
typedef uint8_t dataBufferSize_t;
typedef uint8_t frameNumber_t;

const blocksPerTransaction_t maxBlockPerTrasactionLimit = 127;
const blocksPerTransaction_t minBlockPerTrasactionLimit = 1;


class TranscationHeader{
	private:
		msgType_e msgType;
		transactionID_t transactionID;
		blocksPerTransaction_t maxBlocksPerTransaction;
		maxPacketLen_t maxPacketlen;
		ACKTimeout_t maxACKTimeout;
		crc_t crc;
		dataBufferSize_t dataBufferSize;
	public:

		//constructors and destructors
		TranscationHeader();
		TranscationHeader(msgType_e msgType,transactionID_t transactionID,crc_t crc,dataBufferSize_t dataBufferSize,
					blocksPerTransaction_t blocksPerTransaction,maxPacketLen_t maxSizeofBlock,
					ACKTimeout_t ACKTimeout);
		TranscationHeader(const TranscationHeader& packet)=delete;
		~TranscationHeader();

		//operator overloading
		TranscationHeader& operator=(const TranscationHeader& packet)=delete;


		//getters
		ACKTimeout_t getAckTimeout() const;
		blocksPerTransaction_t getBlocksPerTransaction() const ;
		maxPacketLen_t getPacketLength() const ;
		msgType_e getMsgType() const ;
		transactionID_t getTransactionId() const ;
		crc_t getCrc() const ;
		dataBufferSize_t getDataBufferSize() const;

		//setters
		void setAckTimeout(ACKTimeout_t ackTimeout) ;
		void setBlocksPerTransaction(blocksPerTransaction_t blocksPerTransaction) ;
		void setCrc(crc_t crc) ;
		void setMaxPacketLength(maxPacketLen_t maxSizeofBlock) ;
		void setMsgType(msgType_e msgType) ;
		void setTransactionId(transactionID_t transactionId) ;
		void setDataBufferSize(dataBufferSize_t dataBufferSize);


		//ByteArray to type conversion
		bool bytes_to_MsgType(const uint8_t* ptr,size_t len);
		bool bytes_to_TransactionId(const uint8_t* ptr,size_t len);
		bool byte_to_BlockPerTransaction(const uint8_t* ptr,size_t len);
		bool byte_to_MaxSizeOfBlock(const uint8_t* ptr,size_t len);
		bool byte_to_ACKTimeout(const uint8_t* ptr,size_t len);
		bool byte_to_CRC(const uint8_t* ptr,size_t len);
		bool byte_to_DataBufferSize(const uint8_t* ptr,size_t len);

		//type to byteArray conversion
		bool msgType_to_Bytes(uint8_t* ptr,size_t len);
		bool transactionID_to_Bytes(uint8_t* ptr,size_t len);
		bool blockPerTransaction_to_Bytes(uint8_t* ptr,size_t len);
		bool maxSizeOfBlock_to_Bytes(uint8_t* ptr,size_t len);
		bool ackTimeout_to_Bytes(uint8_t* ptr,size_t len);
		bool crc_to_Bytes(uint8_t* ptr,size_t len);
		bool dataBufferSize_to_Bytes(uint8_t* ptr,size_t len);


		void printAll();
};




class blockHeader{
private:
	msgType_e msgType;
	blockMetaData_t blockMetadata;
	crc_t crc;
	dataBufferSize_t dataBufferSize;
public:
	blockHeader(const blockHeader& blockHeader)=delete;
	blockHeader& operator=(const blockHeader& packet)=delete;

	blockHeader();
	blockHeader(msgType_e msgType,frameNumber_t frameNumber,continueBit_e continueBit,crc_t crc,dataBufferSize_t dataBufferSize);
	~blockHeader();

	//getters
	continueBit_e getContinueBit() const ;
	blockMetaData_t getBlockMetadata() const;
	frameNumber_t getFrameNumber() const ;
	msgType_e getMsgType() const ;
	crc_t getCrc() const ;
	dataBufferSize_t getDataBufferSize() const;

	//setters
	void setContinueBit(continueBit_e continueBit) ;
	void setBlockMetadata(blockMetaData_t blockMetadata);
	void setFrameNumber(frameNumber_t frameNumber) ;
	void setMsgType(msgType_e msgType) ;
	void setDataBufferSize(dataBufferSize_t dataBufferSize);
	void setCrc(crc_t crc) ;

	//ByteArray to type conversion
	bool bytes_to_MsgType(const uint8_t* ptr,size_t len);
	bool bytes_to_BlockMetadata(const uint8_t* ptr,size_t len);
	bool bytes_to_CRC(const uint8_t* ptr,size_t len);
	bool Bytes_to_DataBufferSize(const uint8_t* ptr,size_t len);

	//type to byteArray conversion
	bool msgType_to_Bytes(uint8_t* ptr,size_t len);
	bool blockMetaData_to_Bytes(uint8_t* ptr,size_t len);
	bool crc_to_Bytes(uint8_t* ptr,size_t len);
	bool DataBufferSize_to_Bytes(uint8_t* ptr,size_t len);

	void printAll();


};




class TransactionPacket{
private:
	TranscationHeader header;
	uint8_t* buffer;
public:
	TransactionPacket(const TransactionPacket& packet)=delete;
	TransactionPacket& operator=(const TransactionPacket& packet)=delete;

	TransactionPacket();
	TransactionPacket(msgType_e msgType,transactionID_t transactionID=0,crc_t crc=0,
					blocksPerTransaction_t blocksPerTransaction=0,maxPacketLen_t maxSizeofBlock=0,
					ACKTimeout_t ACKTimeout=0,uint8_t* buffer=NULL,dataBufferSize_t bufferSize=0);
	~TransactionPacket();

	//getters
	uint8_t* getBuffer();
	TranscationHeader& getHeader();
	size_t getPacketSize();

	//setters
	bool setBuffer(uint8_t *buffer,dataBufferSize_t length);
	void setHeader(msgType_e msgType,transactionID_t transactionID,crc_t crc,dataBufferSize_t dataBufferSize,
					blocksPerTransaction_t blocksPerTransaction,maxPacketLen_t maxSizeofBlock,
					ACKTimeout_t ACKTimeout);

	//Serialization/ deserialization
	bool deserialize(uint8_t* buffer,size_t len);
	size_t serialize(uint8_t* ptr,size_t len);

	//generate crc
	bool generateCRC();

	//print all atributes
	void printAll();
};



class blockTxRxPacket{
private:
	blockHeader header;
	uint8_t* buffer;
public:
	blockTxRxPacket(const blockTxRxPacket& packet) = delete;
	blockTxRxPacket& operator=(const blockTxRxPacket& packet)= delete;

	blockTxRxPacket();
	blockTxRxPacket(msgType_e msgType,frameNumber_t frameNumber,continueBit_e continueBit,crc_t crc,
			uint8_t* buffer,dataBufferSize_t bufferSize);
	~blockTxRxPacket();

	//getters
	uint8_t* getBuffer();
	blockHeader& getHeader();
	size_t getPacketSize();
	static size_t getHeaderSize();

	//setters
	bool setBuffer(uint8_t *buffer,dataBufferSize_t length);
	void setHeader(msgType_e msgType,frameNumber_t frameNumber,continueBit_e continueBit,crc_t crc,
			dataBufferSize_t bufferSize);

	//Serialization/ deserialization
	bool deserialize(uint8_t* buffer,size_t len);
	size_t serialize(uint8_t* ptr,size_t len);

	//generate crc
	bool generateCRC();

	//print all atributes
	void printAll();
};

}


#endif /* COMPONENTS_COMMUNICATION_DATADUMPPROTOCOL_PACKET_H_ */
