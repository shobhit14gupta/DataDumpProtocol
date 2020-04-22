/*
 * packet.cpp
 *
 *  Created on: Mar 29, 2020
 *      Author: karsh
 */

#include "DDPacket.h"
#include "string.h"
#include <iostream>
#include "../ErrorHandler/errorHandler.h"
#include "../Logger/logger.h"

using namespace DD_PACKET;

/*
 *
 */
TranscationHeader::TranscationHeader()
:msgType(msgType_e::UNASSIGNED),transactionID(0),maxBlocksPerTransaction(0),maxPacketlen(0),
maxACKTimeout(0),crc(0),dataBufferSize(0)
{

}


/*
 *
 */
TranscationHeader::TranscationHeader(msgType_e msgType, transactionID_t transactionID,
		crc_t crc, dataBufferSize_t dataBufferSize,blocksPerTransaction_t blocksPerTransaction,
		maxPacketLen_t maxSizeofBlock, ACKTimeout_t ACKTimeout)
:msgType(msgType),transactionID(transactionID),maxPacketlen(maxSizeofBlock),
 maxACKTimeout(ACKTimeout),crc(crc),dataBufferSize(dataBufferSize)
{
	try{
		if (blocksPerTransaction >= minBlockPerTrasactionLimit
				&& blocksPerTransaction <= maxBlockPerTrasactionLimit) {
			this->maxBlocksPerTransaction = blocksPerTransaction;
		}
		else {
			this->maxBlocksPerTransaction = 0; //zero block per transaction means invalid header
		}
	}
	catch(std::exception& e){
			errorlog<<e.what()<<endl;
	}
	catch(emu_err_t& e){
		exceptlog;
		errorlog<<endl;
		return ;
	}
}






/*
 *
 */
TranscationHeader::~TranscationHeader() {
}


/*
 *
 */
ACKTimeout_t TranscationHeader::getAckTimeout() const {
	return maxACKTimeout;
}


/*
 *
 */
blocksPerTransaction_t TranscationHeader::getBlocksPerTransaction() const {
	return maxBlocksPerTransaction;
}


/*
 *
 */
maxPacketLen_t TranscationHeader::getPacketLength() const {
	return maxPacketlen;
}


/*
 *
 */
msgType_e TranscationHeader::getMsgType() const {
	return msgType;
}


/*
 *
 */
transactionID_t TranscationHeader::getTransactionId() const {
	return transactionID;
}

/*
 *
 */
crc_t TranscationHeader::getCrc() const {
	return crc;
}

/*
 *
 */
void TranscationHeader::setAckTimeout(ACKTimeout_t ackTimeout) {
	this->maxACKTimeout = ackTimeout;
}


/*
 *
 */
void TranscationHeader::setBlocksPerTransaction(
		blocksPerTransaction_t blocksPerTransaction) {
	this->maxBlocksPerTransaction = blocksPerTransaction;
}


/*
 *
 */
void TranscationHeader::setCrc(crc_t crc) {
	this->crc = crc;
}


/*
 *
 */
void TranscationHeader::setMaxPacketLength(maxPacketLen_t maxSizeofBlock) {
	this->maxPacketlen = maxSizeofBlock;
}


/*
 *
 */
void TranscationHeader::setMsgType(msgType_e msgType) {
	this->msgType = msgType;
}

/*
 *
 */
dataBufferSize_t TranscationHeader::getDataBufferSize() const {
	return dataBufferSize;
}


/*
 *
 */
void TranscationHeader::setDataBufferSize(dataBufferSize_t dataBufferSize) {
	this->dataBufferSize = dataBufferSize;
}

/*
 *
 */
void TranscationHeader::setTransactionId(transactionID_t transactionId) {
	transactionID = transactionId;
}



