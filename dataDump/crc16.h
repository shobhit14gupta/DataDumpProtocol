/*
 * crc16.h
 *
 *  Created on: Mar 29, 2020
 *      Author: karsh
 */

#ifndef COMPONENTS_COMMUNICATION_DATADUMPPROTOCOL_CRC16_H_
#define COMPONENTS_COMMUNICATION_DATADUMPPROTOCOL_CRC16_H_

#include "stdio.h"
#include "stdint.h"

class CRC16{
public:
	static uint16_t generate(uint8_t& buffer,size_t length);
};



#endif /* COMPONENTS_COMMUNICATION_DATADUMPPROTOCOL_CRC16_H_ */
