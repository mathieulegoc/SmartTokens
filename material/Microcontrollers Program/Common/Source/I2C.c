/*
 * I2C.c
 *
 *  Created on: Feb 11, 2015
 *      Author: Mathieu
 */

#include "I2C.h"
#include <jendefs.h>
#include <AppHardwareApi_JN5168.h>
#include "Config.h"
#include <string.h>
#include "Uart.h"
#include <string.h>
#include <dbg.h>
#include <dbg_uart.h>

PUBLIC void vInitI2C() {
	DBG_vPrintf(DEBUG_DEVICE_CONFIG,"vInitI2C\n");
	vAHI_SiMasterConfigure(FALSE, FALSE, 4);
}

/** Read multiple bits from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-7)
 * @param length Number of bits to read (not more than 8)
 * @param data Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Status of read operation (true = success)
 */
PUBLIC int8 readBits(uint8 devAddr, uint8 regAddr, uint8 bitStart, uint8 length,
		uint8 *data) {
	// 01101001 read byte
	// 76543210 bit numbers
	//    xxx   args: bitStart=4, length=3
	//    010   masked
	//   -> 010 shifted
	uint8 count, b;
	if ((count = readByte(devAddr, regAddr, &b)) != 0) {
		uint8 mask = ((1 << length) - 1) << (bitStart - length + 1);
		b &= mask;
		b >>= (bitStart - length + 1);
		*data = b;
	}
	return count;
}

/** Read a single bit from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitNum Bit position to read (0-7)
 * @param data Container for single bit value
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Status of read operation (true = success)
 */
PUBLIC int8 readBit(uint8 devAddr, uint8 regAddr, uint8 bitNum, uint8 *data) {
	uint8 b;
	uint8 count = readByte(devAddr, regAddr, &b);
	*data = b & (1 << bitNum);
	return count;
}

/** Read multiple bytes from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register regAddr to read from
 * @param length Number of bytes to read
 * @param data Buffer to store read data in
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Number of bytes read (-1 indicates failure)
 */
PUBLIC bool readBytes(uint8 devAddr, uint8 regAddr, uint8 length, uint8 *data) {
	uint8 i = 0;
	if (bStartWriting(devAddr)) {
		if (bWriteByte(regAddr)) {
			if (bStartReading(devAddr)) {
				for (i = 0; i < length - 1; i++)
					data[i] = u8ReadByte();
				data[length - 1] = u8ReadLastByte();
				return TRUE;
			}
		}
	}
	return FALSE;
}

/** Read single byte from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param data Container for byte value read from device
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Status of read operation (true = success)
 */
PUBLIC bool readByte(uint8 devAddr, uint8 regAddr, uint8 *data) {
	return readBytes(devAddr, regAddr, 1, data);
}

/** Write multiple words to a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register address to write to
 * @param length Number of words to write
 * @param data Buffer to copy new data from
 * @return Status of operation (true = success)
 */
bool writeWords(uint8 devAddr, uint8 regAddr, uint8 length, uint16* data) {
	uint8 i = 0;
	bool b = false;
	if (bStartReading(devAddr)) {
		if (bWriteByte(regAddr)) {
			for (i = 0; i < (length - 1) * 2; i++) {
				b = bWriteByte((uint8) (data[i++] >> 8));	// send MSB
				b |= bWriteByte((uint8) (data[i]));		// send LSB
				if (!b)
					return FALSE;
			}
			b = bWriteByte((uint8) (data[length - 1] >> 8));		// send MSB
			b |= bWriteLastByte((uint8) (data[length - 2]));		// send LSB
			if (b)
				return TRUE;
			else
				return FALSE;
		}
	}
	return FALSE;
}

/** Write single word to a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register address to write to
 * @param data New word value to write
 * @return Status of operation (true = success)
 */
PUBLIC bool writeWord(uint8 devAddr, uint8 regAddr, uint16 data) {
	return writeWords(devAddr, regAddr, 1, &data);
}

/** Write multiple bits in an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitStart First bit position to write (0-7)
 * @param length Number of bits to write (not more than 8)
 * @param data Right-aligned value to write
 * @return Status of operation (true = success)
 */
PUBLIC bool writeBits(uint8 devAddr, uint8 regAddr, uint8 bitStart,
		uint8 length, uint8 data) {
	//      010 value to write
	// 76543210 bit numbers
	//    xxx   args: bitStart=4, length=3
	// 00011100 mask byte
	// 10101111 original value (sample)
	// 10100011 original & ~mask
	// 10101011 masked | value
	uint8 b;
	if (readByte(devAddr, regAddr, &b) != 0) {
		uint8 mask = ((1 << length) - 1) << (bitStart - length + 1);
		data <<= (bitStart - length + 1); // shift data into correct position
		data &= mask; // zero all non-important bits in data
		b &= ~(mask); // zero all important bits in existing byte
		b |= data; // combine data with existing byte
		return writeByte(devAddr, regAddr, b);
	} else {
		return false;
	}
}

/** write a single bit in an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitNum Bit position to write (0-7)
 * @param value New bit value to write
 * @return Status of operation (true = success)
 */
PUBLIC bool writeBit(uint8 devAddr, uint8 regAddr, uint8 bitNum, uint8 data) {
	uint8 b;
	readByte(devAddr, regAddr, &b);
	b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
	return writeByte(devAddr, regAddr, b);
}

/** Write multiple bytes to an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register address to write to
 * @param length Number of bytes to write
 * @param data Buffer to copy new data from
 * @return Status of operation (true = success)
 */
