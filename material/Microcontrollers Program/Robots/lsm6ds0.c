/*
    Pretty LAYer for ChibiOS/RT - Copyright (C) 2014 Rocco Marco Guglielmi

    This file is part of PLAY for ChibiOS/RT.

    PLAY is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    PLAY is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
    Special thanks to Giovanni Di Sirio for teachings, his moral support and
    friendship. Note that some or every piece of this file could be part of
    the ChibiOS project that is intellectual property of Giovanni Di Sirio.
    Please refer to ChibiOS/RT license before use this file.
 */

/**
 * @file    lsm6ds0.c
 * @brief   LSM6DS0 MEMS interface module through I2C code.
 *
 * @addtogroup lsm6ds0
 * @{
 */
#include "lsm6ds0.h"
#include "robot_functions.h"
#include <dbg.h>
#include <dbg_uart.h>
#include "I2C.h"
#include "Timers.h"

uint8 IMUBuffer[22] = {0};

/** Power on and prepare for general usage.
 * This will activate the device and take it out of sleep mode (which must be done
 * after start-up). This function also sets both the accelerometer and the gyroscope
 * to their most sensitive settings, namely +/- 2g and +/- 250 degrees/sec, and sets
 * the clock source to use the X Gyro for reference, which is slightly better than
 * the default internal clock source.
 */
PUBLIC void initializeLSM6DS0(bool useAccel, bool useGyro) {
	xAccelOffset = yAccelOffset = zAccelOffset = 0;
	xGyroOffset = yGyroOffset = zGyroOffset = 0;

	gyroEnabled = useGyro;
	accelEnabled = useAccel;

	if(gyroEnabled)
	{
		gScale = GYRO_FS_245DSP;
		calcGyroRes();
	}

	if(accelEnabled)
	{
		aScale = ACC_FS_2G;
		calcAccelRes();
	}

	if(testConnection())
	{
		resetLSM6DS0();

		if(accelEnabled)
		{
			initAccel();
			setFullScaleAccelRange(aScale);
			setAccelODR(ACC_ODR_119Hz);
		}

		if(gyroEnabled)
		{
			initGyro();
			setFullScaleGyroRange(gScale);
			setGyroODR(GYRO_ODR_119Hz_CO_14Hz);
		}

		calibrateLSM6DS0(gbias,abias);
	}
}

PUBLIC void initGyro()
{
	writeByte(LSM6DS0_ADDRESS,CTRL_REG1_G, 0x60); // 119Hz ODR, fs 245dps
	vWaitMicroseconds(500);
	writeByte(LSM6DS0_ADDRESS,CTRL_REG3_G, 0x00); // Low power disabled,  Normal mode, high cutoff frequency
	vWaitMicroseconds(500);
	writeByte(LSM6DS0_ADDRESS,CTRL_REG4, 0x38); // Normal mode, enable all axes
	vWaitMicroseconds(500);
}

PUBLIC void initAccel()
{
	writeByte(LSM6DS0_ADDRESS,CTRL_REG5_XL, 0x38); // Normal mode, enable all axes
	vWaitMicroseconds(500);
	writeByte(LSM6DS0_ADDRESS,CTRL_REG6_XL, 0x60); // 119Hz ODR, ±2g, 50Hz BW, 408Hz AA
	vWaitMicroseconds(500);
	writeByte(LSM6DS0_ADDRESS,CTRL_REG7_XL, 0x00); // Filters off
	vWaitMicroseconds(500);
}