//ByteArray to type conversion
bool TranscationHeader::bytes_to_MsgType(const uint8_t* ptr,size_t len) {
	if(len>=sizeof(msgType_t) && ptr!=NULL){
		switch(ptr[0]){
		case REQUEST:
			this->msgType = msgType_e::REQUEST;
			break;
		case RESPONSE:
			this->msgType = msgType_e::RESPONSE;
			break;
		case INITIATE_BLOCK_TRANSFER:
			this->msgType = msgType_e::INITIATE_BLOCK_TRANSFER;
			break;
		case BLOCK_TRANSFER:
			this->msgType = msgType_e::BLOCK_TRANSFER;
			break;
		case BLOCK_ACK:
			this->msgType = msgType_e::BLOCK_ACK;
			break;
		case BLOCK_ACKNOT:
			this->msgType = msgType_e::BLOCK_ACKNOT;
			break;
		default:
			this->msgType = msgType_e::UNASSIGNED;
			break;
		}

		return true;
	}
	return false;
}


bool TranscationHeader::bytes_to_TransactionId(const uint8_t* ptr,size_t len) {


	transactionID_t ret=0;
	if(len>=sizeof(transactionID_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(transactionID_t);i++){
			transactionID_t byteCopier=ptr[i];
			ret |= byteCopier<<(i*8);
		}

		this->transactionID = ret;
		return true;
	}
	return false;
}

bool TranscationHeader::byte_to_BlockPerTransaction(const uint8_t* ptr,size_t len) {

	blocksPerTransaction_t ret=0;
	if(len>=sizeof(blocksPerTransaction_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(blocksPerTransaction_t);i++){
			blocksPerTransaction_t byteCopier=ptr[i];
			ret |= byteCopier<<(i*8);
		}
		this->maxBlocksPerTransaction = ret;
		return true;
	}
	return false;
}

bool TranscationHeader::byte_to_MaxSizeOfBlock(const uint8_t* ptr,size_t len) {

	maxPacketLen_t ret=0;
	if(len>=sizeof(maxPacketLen_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(maxPacketLen_t);i++){
			maxPacketLen_t byteCopier=ptr[i];
			ret |= byteCopier<<(i*8);
		}
		this->maxPacketlen = ret;
		return true;
	}
	return false;
}

bool TranscationHeader::byte_to_ACKTimeout(const uint8_t* ptr,size_t len) {

	ACKTimeout_t ret=0;
	if(len>=sizeof(ACKTimeout_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(ACKTimeout_t);i++){
			ACKTimeout_t byteCopier=ptr[i];
			ret |= byteCopier<<(i*8);
		}
		this->maxACKTimeout = ret;
		return true;
	}
	return false;
}

bool TranscationHeader::byte_to_CRC(const uint8_t* ptr,size_t len) {

	crc_t ret=0;
	if(len>=sizeof(crc_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(crc_t);i++){
			crc_t byteCopier=ptr[i];
			ret |= byteCopier<<(i*8);
		}

		this->crc = ret;
		return true;
	}
	return false;
}

bool TranscationHeader::byte_to_DataBufferSize(const uint8_t* ptr,size_t len) {

	dataBufferSize_t ret=0;
	if(len>=sizeof(dataBufferSize_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(dataBufferSize_t);i++){
			dataBufferSize_t byteCopier=ptr[i];
			ret |= byteCopier<<(i*8);
		}
		this->dataBufferSize = ret;
		return true;
	}
	return false;
}


//type to byteArray conversion
bool TranscationHeader::msgType_to_Bytes(uint8_t* ptr,size_t len){
	uint8_t* buf = ptr;
	if(len>=sizeof(msgType_t) && ptr!=NULL){
		switch(this->msgType){
		case REQUEST:
			buf[0]=(uint8_t)(msgType_e::REQUEST);
			break;
		case RESPONSE:
			buf[0]=(uint8_t)(msgType_e::RESPONSE);
			break;
		case INITIATE_BLOCK_TRANSFER:
			buf[0]=(uint8_t)(msgType_e::INITIATE_BLOCK_TRANSFER);
			break;
		case BLOCK_TRANSFER:
			buf[0]=(uint8_t)(msgType_e::BLOCK_TRANSFER);
			break;
		case BLOCK_ACK:
			buf[0]=(uint8_t)(msgType_e::BLOCK_ACK);
			break;
		case BLOCK_ACKNOT:
			this->msgType = msgType_e::BLOCK_ACKNOT;
			break;
		default:
			buf[0]=(uint8_t)(msgType_e::UNASSIGNED);
			break;
		}
		return true;
	}
	return false;
}

