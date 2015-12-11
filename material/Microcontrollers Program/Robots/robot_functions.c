#include "robot_functions.h"
#include <dbg.h>
#include <dbg_uart.h>
#include "I2C.h"
#include "Timers.h"
#include "config.h"
#include <math.h>

#define FREEFALL_THRES 20
#define MOTION_THRES 100
#define HISTORY_LENGTH 5

/****************************************************************************/
/***        Local Variables                                             ***/
/****************************************************************************/
uint8 freefallHistory, motionHistory;
PRIVATE int64 gravity, aNorm;

PRIVATE USBMessage m;
PRIVATE ManipulationState manipulationState;
PRIVATE uint8 currentTouch, previousTouch;
AccelRawData acceleration;
GyroRawData rotation;
ImuRawData imuData;

extern uint64 timestamp;

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
PUBLIC void vRobotInitDIOs(void)
{
	DBG_vPrintf(DEBUG_DEVICE_CONFIG, "vRobotInitDIOs\n");
	/* Initialise DIOs */
	vAHI_DioSetPullup(0xFFFFFFFF, 0);
	vAHI_DioSetDirection(ROBOT_TOUCH_CHANGE | ROBOT_IMU_INT, 0);
}

PUBLIC void vRobotInitSensors(void)
{
	freefallHistory = motionHistory = 0;
	gravity = aNorm = 0;
	previousTouch = currentTouch = 0;
	manipulationState.currentState = manipulationState.previousState = NO_CONTACT_STILL;

	initializeLSM6DS0(true, false);
	initQtInterface();
	measureGravity();
}

PUBLIC void measureGravity()
{
	uint16 i = 0;
	int64 gNorm;

	gravity = 0;

	for (i = 0; i < 2000; i += ROBOT_TICK_PERIOD_MS)
	{
		getAcceleration(&acceleration.ax, &acceleration.ay, &acceleration.az);
		gNorm = (int64) acceleration.ax * acceleration.ax;
		gNorm += (int64) acceleration.ay * acceleration.ay;
		gNorm += (int64) acceleration.az * acceleration.az;

		gravity = (gravity + gNorm) / 2;
		vWaitMilliseconds(10);
	}
	DBG_vPrintf(DEBUG_DEVICE_SENSOR, "g=%d\n", (int32)gravity);
}

PUBLIC void vRobotUpdateIMUData(uint8 msTimestep)
{
	acceleration.idx = (uint16) timestamp;
	getAcceleration(&acceleration.ax, &acceleration.ay, &acceleration.az);

	aNorm = (int64) acceleration.ax * acceleration.ax;
	aNorm += (int64) acceleration.ay * acceleration.ay;
	aNorm += (int64) acceleration.az * acceleration.az;

	aNorm -= gravity; //average g measured at startup

	if (aNorm < 0)
		aNorm *= -1;

	aNorm = aNorm >> 15;
	vMonitorMovement();
}

PUBLIC bool bRobotgetTouchedKeys(uint8* touchedKeys)
{
	vWaitMicroseconds(100);
	if (isQTDataReady())
	{
		currentTouch = readQTKeyStatus();
		if (currentTouch != previousTouch)
		{
			DBG_vPrintf(DEBUG_DEVICE_SENSOR, "touch:%d\n", currentTouch);
			*touchedKeys = currentTouch;
			previousTouch = currentTouch;
			return true;
		}
	}
	return false;
}

PUBLIC void vMonitorMovement()
{
	if (currentTouch == 0)
	{
		motionHistory = 0;
		if (aNorm > 5000)
		{
			if (freefallHistory <= 255 - ROBOT_TICK_PERIOD_MS)
				freefallHistory += ROBOT_TICK_PERIOD_MS;
		}
		else
			freefallHistory = 0;
	}
	else
	{
		if (aNorm > 2000)
		{
			if (motionHistory <= 2 * MOTION_THRES)
				motionHistory += ROBOT_TICK_PERIOD_MS;
		}
		else
		{
			if (motionHistory >= ROBOT_TICK_PERIOD_MS)
				motionHistory -= ROBOT_TICK_PERIOD_MS;
		}
	}
}

PUBLIC bool bChangeInFreefall()
{
	static bool currentFall = false, previousFall = false;

	currentFall = (freefallHistory > FREEFALL_THRES) ? true : false;
	if (currentFall != previousFall)
	{
		if (currentFall)
			DBG_vPrintf(DEBUG_DEVICE_SENSOR, "freefall\n");
		else
			DBG_vPrintf(DEBUG_DEVICE_SENSOR, "no freefall\n");

		previousFall = currentFall;

		return true;
	}
	return false;
}

PUBLIC bool bChangeInMovement()
{
	static bool currentMovement = false, previousMovement = false;

	currentMovement = (motionHistory > MOTION_THRES) ? true : false;
	if (currentMovement != previousMovement)
	{
		if (currentMovement)
			DBG_vPrintf(DEBUG_DEVICE_SENSOR, "in motion\n");
		else
			DBG_vPrintf(DEBUG_DEVICE_SENSOR, "still\n");

		previousMovement = currentMovement;

		return true;
	}
	return false;
}

