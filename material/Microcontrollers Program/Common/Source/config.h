/*****************************************************************************
 *
 * MODULE:              config.h
 *
 * COMPONENT:           config.h,v
 *
 * VERSION:
 *
 * REVISION:            1.2
 *
 * DATED:               2006/09/07 14:23:55
 *
 * STATUS:              Exp
 *
 * AUTHOR:
 *
 * DESCRIPTION:
 *
 * LAST MODIFIED BY:
 *                      $Modtime: $
 *
 ****************************************************************************
 *
 * This software is owned by Jennic and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on Jennic products. You, and any third parties must reproduce
 * the copyright and warranty notice and any other legend of ownership on each
 * copy or partial copy of the software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS". JENNIC MAKES NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * ACCURACY OR LACK OF NEGLIGENCE. JENNIC SHALL NOT, IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT LIMITED TO, SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON WHATSOEVER.
 *
 * Copyright Jennic Ltd 2009. All rights reserved
 *
 ****************************************************************************/

#ifndef  CONFIG_H_INCLUDED
#define  CONFIG_H_INCLUDED

#if defined __cplusplus
extern "C"
{
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <jendefs.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define DBG_ENABLE

/* Network parameters */
#define PAN_ID                      0xCAFE
#define COORDINATOR_ADR             0x0000
#define END_DEVICE_START_ADR        0x0001
#define MAX_END_DEVICES             100

/* Defines the channels to scan. Each bit represents one channel. All channels
 in the channels (11-26) in the 2.4GHz band are scanned. */
#define SCAN_CHANNELS               0x07FFF800UL
#define CHANNEL_MIN                 11

/* Duration (ms) = 15.36ms x (2^ACTIVE_SCAN_DURATION + 1) */
#define ACTIVE_SCAN_DURATION        3
/* Duration (ms) = 15.36ms x (2^ENERGY_SCAN_DURATION + 1) */
#define ENERGY_SCAN_DURATION        3

/* Custom macros */
#define ON							TRUE
#define OFF							FALSE
#define false						FALSE
#define true						TRUE

/* Messages types */
#define DBG 							0
#define IMU_RAW_DATA 					1
#define ACCEL_RAW_DATA 					2
#define GYRO_RAW_DATA 					3
#define TOUCH_RAW_DATA 					8
#define STATUS_DATA						9
#define ROBOT_MOTORS_VALUES				7
#define TIME_DATA						8

#define DATA_TO_STREAM					0xF
#define STREAM_IMU						0xA
#define STREAM_TOUCH					0xB
#define STREAM_STATUS					0xC
#define STREAM_ACCEL					0xD
#define STREAM_GYRO						0xE
#define STREAM_TIMESTAMP				0x9

#define ASK_FOR_DATA					0xAA
#define SET_TIMESTAMP					0xAB
#define ASK_FOR_TIMESTAMP				0xAC
#define MEASURE_TIME_OF_FLIGHT			0xAD
#define MESSAGE_SIZE					0x02

/* Debug flags */
#ifdef DBG_ENABLE
#define DEBUG_EXCEPTION							TRUE
#define DEBUG_INCOMING							TRUE
#define DEBUG_DEVICE_FUNC					    TRUE
#define DEBUG_DEVICE_VARS					    TRUE
#define DEBUG_DEVICE_SENSOR						TRUE
#define DEBUG_DEVICE_INT						TRUE
#define DEBUG_DEVICE_CONFIG						TRUE
#define DEBUG_CBUFFER							FALSE
#define DEBUG_STATE_MACHINE						TRUE

#else
#define DEBUG_EXCEPTION							FALSE
#define DEBUG_DEVICE_FUNC					    FALSE
#define DEBUG_INCOMING							FALSE
#define DEBUG_DEVICE_VARS					    FALSE
#define DEBUG_DEVICE_SENSOR						FALSE
#define DEBUG_DEVICE_INT						FALSE
#define DEBUG_DEVICE_CONFIG						FALSE
#define DEBUG_CBUFFER							FALSE
#define DEBUG_STATE_MACHINE						FALSE
#endif

/****************************************************************************/
/***        Pins Definitions                                             ***/
/****************************************************************************/
#define ROBOT_TOUCH_CHANGE 	1<<12
#define ROBOT_IMU_INT		1<<17
#define DONGLE_STATUS_LED	1<<15

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef uint8 State;
typedef uint8 Transition;
typedef uint8 Transition;
typedef uint8 Grip;

typedef struct Message
{
	uint8 message_id;
	uint8 message_size;
	uint16 payload[7];
} Message;

typedef struct
{
	int16 ax;
	int16 ay;
	int16 az;
	int16 gx;
	int16 gy;
	int16 gz;
} ImuRawData;

typedef struct
{
	int16 idx;
	int16 ax;
	int16 ay;
	int16 az;
} AccelRawData;

typedef struct
{
	int16 gx;
	int16 gy;
	int16 gz;
} GyroRawData;

typedef struct
{
	int16 iAx;
	int16 iAy;
	int16 iAz;
	int16 iGx;
	int16 iGy;
	int16 iGz;
} ImuIntegrals;

typedef struct
{
	Transition transition;
	Grip grip;
} StatusData;

typedef struct
{
	State currentState;
	State previousState;
} ManipulationState;

/****************************************************************************/
/***        Enums                                         			      ***/
/****************************************************************************/

/* Manipulation states */
typedef enum
{
	NO_MOTION = 0x00, MOVING = 0x01, FALLING = 0x02
} MOTION_STATE_t;

typedef enum
{
	NO_GRIP = 0x00,
	ONE_FINGER_SIDE = 0x01,
	TWO_FINGERS_SIDE_PINCH = 0x02,
	THREE_FINGERS_SIDE_PINCH = 0x03,
	FOUR_FINGERS_SIDE_PINCH = 0x04,
	ONE_FINGER_TOP_OR_BOTTOM = 0x05,
	TWO_FINGERS_TOP_BOTTOM_PINCH = 0x06
} GRIP_TYPES_t;

typedef enum
{
	NO_TRANSITION = 0x0,
	FALL = 0x14,
	TOUCH = 0x12,
	RELEASE = 0x21,
	CONTACT_STILL_GRIP_CHANGED = 0x22,
	MOVE = 0x23,
	STOP = 0x32,
	CONTACT_MOVING_GRIP_CHANGED = 0x33,
	THROW = 0x34,
	HALT = 0x41,
	HIT_CATCH = 0x43
} TRANSITIONS_t;

typedef enum
{
	NO_CONTACT_STILL = 1,
	CONTACT_STILL = 2,
	CONTACT_MOVING = 3,
	NO_CONTACT_MOVING = 4
} STATES_t;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* CONFIG_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