bool TranscationHeader::transactionID_to_Bytes(uint8_t* ptr,size_t len){
	uint8_t* buf = ptr;
	if(len>=sizeof(transactionID_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(transactionID_t);i++){
			transactionID_t byteCopier=255;
			byteCopier = byteCopier<<(i*8);
			uint8_t val= (this->transactionID & byteCopier)>>(i*8);
			buf[i] = val;

		}
		return true;
	}
	return false;
}


bool TranscationHeader::blockPerTransaction_to_Bytes(uint8_t* ptr,size_t len){
	uint8_t* buf = ptr;
	if(len>=sizeof(blocksPerTransaction_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(blocksPerTransaction_t);i++){
			blocksPerTransaction_t byteCopier=255;
			byteCopier = byteCopier<<(i*8);
			buf[i] = (this->maxBlocksPerTransaction & byteCopier)>>(i*8);
		}
		return true;
	}
	return false;
}


bool TranscationHeader::maxSizeOfBlock_to_Bytes(uint8_t* ptr,size_t len){
	uint8_t* buf = ptr;
	if(len>=sizeof(maxPacketLen_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(maxPacketLen_t);i++){
			maxPacketLen_t byteCopier=255;
			byteCopier = byteCopier<<(i*8);
			buf[i] = (this->maxPacketlen & byteCopier)>>(i*8);
		}
		return true;
	}
	return false;
}


bool TranscationHeader::ackTimeout_to_Bytes(uint8_t* ptr,size_t len){
	uint8_t* buf = ptr;
	if(len>=sizeof(ACKTimeout_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(ACKTimeout_t);i++){
			ACKTimeout_t byteCopier=255;
			byteCopier = byteCopier<<(i*8);
			buf[i] = (this->maxACKTimeout & byteCopier)>>(i*8);
		}
		return true;
	}
	return false;
}


bool TranscationHeader::crc_to_Bytes(uint8_t* ptr,size_t len){

	uint8_t* buf = ptr;
	if(len>=sizeof(crc_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(crc_t);i++){
			crc_t byteCopier=255;
			byteCopier = byteCopier<<(i*8);
			buf[i] = (this->crc & byteCopier)>>(i*8);
		}
		return true;
	}
	return false;
}


bool TranscationHeader::dataBufferSize_to_Bytes(uint8_t* ptr,size_t len){
	uint8_t* buf = ptr;
	if(len>=sizeof(dataBufferSize_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(dataBufferSize_t);i++){
			dataBufferSize_t byteCopier=255;
			byteCopier = byteCopier<<(i*8);
			buf[i] = (this->dataBufferSize & byteCopier)>>(i*8);
		}
		return true;
	}
	return false;
}


void TranscationHeader::printAll() {
	std::cout<< "{msgType} "<<this->getMsgType()<<std::endl;
	std::cout<< "{transactionID} "<<this->getTransactionId()<<std::endl;
	std::cout<< "{blocksPerTransaction} "<<this->getBlocksPerTransaction()<<std::endl;
	std::cout<< "{maxSizeofBlock} "<<this->getPacketLength()<<std::endl;
	std::cout<< "{ACKTimeout} "<<this->getAckTimeout()<<std::endl;
	std::cout<< "{crc} "<<this->getCrc()<<std::endl;
	std::cout<< "{bufferSize} "<<this->getDataBufferSize()<<std::endl;
}

//====================================================================
blockHeader::blockHeader():msgType(msgType_e::UNASSIGNED),blockMetadata(0),crc(0),dataBufferSize(0){
}

blockHeader::blockHeader(msgType_e msgType,frameNumber_t frameNumber,continueBit_e continueBit,crc_t crc,
		dataBufferSize_t dataBufferSize)
:msgType(msgType),crc(crc),dataBufferSize(dataBufferSize){
	uint8_t msbPos= (sizeof(blockMetaData_t)*8)-1;
	this->blockMetadata = (continueBit<<msbPos)| frameNumber;
}

//blockHeader::blockHeader(const blockHeader& blockHeader){
//	this->msgType = blockHeader.msgType;
//	this->blockMetadata = blockHeader.blockMetadata;
//	this->crc = blockHeader.crc;
//	this->dataBufferSize=blockHeader.dataBufferSize;
//}