PUBLIC bool writeBytes(uint8 devAddr, uint8 regAddr, uint8 length, uint8* data) {
	uint8 i = 0;

	if (bStartWriting(devAddr)) {
		if (bWriteByte(regAddr)) {
			for (i = 0; i < length - 1; i++) {
				if (!bWriteByte(data[i]))
					return FALSE;
			}
			return bWriteLastByte(data[length - 1]);
		}
	}
	return FALSE;
}

/** Write single byte to an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register address to write to
 * @param data New byte value to write
 * @return Status of operation (true = success)
 */
PUBLIC bool writeByte(uint8 devAddr, uint8 regAddr, uint8 data) {
	return writeBytes(devAddr, regAddr, (uint8)1, &data);
}

PUBLIC bool bStartWriting(uint8 slaveAddress) {
	bool b = FALSE;

	vAHI_SiMasterWriteSlaveAddr(slaveAddress, FALSE);
	bAHI_SiMasterSetCmdReg( E_AHI_SI_START_BIT,
							E_AHI_SI_NO_STOP_BIT,
							E_AHI_SI_NO_SLAVE_READ,
							E_AHI_SI_SLAVE_WRITE,
							E_AHI_SI_SEND_ACK,
							E_AHI_SI_NO_IRQ_ACK); // not sure for the nack
	while (bAHI_SiMasterPollTransferInProgress());
	b = bAHI_SiMasterCheckRxNack();

	return !b;  //expecting ack
}

PUBLIC bool bWriteByte(uint8 data) {
	bool b = FALSE;

	vAHI_SiMasterWriteData8(data);
	bAHI_SiMasterSetCmdReg( E_AHI_SI_NO_START_BIT,
							E_AHI_SI_NO_STOP_BIT,
							E_AHI_SI_NO_SLAVE_READ,
							E_AHI_SI_SLAVE_WRITE,
							E_AHI_SI_SEND_ACK,
							E_AHI_SI_NO_IRQ_ACK);
	while (bAHI_SiMasterPollTransferInProgress());
	b = bAHI_SiMasterCheckRxNack();

	return !b;  //expecting ack
}

PUBLIC bool bWriteLastByte(uint8 data) {
	bool b = FALSE;

	vAHI_SiMasterWriteData8(data);
	bAHI_SiMasterSetCmdReg( E_AHI_SI_NO_START_BIT,
							E_AHI_SI_NO_STOP_BIT,
							E_AHI_SI_NO_SLAVE_READ,
							E_AHI_SI_SLAVE_WRITE,
							E_AHI_SI_SEND_ACK,
							E_AHI_SI_NO_IRQ_ACK);
	while (bAHI_SiMasterPollTransferInProgress());
	b = bAHI_SiMasterCheckRxNack();

	bAHI_SiMasterSetCmdReg( E_AHI_SI_NO_START_BIT,
							E_AHI_SI_STOP_BIT,
							E_AHI_SI_NO_SLAVE_READ,
							E_AHI_SI_NO_SLAVE_WRITE,
							E_AHI_SI_SEND_NACK,	//not sure for the ack
							E_AHI_SI_NO_IRQ_ACK);
	return b;  //expecting nack
}

PUBLIC bool bStartReading(uint8 slaveAddress) {
	bool b = FALSE;

	vAHI_SiMasterWriteSlaveAddr(slaveAddress, TRUE);
	bAHI_SiMasterSetCmdReg( E_AHI_SI_START_BIT,
							E_AHI_SI_NO_STOP_BIT,
							E_AHI_SI_NO_SLAVE_READ,
							E_AHI_SI_SLAVE_WRITE,
							E_AHI_SI_SEND_ACK,
							E_AHI_SI_NO_IRQ_ACK);
	while (bAHI_SiMasterPollTransferInProgress());
	b = bAHI_SiMasterCheckRxNack();
	return !b; //expecting ack
}

PUBLIC uint8 u8ReadByte() {
	uint8 readByte = 0;
	bAHI_SiMasterSetCmdReg( E_AHI_SI_NO_START_BIT,
							E_AHI_SI_NO_STOP_BIT,
							E_AHI_SI_SLAVE_READ,
							E_AHI_SI_NO_SLAVE_WRITE,
							E_AHI_SI_SEND_ACK,
							E_AHI_SI_NO_IRQ_ACK);
	while (bAHI_SiMasterPollTransferInProgress());
	readByte = u8AHI_SiMasterReadData8();

	return readByte;
}

PUBLIC uint8 u8ReadLastByte() {
	uint8 readByte = 0;
	bAHI_SiMasterSetCmdReg(E_AHI_SI_NO_START_BIT,
							E_AHI_SI_NO_STOP_BIT,
							E_AHI_SI_SLAVE_READ,
							E_AHI_SI_NO_SLAVE_WRITE,
							E_AHI_SI_SEND_NACK,
							E_AHI_SI_NO_IRQ_ACK);
	while (bAHI_SiMasterPollTransferInProgress());
	readByte = u8AHI_SiMasterReadData8();

	bAHI_SiMasterSetCmdReg( E_AHI_SI_NO_START_BIT,
							E_AHI_SI_STOP_BIT,
							E_AHI_SI_NO_SLAVE_READ,
							E_AHI_SI_NO_SLAVE_WRITE,
							E_AHI_SI_SEND_NACK,			//Last bit, no ack to signify the end
							E_AHI_SI_NO_IRQ_ACK);
	return readByte;
}