PUBLIC void calibrateLSM6DS0(float *gbias, float *abias)
{
	int64 gyro_bias[3] = {0, 0, 0}, accel_bias[3] = {0, 0, 0};
	int ii;
	uint8 samples=0;

	writeBit(LSM6DS0_ADDRESS,CTRL_REG9,1,1); 	 // Enable FIFO
	vWaitMilliseconds(20);
	writeByte(LSM6DS0_ADDRESS,FIFO_CTRL, 0x20 | 0x1F);  // Enable gyro FIFO stream mode and set watermark at 32 samples
	vWaitMilliseconds(1000);  // delay 1000 milliseconds to collect FIFO samples

	readByte(LSM6DS0_ADDRESS,FIFO_SRC,IMUBuffer);
	samples = IMUBuffer[0] & 0x3F; // Read number of stored samples

	for(ii = 0; ii < samples ; ii++)  // Read the gyro data stored in the FIFO
	{
		if(gyroEnabled)
		{
			readBytes(LSM6DS0_ADDRESS, OUT_X_L_G, 6, IMUBuffer);
			gyro_bias[0] += (((int16)IMUBuffer[1] << 8) | IMUBuffer[0]);
			gyro_bias[1] += (((int16)IMUBuffer[3] << 8) | IMUBuffer[2]);
			gyro_bias[2] += (((int16)IMUBuffer[5] << 8) | IMUBuffer[4]);
			vWaitMicroseconds(500);
		}

		if(accelEnabled)
		{
			readBytes(LSM6DS0_ADDRESS, OUT_X_L_XL, 6, IMUBuffer);
			accel_bias[0] += (((int16)IMUBuffer[1] << 8) | IMUBuffer[0]);
			accel_bias[1] += (((int16)IMUBuffer[3] << 8) | IMUBuffer[2]);
			accel_bias[2] += (((int16)IMUBuffer[5] << 8) | IMUBuffer[4]) - (int16)(1./aRes); // Assumes sensor facing up!
			vWaitMicroseconds(500);
		}
	}

	if(gyroEnabled)
	{
		gyro_bias[0] /= samples; // average the data
		gyro_bias[1] /= samples;
		gyro_bias[2] /= samples;

		gbias[0] = (float)gyro_bias[0]*gRes;  // Properly scale the data to get deg/s
		gbias[1] = (float)gyro_bias[1]*gRes;
		gbias[2] = (float)gyro_bias[2]*gRes;
	}

	if(accelEnabled)
	{
	  accel_bias[0] /= samples; // average the data
	  accel_bias[1] /= samples;
	  accel_bias[2] /= samples;

	  abias[0] = (float)accel_bias[0]*aRes; // Properly scale data to get gs
	  abias[1] = (float)accel_bias[1]*aRes;
	  abias[2] = (float)accel_bias[2]*aRes;
	}

	writeBit(LSM6DS0_ADDRESS,CTRL_REG9,1,0); 	 // Disable FIFO
	vWaitMilliseconds(20);

	writeByte(LSM6DS0_ADDRESS,FIFO_CTRL, 0x00);   // Enable bypass mode
}

/** Verify the I2C connection.
 * Make sure the device is connected and responds as expected.
 * @return True if connection is valid, false otherwise
 */
PUBLIC bool testConnection() {
	return getIMUDeviceID() == 0x68;
}

PUBLIC void setAccelODR(uint8 rate) {
	writeBits(LSM6DS0_ADDRESS, CTRL_REG6_XL, LSM6DS0_GYRO_ODR_SEL_BIT, LSM6DS0_GYRO_ODR_SEL_LENGTH, rate);
	vWaitMicroseconds(500);
}

PUBLIC void setGyroODR(uint8 rate) {
	writeBits(LSM6DS0_ADDRESS, CTRL_REG1_G, LSM6DS0_GYRO_ODR_SEL_BIT, LSM6DS0_GYRO_ODR_SEL_LENGTH, rate);
	vWaitMicroseconds(500);
}

/** Set full-scale gyroscope range.
 * @param range New full-scale gyroscope range value
 * @see getFullScaleRange()
 * @see MPU6050_GYRO_FS_250
 * @see MPU6050_RA_GYRO_CONFIG
 * @see MPU6050_GCONFIG_FS_SEL_BIT
 * @see MPU6050_GCONFIG_FS_SEL_LENGTH
 */