blockHeader::~blockHeader(){}



//getters and setters

continueBit_e blockHeader::getContinueBit() const {
	uint8_t msbPos= (sizeof(blockMetaData_t)*8)-1;
	blockMetaData_t temp = this->blockMetadata;
	temp = temp >> msbPos; //extract msbPos value
	if(temp){
		return CONTINUE_TRANSFER;
	}
	return TERMINATE_TRANSFER;
}

void blockHeader::setContinueBit(continueBit_e continueBit) {
	uint8_t msbPos= (sizeof(blockMetaData_t)*8)-1;
	blockMetaData_t temp = this->blockMetadata;
	temp &= ~(1<<msbPos); 			//clear msbPos bit
	temp |= (continueBit<<msbPos); //set msbPos bit with value in continue bit
	this->blockMetadata =temp;
}

frameNumber_t blockHeader::getFrameNumber() const {
	uint8_t msbPos= (sizeof(blockMetaData_t)*8)-1;
	blockMetaData_t temp =this->blockMetadata;
	temp &= ~(1<<msbPos); //clear msbPos bit, value remains is frame count only
	return temp;
}

void blockHeader::setFrameNumber(frameNumber_t frameNumber) {

	uint8_t msbPos= (sizeof(blockMetaData_t)*8)-1;
	frameNumber &= ~(1<<msbPos); // make sure msbPos bit is cleared
	this->blockMetadata |= frameNumber; //set frame number
}

msgType_e blockHeader::getMsgType() const {
	return msgType;
}

void blockHeader::setMsgType(msgType_e msgType) {
	this->msgType = msgType;
}

crc_t blockHeader::getCrc() const {
	return crc;
}

dataBufferSize_t blockHeader::getDataBufferSize() const {
	return dataBufferSize;
}

//blockHeader& blockHeader::operator =(const blockHeader &blockHeader) {
//	if(this!=&blockHeader){
//		this->msgType = blockHeader.msgType;
//		this->blockMetadata = blockHeader.blockMetadata;
//		this->crc = blockHeader.crc;
//		this->dataBufferSize=blockHeader.dataBufferSize;
//	}
//	return *this;
//}

void blockHeader::setDataBufferSize(dataBufferSize_t dataBufferSize) {
	this->dataBufferSize = dataBufferSize;
}

void blockHeader::setCrc(crc_t crc) {
	this->crc = crc;
}

blockMetaData_t DD_PACKET::blockHeader::getBlockMetadata() const {
	return blockMetadata;
}

void DD_PACKET::blockHeader::setBlockMetadata(blockMetaData_t blockMetadata) {
	this->blockMetadata = blockMetadata;
}


//ByteArray to type conversion
bool blockHeader::bytes_to_MsgType(const uint8_t* ptr,size_t len) {
	if(len>=sizeof(msgType_t) && ptr!=NULL){
		switch(ptr[0]){
		case REQUEST:
			this->msgType = msgType_e::REQUEST;
			break;
		case RESPONSE:
			this->msgType = msgType_e::RESPONSE;
			break;
		case INITIATE_BLOCK_TRANSFER:
			this->msgType = msgType_e::INITIATE_BLOCK_TRANSFER;
			break;
		case BLOCK_TRANSFER:
			this->msgType = msgType_e::BLOCK_TRANSFER;
			break;
		case BLOCK_ACK:
			this->msgType = msgType_e::BLOCK_ACK;
			break;
		case BLOCK_ACKNOT:
			this->msgType = msgType_e::BLOCK_ACKNOT;
			break;
		default:
			this->msgType = msgType_e::UNASSIGNED;
			break;
		}
		return true;
	}
	return false;
}


bool blockHeader::bytes_to_BlockMetadata(const uint8_t* ptr,size_t len) {

	blockMetaData_t ret=0;
	if(len>=sizeof(blockMetaData_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(blockMetaData_t);i++){
			blockMetaData_t byteCopier=ptr[i];
			ret |= byteCopier<<(i*8);
		}
		this->blockMetadata = ret;
		return true;
	}
	return false;
}


