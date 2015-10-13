#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <AppHardwareApi_JN5168.h>
#include "Config.h"
#include "lsm6ds0.h"
#include "qt1070.h"
#include <string.h>
#include "Uart.h"
#include <string.h>
#include "robot_timers.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define FREEWHEEL		0
#define FORWARD 		1
#define BACKWARD 		2
#define BRAKE 			3



/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

//PUBLIC uint8 touchedFaces[6];
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC void vRobotInitDIOs(void);
PUBLIC void vRobotInitSensors(void);
PUBLIC void measureGravity();
PUBLIC void vRobotUpdateIMUData(uint8 msTimestep);
PUBLIC void vMonitorMovement();
PUBLIC bool bChangeInFreefall();
PUBLIC bool bChangeInMovement();
PUBLIC bool bRobotgetTouchedKeys(uint8* touchedKeys);
PUBLIC bool bRobotUpdateGrip(Grip* grip);
PUBLIC bool bRobotUpdateManipulationState(StatusData *status);

#endif /* FUNCTIONS_H_ */