PUBLIC void setFullScaleGyroRange(uint8 range) {
	writeBits(LSM6DS0_ADDRESS, CTRL_REG1_G, LSM6DS0_GYRO_FS_SEL_BIT, LSM6DS0_GYRO_FS_SEL_LENGTH, range);
	vWaitMicroseconds(500);
}

// ACCEL_CONFIG register

/** Set self-test enabled setting for accelerometer X axis.
 * @param enabled Self-test enabled value
 * @see MPU6050_RA_ACCEL_CONFIG
 */
PUBLIC void setAccelSelfTest(bool enabled) {
	writeBit(LSM6DS0_ADDRESS, CTRL_REG10, LSM6DS0_ACCEL_ST_BIT, enabled);
	vWaitMicroseconds(500);
}

/** Set self-test enabled value for accelerometer Y axis.
 * @param enabled Self-test enabled value
 * @see MPU6050_RA_ACCEL_CONFIG
 */
PUBLIC void setGyroSelfTest(bool enabled) {
	writeBit(LSM6DS0_ADDRESS, CTRL_REG10, LSM6DS0_GYRO_ST_BIT, enabled);
	vWaitMicroseconds(500);
}

/** Set full-scale accelerometer range.
 * @param range New full-scale accelerometer range setting
 * @see getFullScaleAccelRange()
 */
PUBLIC void setFullScaleAccelRange(uint8 range) {
	writeBits(LSM6DS0_ADDRESS, CTRL_REG6_XL, LSM6DS0_ACCEL_FS_SEL_BIT, LSM6DS0_ACCEL_FS_SEL_LENGTH, range);
	vWaitMicroseconds(500);
}


/** Get 3-axis accelerometer readings.
 * These registers store the most recent accelerometer measurements.
 * Accelerometer measurements are written to these registers at the Sample Rate
 * as defined in Register 25.
 *
 * The accelerometer measurement registers, along with the temperature
 * measurement registers, gyroscope measurement registers, and external sensor
 * data registers, are composed of two sets of registers: an internal register
 * set and a user-facing read register set.
 *
 * The data within the accelerometer sensors' internal register set is always
 * updated at the Sample Rate. Meanwhile, the user-facing read register set
 * duplicates the internal register set's data values whenever the serial
 * interface is idle. This guarantees that a burst read of sensor registers will
 * read measurements from the same sampling instant. Note that if burst reads
 * are not used, the user is responsible for ensuring a set of single byte reads
 * correspond to a single sampling instant by checking the Data Ready interrupt.
 *
 * Each 16-bit accelerometer measurement has a full scale defined in ACCEL_FS
 * (Register 28). For each full scale setting, the accelerometers' sensitivity
 * per LSB in ACCEL_xOUT is shown in the table below:
 *
 * <pre>
 * AFS_SEL | Full Scale Range | LSB Sensitivity
 * --------+------------------+----------------
 * 0       | +/- 2g           | 8192 LSB/mg
 * 1       | +/- 4g           | 4096 LSB/mg
 * 2       | +/- 8g           | 2048 LSB/mg
 * 3       | +/- 16g          | 1024 LSB/mg
 * </pre>
 *
 * @param x 16-bit signed integer container for X-axis acceleration
 * @param y 16-bit signed integer container for Y-axis acceleration
 * @param z 16-bit signed integer container for Z-axis acceleration
 * @see MPU6050_RA_GYRO_XOUT_H
 */
PUBLIC bool getAcceleration(int16* x, int16* y, int16* z) {
	bool b = false;
	if(accelEnabled)
	{
		b = readBytes(LSM6DS0_ADDRESS, OUT_X_L_XL, 6, IMUBuffer);
		*x = ((((int16)IMUBuffer[1]) << 8) | IMUBuffer[0]);
		*y = ((((int16)IMUBuffer[3]) << 8) | IMUBuffer[2]);
		*z = ((((int16)IMUBuffer[5]) << 8) | IMUBuffer[4]);
		vWaitMicroseconds(500);
	}
	return b;
}