bool blockHeader::bytes_to_CRC(const uint8_t* ptr,size_t len) {

	crc_t ret=0;
	if(len>=sizeof(crc_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(crc_t);i++){
			crc_t byteCopier=ptr[i];
			ret |= byteCopier<<(i*8);
		}
		this->crc = ret;
		return true;
	}
	return false;
}

bool blockHeader::Bytes_to_DataBufferSize(const uint8_t* ptr,size_t len) {

	dataBufferSize_t ret=0;
	if(len>=sizeof(dataBufferSize_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(dataBufferSize_t);i++){
			dataBufferSize_t byteCopier=ptr[i];
			ret |= byteCopier<<(i*8);
		}
		this->dataBufferSize = ret;
		return true;
	}
	return false;
}




//type to byteArray conversion
bool blockHeader::msgType_to_Bytes(uint8_t* ptr,size_t len){
	uint8_t* buf = ptr;
	if(len>=sizeof(msgType_t) && ptr!=NULL){
		switch(this->msgType){
		case REQUEST:
			buf[0]=(uint8_t)(msgType_e::REQUEST);
			break;
		case RESPONSE:
			buf[0]=(uint8_t)(msgType_e::RESPONSE);
			break;
		case INITIATE_BLOCK_TRANSFER:
			buf[0]=(uint8_t)(msgType_e::INITIATE_BLOCK_TRANSFER);
			break;
		case BLOCK_TRANSFER:
			buf[0]=(uint8_t)(msgType_e::BLOCK_TRANSFER);
			break;
		case BLOCK_ACK:
			buf[0]=(uint8_t)(msgType_e::BLOCK_ACK);
			break;
		case BLOCK_ACKNOT:
			this->msgType = msgType_e::BLOCK_ACKNOT;
			break;
		default:
			buf[0]=(uint8_t)(msgType_e::UNASSIGNED);
			break;
		}
		return true;
	}
	return false;
}

bool blockHeader::blockMetaData_to_Bytes(uint8_t* ptr,size_t len){

	uint8_t* buf = ptr;
	if(len>=sizeof(blockMetaData_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(blockMetaData_t);i++){
			blockMetaData_t byteCopier=255;
			byteCopier = byteCopier<<(i*8);
			buf[i] = (this->blockMetadata & byteCopier)>>(i*8);
		}
		return true;
	}
	return false;
}

bool blockHeader::crc_to_Bytes(uint8_t* ptr,size_t len){

	uint8_t* buf = ptr;
	if(len>=sizeof(crc_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(crc_t);i++){
			crc_t byteCopier=255;
			byteCopier = byteCopier<<(i*8);
			buf[i] = (this->crc & byteCopier)>>(i*8);
		}
		return true;
	}
	return false;
}


bool blockHeader::DataBufferSize_to_Bytes(uint8_t* ptr,size_t len){

	uint8_t* buf = ptr;
	if(len>=sizeof(dataBufferSize_t) && ptr!=NULL){
		for(uint8_t i=0;i<sizeof(dataBufferSize_t);i++){
			dataBufferSize_t byteCopier=255;
			byteCopier = byteCopier<<(i*8);
			buf[i] = (this->dataBufferSize & byteCopier)>>(i*8);
		}
		return true;
	}
	return false;
}



void blockHeader::printAll() {
	std::cout<<"{msgType} "<<this->msgType<<std::endl;
	std::cout<<"{FrameNumber} "<<this->getFrameNumber()<<std::endl;
	std::cout<<"{ContinueBit} "<<this->getContinueBit()<<std::endl;
	std::cout<<"{crc} "<<this->getCrc()<<std::endl;
	std::cout<<"{dataBufferSize} "<<this->getDataBufferSize()<<std::endl;
}
//===========================================================
TransactionPacket::TransactionPacket()
:header(),buffer(NULL){

}

TransactionPacket::TransactionPacket(msgType_e msgType,transactionID_t transactionID, crc_t crc,
		blocksPerTransaction_t blocksPerTransaction,maxPacketLen_t maxSizeofBlock, ACKTimeout_t ACKTimeout,
		uint8_t* buffer, dataBufferSize_t bufferSize)