PUBLIC bool bRobotUpdateGrip(Grip* grip)
{
	uint8 touchedKeys;

	bool touchUpdate = bRobotgetTouchedKeys(&touchedKeys);

	if (touchUpdate)
	{
		*grip = touchedKeys;

//		switch (touchedKeys)
//		{
//		case 0x00:
//			*grip = NO_GRIP;
//			break;
//		case 0x01:
//		case 0x02:
//		case 0x04:
//		case 0x08:
//			*grip = ONE_FINGER_SIDE;
//			break;
//		case 0x10:
//		case 0x20:
//			*grip = ONE_FINGER_TOP_OR_BOTTOM;
//			break;
//		case 0x05:
//		case 0x06:
//		case 0x09:
//		case 0x0A:
//			*grip = TWO_FINGERS_SIDE_PINCH;
//			break;
//		case 0x07:
//		case 0x0B:
//		case 0x0D:
//		case 0x0E:
//			*grip = THREE_FINGERS_SIDE_PINCH;
//			break;
//		case 0x0F:
//			*grip = FOUR_FINGERS_SIDE_PINCH;
//			break;
//		case 0x30:
//			*grip = TWO_FINGERS_TOP_BOTTOM_PINCH;
//			break;
//		default:
//			break;
//		}
	}
	return touchUpdate;
}

PUBLIC bool bRobotUpdateManipulationState(StatusData *status)
{
	static bool display = true;
	status->transition = NO_TRANSITION;

	bool newGrip = bRobotUpdateGrip(&status->grip);
	vRobotUpdateIMUData(ROBOT_TICK_PERIOD_MS);
	bool newFall = bChangeInFreefall();
	bool newMovement = bChangeInMovement();

	switch (manipulationState.currentState)
	{
	case NO_CONTACT_STILL:
		if (display)
		{
			DBG_vPrintf(DEBUG_STATE_MACHINE, "NO_CONTACT_STILL");
			display = false;
		}
		if (newGrip)
		{
			display = true;
			DBG_vPrintf(DEBUG_STATE_MACHINE, "-TOUCH\n");
			manipulationState.previousState = manipulationState.currentState;
			manipulationState.currentState = CONTACT_STILL;
			status->transition = TOUCH;
		}
		else if (newFall)
		{

			display = true;
			DBG_vPrintf(DEBUG_STATE_MACHINE, "-FALL\n");
			manipulationState.previousState = manipulationState.currentState;
			manipulationState.currentState = NO_CONTACT_MOVING;
			status->transition = FALL;
		}
		break;
	case CONTACT_STILL:
		if (display)
		{
			DBG_vPrintf(DEBUG_STATE_MACHINE, "CONTACT_STILL");
			display = false;
		}
		if (newGrip)
		{
			manipulationState.previousState = manipulationState.currentState;
			if (status->grip == NO_GRIP)
			{
				display = true;
				DBG_vPrintf(DEBUG_STATE_MACHINE, "-RELEASE\n");
				manipulationState.currentState = NO_CONTACT_STILL;
				status->transition = RELEASE;
			}
			else
			{
				display = true;
				DBG_vPrintf(DEBUG_STATE_MACHINE, "-GRIP_CHANGED\n");
				status->transition = CONTACT_STILL_GRIP_CHANGED;
			}
		}
		else if (newMovement)
		{
			display = true;
			DBG_vPrintf(DEBUG_STATE_MACHINE, "-MOVE\n");
			manipulationState.previousState = manipulationState.currentState;
			manipulationState.currentState = CONTACT_MOVING;
			status->transition = MOVE;
		}
		break;

	case CONTACT_MOVING:
		if (display)
		{
			DBG_vPrintf(DEBUG_STATE_MACHINE, "CONTACT_MOVING");
			display = false;
		}
		if (newGrip)
		{
			manipulationState.previousState = manipulationState.currentState;
			if (status->grip == NO_GRIP)
			{
				display = true;
				DBG_vPrintf(DEBUG_STATE_MACHINE, "-THROW\n");
				manipulationState.currentState = NO_CONTACT_MOVING;
				status->transition = THROW;
			}
			else
			{
				display = true;
				DBG_vPrintf(DEBUG_STATE_MACHINE, "-CHANGED\n");
				status->transition = CONTACT_MOVING_GRIP_CHANGED;
			}
		}
		else if (newMovement)
		{
			display = true;
			DBG_vPrintf(DEBUG_STATE_MACHINE, "-STOP\n");
			manipulationState.previousState = manipulationState.currentState;
			manipulationState.currentState = CONTACT_STILL;
			status->transition = STOP;
		}
		break;
	case NO_CONTACT_MOVING:
		if (display)
		{
			DBG_vPrintf(DEBUG_STATE_MACHINE, "NO_CONTACT_MOVING");
			display = false;
		}
		if (newGrip)
		{
			display = true;
			DBG_vPrintf(DEBUG_STATE_MACHINE, "-HIT_CATCH\n");
			manipulationState.previousState = manipulationState.currentState;
			manipulationState.currentState = CONTACT_MOVING;
			status->transition = HIT_CATCH;
		}
		else if (motionHistory == 0)
		{
			display = true;
			DBG_vPrintf(DEBUG_STATE_MACHINE, "-HALT\n");
			manipulationState.previousState = manipulationState.currentState;
			manipulationState.currentState = NO_CONTACT_STILL;
			status->transition = HALT;
		}
		break;
	default:
		break;
	}

	if (status->transition == NO_TRANSITION)
		return false;
	else
		return true;
}

//#ifdef DBG_ENABLE
//	m.message_id = IMU_RAW_DATA;
//	m.message_size = sizeof(imuData);
//	m.address = 0;
//	m.message_id = ACCEL_RAW_DATA;
//	m.message_size = sizeof(acceleration);
//	m.address = 0;
//	memcpy(&(m.payload),&acceleration,sizeof(acceleration));
//	vSendUSB(&m);
//#endif

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
