/*============================================================================
Include files
============================================================================*/
#include "Timers.h"
#include <dbg.h>
#include <dbg_uart.h>


/*============================================================================
Global variable definitions
============================================================================*/

/*============================================================================
Function Definitions
============================================================================*/

/*============================================================================
Name    :   vWaitMilliseconds
------------------------------------------------------------------------------
Purpose :	wait the given amount of milliseconds
Input   :   milliseconds
Output  :   none
Return	:	none
Notes   :
============================================================================*/
PUBLIC void vWaitMilliseconds(uint16 milliseconds)
{
	uint8 prescaler;
	uint32 period;

	if(milliseconds){
		if(milliseconds>1000)
			prescaler = 11;
		else if(milliseconds<100)
			prescaler = 6;
		else
			prescaler = 9;

		period = (32000*milliseconds)/(2<<prescaler);

		if(period>0xFFFF)
			period=0xFFFF;

		vAHI_TimerEnable(E_AHI_TIMER_0,prescaler,FALSE,FALSE,FALSE);
		vAHI_TimerConfigureOutputs(E_AHI_TIMER_0,FALSE,TRUE);
		vAHI_TimerStartSingleShot(E_AHI_TIMER_0,period,period);
		while(u16AHI_TimerReadCount(E_AHI_TIMER_0)<=period-2);

	}
}

/*============================================================================
Name    :   vWaitMicroseconds
------------------------------------------------------------------------------
Purpose :	wait the given amount of milliseconds
Input   :   microseconds
Output  :   none
Return	:	none
Notes   :
============================================================================*/
PUBLIC void vWaitMicroseconds(uint16 microseconds)
{
	uint8 prescaler;
	uint32 period;

	if(microseconds){
		if(microseconds>2000)
			prescaler = 2;
		else
			prescaler = 1;

		period = (32*microseconds)/(2<<prescaler);

		if(period>0xFFFF)
			period=0xFFFF;

		vAHI_TimerEnable(E_AHI_TIMER_0,prescaler,FALSE,FALSE,FALSE);
		vAHI_TimerConfigureOutputs(E_AHI_TIMER_0,FALSE,TRUE);
		vAHI_TimerStartSingleShot(E_AHI_TIMER_0,period,period);
		while(u16AHI_TimerReadCount(E_AHI_TIMER_0)<=period-2);
	}
}