:header(msgType,transactionID,crc,bufferSize,blocksPerTransaction,maxSizeofBlock,ACKTimeout),buffer(NULL){

	if(buffer==NULL && bufferSize>0){
		this->buffer = new uint8_t[bufferSize];
		if(this->buffer!=NULL){
			for(uint8_t iter=0;iter<bufferSize;iter++){
				this->buffer[iter]= buffer[iter];
			}
		}
	}
}

TransactionPacket::~TransactionPacket() {
	if(buffer!=NULL){
		delete [] buffer;
	}

}

//TransactionPacket::TransactionPacket(
//		const TransactionPacket &packet) {
//
//	this->header = packet.header;
//	dataBufferSize_t packetDataBufferSize = packet.header.getDataBufferSize();
//
//	if(buffer!=NULL && packetDataBufferSize>0){
//		this->buffer = new uint8_t[packetDataBufferSize];
//		if(this->buffer!=NULL){
//			for(uint8_t iter=0;iter<packetDataBufferSize;iter++){
//				this->buffer[iter]= packet.buffer[iter];
//			}
//		}
//	}
//
//}


bool TransactionPacket::setBuffer(uint8_t *in_buffer,dataBufferSize_t length) {

	if(in_buffer!=NULL && length>0){

		//Check if old buffer exists, if exists delete it
		if(this->buffer!=NULL){
			delete [] this->buffer;
		}

		//create new buffer
		this->buffer = new uint8_t [length];
		if(this->buffer!=NULL){
			memcpy(this->buffer,in_buffer,length);
			this->header.setDataBufferSize(length);
			return true;
		}
	}
	return false;
}


uint8_t* TransactionPacket::getBuffer() {
	return this->buffer;
}

TranscationHeader& TransactionPacket::getHeader() {
	return header;
}

void TransactionPacket::setHeader(msgType_e msgType,transactionID_t transactionID,crc_t crc,dataBufferSize_t dataBufferSize,
		blocksPerTransaction_t blocksPerTransaction,maxPacketLen_t maxSizeofBlock,ACKTimeout_t ACKTimeout) {
	this->header.setMsgType(msgType);
	this->header.setAckTimeout(ACKTimeout);
	this->header.setBlocksPerTransaction(blocksPerTransaction);
	this->header.setCrc(crc);
	this->header.setDataBufferSize(dataBufferSize);
	this->header.setMaxPacketLength(maxSizeofBlock);
	this->header.setTransactionId(transactionID);
}

//TransactionPacket& TransactionPacket::operator =(
//		const TransactionPacket &packet) {
//	if(this!=&packet){
//		this->header = packet.header;
//		dataBufferSize_t packetDataBufferSize = packet.header.getDataBufferSize();
//
//		if(buffer!=NULL && packetDataBufferSize>0){
//			this->buffer = new uint8_t[packetDataBufferSize];
//			if(this->buffer!=NULL){
//				for(uint8_t iter=0;iter<packetDataBufferSize;iter++){
//					this->buffer[iter]= packet.buffer[iter];
//				}
//			}
//		}
//	}
//	return *this;
//}

bool DD_PACKET::TransactionPacket::deserialize(
		uint8_t *ptr, size_t len) {

	if(ptr!=NULL){
		uint8_t* buffer = ptr;

		this->header.bytes_to_MsgType(buffer,sizeof(msgType_t));
		buffer+=sizeof(msgType_t);

		this->header.bytes_to_TransactionId(buffer,sizeof(transactionID_t));
		buffer+=sizeof(transactionID_t);

		this->header.byte_to_BlockPerTransaction(buffer,sizeof(blocksPerTransaction_t));
		buffer+=sizeof(blocksPerTransaction_t);

		this->header.byte_to_MaxSizeOfBlock(buffer,sizeof(maxPacketLen_t));
		buffer+=sizeof(maxPacketLen_t);

		this->header.byte_to_ACKTimeout(buffer,sizeof(ACKTimeout_t));
		buffer+=sizeof(ACKTimeout_t);

		this->header.byte_to_CRC(buffer,sizeof(crc_t));
		buffer+=sizeof(crc_t);

		this->header.byte_to_DataBufferSize(buffer,sizeof(dataBufferSize_t));
		buffer+=sizeof(dataBufferSize_t);

		size_t bufferSize = (size_t)this->header.getDataBufferSize();
		return this->setBuffer(buffer,bufferSize);
	}

	return false;
}

