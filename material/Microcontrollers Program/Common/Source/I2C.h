/*
 * I2C.h
 *
 *  Created on: Feb 11, 2015
 *      Author: Mathieu
 */

#ifndef I2C_H_
#define I2C_H_

#include <jendefs.h>
#include <AppHardwareApi_JN5168.h>
#include "Config.h"
#include <string.h>
#include "Uart.h"
#include <string.h>

PUBLIC void vInitI2C();
PUBLIC bool bStartWriting(uint8 slaveAddress);
PUBLIC bool bWriteByte(uint8 data);
PUBLIC bool bWriteLastByte(uint8 data);
PUBLIC bool bStartReading(uint8 slaveAddress);
PUBLIC uint8 u8ReadByte();
PUBLIC uint8 u8ReadLastByte();
PUBLIC bool bStartWriting(uint8 slaveAddress);
PUBLIC bool bWriteByte(uint8 data);
PUBLIC bool bWriteLastByte(uint8 data);
PUBLIC bool bStartReading(uint8 slaveAddress);
PUBLIC uint8 u8ReadByte();
PUBLIC uint8 u8ReadLastByte();

PUBLIC bool writeWords(uint8 devAddr, uint8 regAddr, uint8 length, uint16* data);
PUBLIC bool writeWord(uint8 devAddr, uint8 regAddr, uint16 data);
PUBLIC bool writeBit(uint8 devAddr, uint8 regAddr, uint8 bitNum, uint8 data);
PUBLIC bool writeBits(uint8 devAddr, uint8 regAddr, uint8 bitStart, uint8 length, uint8 data);
PUBLIC bool writeByte(uint8 devAddr, uint8 regAddr, uint8 data);
PUBLIC bool writeBytes(uint8 devAddr, uint8 regAddr, uint8 length, uint8* data);
PUBLIC int8 readBit(uint8 devAddr, uint8 regAddr, uint8 bitNum, uint8 *data);
PUBLIC int8 readBits(uint8 devAddr, uint8 regAddr, uint8 bitStart, uint8 length, uint8 *data);
PUBLIC bool readByte(uint8 devAddr, uint8 regAddr, uint8 *data);
PUBLIC bool readBytes(uint8 devAddr, uint8 regAddr, uint8 length, uint8 *data);
#endif /* I2C_H_ */
