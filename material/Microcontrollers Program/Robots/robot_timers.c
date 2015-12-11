#include "robot_timers.h"
#include <dbg.h>

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/


/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/


/****************************************************************************
 *
 * NAME: vTickTimerISR
 *
 * DESCRIPTION Timer interrupt service routine.
 *
 * Flash LED so we can see software is running.
 *
 * Check for data ready for transmission and initiate.
 *
 * PARAMETERS       Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 *
 *
 ****************************************************************************/
PUBLIC void vRobotInitTickTimer(void)
{
	DBG_vPrintf(DEBUG_DEVICE_FUNC,"vRobotInitTickTimer\n");
	/* Initialise tick timer to give 10ms interrupt */
	vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_DISABLE);
	vAHI_TickTimerWrite(0);
	vAHI_TickTimerInterval(ROBOT_TICK_PERIOD_COUNT);
	vAHI_TickTimerIntEnable(TRUE);
	vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_RESTART);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