size_t DD_PACKET::TransactionPacket::serialize(uint8_t *ptr, size_t len) {

	if(ptr!=NULL){
		uint8_t* buffer = ptr;


		size_t payloadSize = this->getPacketSize();
		if(len<payloadSize ){
			return 0;
		}

		if(this->header.msgType_to_Bytes(buffer,sizeof(msgType_t))==false)
			return 0;
		buffer+=sizeof(msgType_t);

		if(this->header.transactionID_to_Bytes(buffer,sizeof(transactionID_t))==false)
			return 0;
		buffer+=sizeof(transactionID_t);

		if(this->header.blockPerTransaction_to_Bytes(buffer,sizeof(blocksPerTransaction_t))==false)
			return 0;
		buffer+=sizeof(blocksPerTransaction_t);

		if(this->header.maxSizeOfBlock_to_Bytes(buffer,sizeof(maxPacketLen_t))==false)
			return 0;
		buffer+=sizeof(maxPacketLen_t);

		if(this->header.ackTimeout_to_Bytes(buffer,sizeof(ACKTimeout_t))==false)
			return 0;
		buffer+=sizeof(ACKTimeout_t);

		if(this->header.crc_to_Bytes(buffer,sizeof(crc_t))==false)
			return 0;
		buffer+=sizeof(crc_t);

		if(this->header.dataBufferSize_to_Bytes(buffer,sizeof(dataBufferSize_t))==false)
			return 0;
		buffer+=sizeof(dataBufferSize_t);

		size_t bufferSize = (size_t)this->header.getDataBufferSize();
		if(bufferSize>0){
			if(this->setBuffer(buffer,bufferSize)==true){
				return payloadSize;
			}
		}
	}
 return 0;
}


bool TransactionPacket::generateCRC(){
//	size_t payloadSize = this->size();
//	uint8_t byteBuf[payloadSize]={0};
//
//	if(this->serialize(byteBuf,payloadSize)==false){
//		return false;
//	}
//
//	this->getHeader().setCrc((crc_t)CRC16::generate(*byteBuf,payloadSize));
	return true;
}


size_t TransactionPacket::getPacketSize(){
	size_t bufferSize = (size_t)this->header.getDataBufferSize();
	size_t payloadSize = sizeof(msgType_t) + sizeof(transactionID_t) +
					+ sizeof(blocksPerTransaction_t) + sizeof(maxPacketLen_t)
					+ sizeof(ACKTimeout_t) + sizeof(crc_t)
					+ sizeof(dataBufferSize_t) + bufferSize;
	return payloadSize;
}

void TransactionPacket::printAll() {
	this->header.printAll();
	uint8_t* temp=this->getBuffer();
	if(temp!=NULL)
	{
		for(uint8_t i=0;i<this->header.getDataBufferSize();i++){
			std::cout<<"["<<i<<"] "<<temp[i]<<std::endl;
		}
	}
}

//===========================================================
blockTxRxPacket::blockTxRxPacket()
:header(),buffer(NULL) {
}

blockTxRxPacket::blockTxRxPacket(msgType_e msgType, frameNumber_t frameNumber,
		continueBit_e continueBit, crc_t crc, uint8_t *in_buffer,dataBufferSize_t bufferSize)
:header(msgType,frameNumber,continueBit,crc,bufferSize),buffer(NULL) {
	if(in_buffer!=NULL && bufferSize>0){
		this->buffer = new uint8_t[bufferSize];
		if(this->buffer!=NULL){
			for(uint8_t iter=0;iter<bufferSize;iter++){
				this->buffer[iter]= in_buffer[iter];
			}
		}
	}

}

blockTxRxPacket::~blockTxRxPacket() {
	if(this->buffer!=NULL){
		delete [] this->buffer;
	}
}


uint8_t* blockTxRxPacket::getBuffer() {
	return (uint8_t*)this->buffer;
}


