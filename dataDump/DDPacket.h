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
		blocksPerTransaction_t blocksPerTransaction;
		maxPacketLen_t maxPacketlen;
		ACKTimeout_t ackTimeout;
		crc_t crc;
		dataBufferSize_t dataBufferSize;
	public:

		//constructors and destructors
		TranscationHeader();
		TranscationHeader(msgType_e msgType,transactionID_t transactionID,crc_t crc,dataBufferSize_t dataBufferSize,
					blocksPerTransaction_t blocksPerTransaction,maxPacketLen_t maxSizeofBlock,
					ACKTimeout_t ACKTimeout);
		TranscationHeader(const TranscationHeader& packet);
		~TranscationHeader();

		//operator overloading
		TranscationHeader& operator=(const TranscationHeader& packet);


		//getters
		ACKTimeout_t getAckTimeout() const;
		blocksPerTransaction_t getBlocksPerTransaction() const ;
		maxPacketLen_t getMaxPacketLength() const ;
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
		bool bytes2MsgType(const uint8_t* ptr,size_t len);
		bool bytes2TransactionId(const uint8_t* ptr,size_t len);
		bool byte2BlockPerTransaction(const uint8_t* ptr,size_t len);
		bool byte2MaxSizeOfBlock(const uint8_t* ptr,size_t len);
		bool byte2ACKTimeout(const uint8_t* ptr,size_t len);
		bool byte2CRC(const uint8_t* ptr,size_t len);
		bool byte2DataBufferSize(const uint8_t* ptr,size_t len);

		//type to byteArray conversion
		bool msgType2Bytes(uint8_t* ptr,size_t len);
		bool TransactionId2Bytes(uint8_t* ptr,size_t len);
		bool BlockPerTransaction2Bytes(uint8_t* ptr,size_t len);
		bool MaxSizeOfBlock2Bytes(uint8_t* ptr,size_t len);
		bool ACKTimeout2Bytes(uint8_t* ptr,size_t len);
		bool CRC2Bytes(uint8_t* ptr,size_t len);
		bool DataBufferSize2Bytes(uint8_t* ptr,size_t len);


		void printAll();
};




class blockHeader{
private:
	msgType_e msgType;
	blockMetaData_t blockMetadata;
	crc_t crc;
	dataBufferSize_t dataBufferSize;
public:
	//constructors and destructors
	blockHeader();
	blockHeader(msgType_e msgType,frameNumber_t frameNumber,continueBit_e continueBit,crc_t crc,dataBufferSize_t dataBufferSize);
	blockHeader(const blockHeader& blockHeader);
	~blockHeader();

	//operator overloading
	blockHeader& operator=(const blockHeader& packet);

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
	bool bytes2MsgType(const uint8_t* ptr,size_t len);
	bool bytes2BlockMetadata(const uint8_t* ptr,size_t len);
	bool bytes2CRC(const uint8_t* ptr,size_t len);
	bool Bytes2DataBufferSize(const uint8_t* ptr,size_t len);

	//type to byteArray conversion
	bool msgType2Bytes(uint8_t* ptr,size_t len);
	bool blockMetaData2Bytes(uint8_t* ptr,size_t len);
	bool CRC2Bytes(uint8_t* ptr,size_t len);
	bool DataBufferSize2Bytes(uint8_t* ptr,size_t len);

	void printAll();


};




class TransactionPacket{
private:
	TranscationHeader header;
	uint8_t* buffer;
public:

	//constructors and destructors
	TransactionPacket();
	TransactionPacket(msgType_e msgType,transactionID_t transactionID=0,crc_t crc=0,
					blocksPerTransaction_t blocksPerTransaction=0,maxPacketLen_t maxSizeofBlock=0,
					ACKTimeout_t ACKTimeout=0,uint8_t* buffer=NULL,dataBufferSize_t bufferSize=0);
	~TransactionPacket();
	TransactionPacket(const TransactionPacket& packet)=delete;

	//operator overloading
	TransactionPacket& operator=(const TransactionPacket& packet)=delete;

	//getters
	uint8_t* getBuffer();
	TranscationHeader& getHeader();

	//setters
	bool setBuffer(uint8_t *buffer,dataBufferSize_t length);
	void setHeader(const TranscationHeader &header);
	void setHeader(msgType_e msgType,transactionID_t transactionID,crc_t crc,dataBufferSize_t dataBufferSize,
					blocksPerTransaction_t blocksPerTransaction,maxPacketLen_t maxSizeofBlock,
					ACKTimeout_t ACKTimeout);

	//bytes to packet
	bool deserialize(uint8_t* buffer,size_t len);

	//packets to bytes
	size_t serialize(uint8_t* ptr,size_t len);

	//generate crc
	bool generateCRC();

	//size of packet
	size_t size();

	//print all atributes
	void printAll();
};



class blockTxRxPacket{
private:
	blockHeader header;
	uint8_t* buffer;
public:
	//constructors and destructors
	blockTxRxPacket();
	blockTxRxPacket(msgType_e msgType,frameNumber_t frameNumber,continueBit_e continueBit,crc_t crc,
			uint8_t* buffer,dataBufferSize_t bufferSize);
	~blockTxRxPacket();
	blockTxRxPacket(const blockTxRxPacket& packet) = delete;

	//operator overloading
	blockTxRxPacket& operator=(const blockTxRxPacket& packet)= delete;

	//getters
	uint8_t* getBuffer();
	blockHeader& getHeader();

	//setters
	bool setBuffer(uint8_t *buffer,dataBufferSize_t length);
	void printBuffer();
	void setHeader(const blockHeader &header);
	void setHeader(msgType_e msgType,frameNumber_t frameNumber,continueBit_e continueBit,crc_t crc,dataBufferSize_t bufferSize);

	//bytes to packet
	bool deserialize(uint8_t* buffer,size_t len);

	//packets to bytes
	size_t serialize(uint8_t* ptr,size_t len);

	//generate crc
	bool generateCRC();

	//size of packet
	size_t size();

	//print all atributes
	void printAll();

	static size_t headerSize();
};

}


#endif /* COMPONENTS_COMMUNICATION_DATADUMPPROTOCOL_PACKET_H_ */
