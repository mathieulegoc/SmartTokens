#include "dongle_functions.h"
#include <dbg.h>
#include <dbg_uart.h>
#include "Timers.h"
#include "config.h"

/****************************************************************************
 *
 * NAME: vDongleInitDIOs
 *
 * DESCRIPTION: Initialize the DIOs pins
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void vDongleInitDIOs(){
	DBG_vPrintf(DEBUG_DEVICE_CONFIG,"vDongleInitDIOs\n");
	vAHI_DioSetPullup(0xFFFFFFFF, 0);
	vAHI_DioSetDirection(0, DONGLE_STATUS_LED); // set DIO15 to output

	vDongleSetLED(OFF);
}

/****************************************************************************
 *
 * NAME: vDongleSetLED
 *
 * DESCRIPTION: Turns on or off the LED
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void vDongleSetLED(bool state)
{
	if(state)
		vAHI_DioSetOutput(0,DONGLE_STATUS_LED);
	else
		vAHI_DioSetOutput(DONGLE_STATUS_LED, 0);
}

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
PUBLIC void vDongleInitTickTimer(void)
{
	DBG_vPrintf(DEBUG_DEVICE_CONFIG,"vDongleInitTickTimer\n");
	/* Initialise tick timer to give 10ms interrupt */
	vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_DISABLE);
	vAHI_TickTimerWrite(0);
	vAHI_TickTimerInterval(DONGLE_TICK_PERIOD_COUNT);
	vAHI_TickTimerIntEnable(TRUE);
	vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_RESTART);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