bool blockTxRxPacket::setBuffer(uint8_t *in_buffer,dataBufferSize_t length) {
	if(in_buffer!=NULL && length>0){

		if(this->buffer!=NULL){
			delete [] this->buffer;
		}


		this->buffer = new uint8_t[length];
		if(this->buffer!=NULL){
			memcpy(this->buffer,in_buffer,length);
			this->header.setDataBufferSize(length);
			return true;
		}
	}
	return false;
}



blockHeader& blockTxRxPacket::getHeader(){
	return header;
}


void blockTxRxPacket::setHeader(msgType_e msgType,frameNumber_t frameNumber,
		continueBit_e continueBit,crc_t crc,dataBufferSize_t bufferSize) {
	this->header.setMsgType(msgType);
	this->header.setFrameNumber(frameNumber);
	this->header.setContinueBit(continueBit);
	this->header.setCrc(crc);
	this->header.setDataBufferSize(bufferSize);
}


bool DD_PACKET::blockTxRxPacket::deserialize(uint8_t *ptr, size_t len) {

	if(ptr!=NULL){
		uint8_t* buffer = ptr;

		this->header.bytes_to_MsgType(buffer,sizeof(msgType_t));
		buffer+=sizeof(msgType_t);

		this->header.bytes_to_BlockMetadata(buffer,sizeof(blockMetaData_t));
		buffer+=sizeof(blockMetaData_t);

		this->header.bytes_to_CRC(buffer,sizeof(crc_t));
		buffer+=sizeof(crc_t);

		this->header.Bytes_to_DataBufferSize(buffer,sizeof(dataBufferSize_t));
		buffer+=sizeof(dataBufferSize_t);

		size_t bufferSize = (size_t)this->header.getDataBufferSize();
		return this->setBuffer(buffer,bufferSize);
	}

	return false;
}

size_t DD_PACKET::blockTxRxPacket::serialize(uint8_t *ptr, size_t len) {
	if(ptr!=NULL){
		uint8_t* buffer = ptr;

		size_t payloadSize = this->getPacketSize();
		if(len<payloadSize ){
			return 0;
		}

		if(this->header.msgType_to_Bytes(buffer,sizeof(msgType_t))==false)
			return 0;
		buffer+=sizeof(msgType_t);

		if(this->header.blockMetaData_to_Bytes(buffer,sizeof(blockMetaData_t))==false)
			return 0;
		buffer+=sizeof(blockMetaData_t);

		if(this->header.crc_to_Bytes(buffer,sizeof(crc_t))==false)
			return 0;
		buffer+=sizeof(crc_t);

		if(this->header.DataBufferSize_to_Bytes(buffer,sizeof(dataBufferSize_t))==false)
			return 0;
		buffer+=sizeof(dataBufferSize_t);

		size_t bufferSize = (size_t)this->header.getDataBufferSize();
		if(bufferSize>0){
			//copy 'packet buffer' data to uint8_t buffer
			memcpy(buffer,this->buffer,bufferSize);
			return payloadSize;
		}

	}
 return 0;

}

bool blockTxRxPacket::generateCRC(){

//	size_t payloadSize = this->size();
//	uint8_t byteBuf[payloadSize]={0};
//
//	if(this->serialize(byteBuf,payloadSize)==false){
//		return false;
//	}
//
//	this->getHeader().setCrc((crc_t)CRC16::generate(*byteBuf,payloadSize));
	return true;
}


size_t blockTxRxPacket::getPacketSize(){
	size_t bufferSize = (size_t)this->header.getDataBufferSize();
	size_t payloadSize = sizeof(msgType_t) + sizeof(blockMetaData_t)
					+ sizeof(crc_t)+ sizeof(dataBufferSize_t) + bufferSize;
	return payloadSize;
}

size_t blockTxRxPacket::getHeaderSize(){
	return ( sizeof(msgType_t) + sizeof(blockMetaData_t)
			+ sizeof(crc_t)+ sizeof(dataBufferSize_t));
}

void blockTxRxPacket::printAll() {
	this->header.printAll();
	uint8_t* temp=this->getBuffer();
	if(temp!=NULL){
		for(uint8_t i=0;i<this->header.getDataBufferSize();i++){
			std::cout<<"["<<i<<"]"<<" "<<temp[i]<<std::endl;
		}
	}
}
//===========================================================