/** Get 3-axis gyroscope readings.
 * These gyroscope measurement registers, along with the accelerometer
 * measurement registers, temperature measurement registers, and external sensor
 * data registers, are composed of two sets of registers: an internal register
 * set and a user-facing read register set.
 * The data within the gyroscope sensors' internal register set is always
 * updated at the Sample Rate. Meanwhile, the user-facing read register set
 * duplicates the internal register set's data values whenever the serial
 * interface is idle. This guarantees that a burst read of sensor registers will
 * read measurements from the same sampling instant. Note that if burst reads
 * are not used, the user is responsible for ensuring a set of single byte reads
 * correspond to a single sampling instant by checking the Data Ready interrupt.
 *
 * Each 16-bit gyroscope measurement has a full scale defined in FS_SEL
 * (Register 27). For each full scale setting, the gyroscopes' sensitivity per
 * LSB in GYRO_xOUT is shown in the table below:
 *
 * <pre>
 * FS_SEL | Full Scale Range   | LSB Sensitivity
 * -------+--------------------+----------------
 * 0      | +/- 250 degrees/s  | 131 LSB/deg/s
 * 1      | +/- 500 degrees/s  | 65.5 LSB/deg/s
 * 2      | +/- 1000 degrees/s | 32.8 LSB/deg/s
 * 3      | +/- 2000 degrees/s | 16.4 LSB/deg/s
 * </pre>
 *
 * @param x 16-bit signed integer container for X-axis rotation
 * @param y 16-bit signed integer container for Y-axis rotation
 * @param z 16-bit signed integer container for Z-axis rotation
 * @see getMotion6()
 * @see MPU6050_RA_GYRO_XOUT_H
 */
PUBLIC void getRotation(int16* x, int16* y, int16* z) {
	if(gyroEnabled)
	{
		readBytes(LSM6DS0_ADDRESS, OUT_X_L_G, 6, IMUBuffer);
		*x = ((((int16)IMUBuffer[1]) << 8) | IMUBuffer[0]);
		*y = ((((int16)IMUBuffer[3]) << 8) | IMUBuffer[2]);
		*z = ((((int16)IMUBuffer[5]) << 8) | IMUBuffer[4]);
		vWaitMicroseconds(500);
	}
}
/** Get X-axis accelerometer reading.
 * @return X-axis acceleration measurement in 16-bit 2's complement format
 * @see getMotion6()
 * @see MPU6050_RA_ACCEL_XOUT_H
 */
PUBLIC int16 getAccelerationX() {
	if(accelEnabled)
	{
		readBytes(LSM6DS0_ADDRESS, OUT_X_L_XL, 2, IMUBuffer);
		return (((int16)IMUBuffer[1]) << 8) | IMUBuffer[0];
	}
	else
		return 0;
}
/** Get Y-axis accelerometer reading.
 * @return Y-axis acceleration measurement in 16-bit 2's complement format
 * @see getMotion6()
 * @see MPU6050_RA_ACCEL_YOUT_H
 */
PUBLIC int16 getAccelerationY() {
	if(accelEnabled)
	{
		readBytes(LSM6DS0_ADDRESS, OUT_Y_L_XL, 2, IMUBuffer);
		return (((int16)IMUBuffer[1]) << 8) | IMUBuffer[0];
	}
	else
		return 0;
}
/** Get Z-axis accelerometer reading.
 * @return Z-axis acceleration measurement in 16-bit 2's complement format
 * @see getMotion6()
 * @see MPU6050_RA_ACCEL_ZOUT_H
 */
PUBLIC int16 getAccelerationZ() {
	if(accelEnabled)
	{
		readBytes(LSM6DS0_ADDRESS, OUT_Z_L_XL, 2, IMUBuffer);
		return (((int16)IMUBuffer[1]) << 8) | IMUBuffer[0];
	}
	else
		return 0;
}

// TEMP_OUT_* registers

/** Get current internal temperature.
 * @return Temperature reading in 16-bit 2's complement format
 * @see MPU6050_RA_TEMP_OUT_H
 */
PUBLIC int16 getTemperature() {
	readBytes(LSM6DS0_ADDRESS, OUT_TEMP_L, 2, IMUBuffer);
	return (((int16)IMUBuffer[1]) << 8) | IMUBuffer[0];
}

/** Get X-axis gyroscope reading.
 * @return X-axis rotation measurement in 16-bit 2's complement format
 * @see getMotion6()
 * @see MPU6050_RA_GYRO_XOUT_H
 */
PUBLIC int16 getRotationX() {
	if(gyroEnabled)
	{
		readBytes(LSM6DS0_ADDRESS, OUT_X_L_G, 2, IMUBuffer);
		return (((int16)IMUBuffer[1]) << 8) | IMUBuffer[0];
	}
	else
		return 0;
}
/** Get Y-axis gyroscope reading.
 * @return Y-axis rotation measurement in 16-bit 2's complement format
 * @see getMotion6()
 * @see MPU6050_RA_GYRO_YOUT_H
 */
PUBLIC int16 getRotationY() {
	if(gyroEnabled)
	{
		readBytes(LSM6DS0_ADDRESS, OUT_Y_L_G, 2, IMUBuffer);
		return (((int16)IMUBuffer[1]) << 8) | IMUBuffer[0];
	}
	else
		return 0;
}
/** Get Z-axis gyroscope reading.
 * @return Z-axis rotation measurement in 16-bit 2's complement format
 * @see getMotion6()
 * @see MPU6050_RA_GYRO_ZOUT_H
 */
PUBLIC int16 getRotationZ() {
	if(gyroEnabled)
	{
		readBytes(LSM6DS0_ADDRESS, OUT_Z_L_G, 2, IMUBuffer);
		return (((int16)IMUBuffer[1]) << 8) | IMUBuffer[0];
	}
	else
		return 0;
}

// PWR_MGMT_1 register

/** Trigger a full device reset.
 * A small delay of ~50ms may be desirable after triggering a reset.
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_DEVICE_RESET_BIT
 */
PUBLIC void resetLSM6DS0() {
	writeBit(LSM6DS0_ADDRESS, CTRL_REG8, LSM6DS0_DEVICE_RESET_BIT, true);
	vWaitMilliseconds(50);
}

// WHO_AM_I register

/** Get Device ID.
 * This register is used to verify the identity of the device (0b110100, 0xD4).
 * @return Device ID ( should be 0xD4)
 * @see LSM6DS0_SUB_WHO_AM_I
 * @see LSM6DS0_SUB_WHO_AM_I_BIT
 * @see LSM6DS0_SUB_WHO_AM_I_LENGTH
 */
PUBLIC uint8 getIMUDeviceID() {
	readByte(LSM6DS0_ADDRESS, WHO_AM_I, IMUBuffer);
	vWaitMicroseconds(500);
	return IMUBuffer[0];
}

PUBLIC void calcGyroRes()
{
	// Possible gyro scales (and their register bit settings) are:
	// 245 DPS (00), 500 DPS (01), 2000 DPS (10). Here's a bit of an algorithm
	// to calculate DPS/(ADC tick) based on that 2-bit value:
	switch (gScale)
	{
		case GYRO_FS_245DSP:
			gRes = 245.0 / 32768.0;
			break;
		case GYRO_FS_500DSP:
			gRes = 500.0 / 32768.0;
			break;
		case GYRO_FS_2000DSP:
			gRes = 2000.0 / 32768.0;
			break;
	}
	DBG_vPrintf(DEBUG_DEVICE_CONFIG, "Gyro resolution: %d\n", gRes);
}

PUBLIC void calcAccelRes()
{
	// Possible accelerometer scales (and their register bit settings) are:
	// 2 g (000), 4g (001), 6g (010) 8g (011), 16g (100). Here's a bit of an
	// algorithm to calculate g/(ADC tick) based on that 3-bit value:
	aRes = (aScale == ACC_FS_16G) ? 16.0f / 32768.0f : (((float) aScale + 1.0f) * 2.0f) / 32768.0f;
}
