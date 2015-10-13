/***************************************************************************/
/*!
 *\MODULE
 *
 *\COMPONENT
 *
 *\VERSION
 *
 *\REVISION
 *
 *\DATED
 *
 *\STATUS
 *
 *\AUTHOR
 *
 *\DESCRIPTION         Controls transmission and reception of data on the UART.
 *
 * Operates the UART using an interrupt handler that is called when
 * characters are received or are ready to transmit:
 *
 * - Data received by the UART is added to the receive queue.
 *
 * - Data for transmission by the UART is removed from the transmit queue
 * and transmitted by the UART.
 *
 * Hardware flow control using RTS and CTS is implemented in one of two ways:
 *
 * \par Manual Hardware Flow Control:
 * is used when UART_AUTOFLOW is defined to FALSE.
 * In this mode the RTS line is lowered by software as the receive queue begins to
 * fill and raised as the the queue begins to empty. An interrupt is programmed to
 * occur when the CTS line is changed, the state of the CTS line is read and
 * the transmission of data is allowed or denied dependent upon the state of the
 * CTS line.
 *
 * \par Automatic Hardware Flow Control:
 * can be enabled in the UART by setting
 * the UART_AUTOFLOW define to TRUE. In this mode when the 16 byte receive
 * FIFO begins to receive the 16th byte the UART's RTS line is lowered and
 * raised once space is available in the FIFO again . When the UART's CTS
 * line is low the UART will not transmit any data from the transmit FIFO.
 *
 * \attention The FTDI 3V Serial to USB cables supplied with the evaluation kits
 * and their drivers do not react quickly enough to the RTS being lowered which
 * results in an extra character being sent to the UART, which is then lost. When using
 * two UARTs with automatic flow control they should react quickly enough to prevent
 * such data loss. Manual Hardware Flow Control is therefore recommended when using the
 * evaluation kits and FTDI cables.
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5168, JN5148, JN5139].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2014. All rights reserved
 *
 ****************************************************************************/
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <AppHardwareApi.h>
#include <PeripheralRegs_JN5168.h>
#include <AppHardwareApi_JN5168.h>
#include <stdlib.h>
#include "serialq.h"
#include "Uart.h"
#include <dbg.h>
#include <dbg_uart.h>
#include "serialq.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#if UART == E_AHI_UART_0
#define UART_START_ADR  					0x02003000UL /**< Address of start of UART's registers */
#else
#define UART_START_ADR  					0x02004000UL /**< Address of start of UART's registers */
#endif
#define UART_DLM_OFFSET 						0x04		 /**< Offset of UART's DLM register */
#define UART_LCR_OFFSET 						0x0C		 /**< Offset of UART's LCR register */
#define UART_MCR_OFFSET 						0x10		 /**< Offset of UART's MCR register */
#define UART_EFR_OFFSET							0x20		 /**< Offset of UART's EFR register */
/* Define for devices with AFC */
#define UART_AFC_OFFSET							0x2C		 /**< Offset of UART's AFC register */
/* Enable fine grained baud rate selection */
#define UART_ENABLE_ADVANCED_BAUD_SELECTION		TRUE
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE bool_t bRxEnable; /**< UART receive enabled */
PRIVATE bool_t bTxEnable; /**< UART transmit enabled */
PRIVATE bool_t bModemInt; /**< UART modem status change interrupt enabled */
PRIVATE bool_t bTxIntServiced; /**< LAst UART transmit interrupt was serviced, expect another transmit interrupt to follow */

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
#if ENABLE_HW_FLOW_CONTROL
PRIVATE void vUART_SetRts(bool_t);
PRIVATE void vUART_SetAutoFlow(bool_t);
#endif

PRIVATE void vUART_SetBaudRate(uint8 u8Uart, uint32 u32BaudRate);
PRIVATE void vUART_HandleUartInterrupt(uint32 u32Device, uint32 u32ItemBitmap);
/****************************************************************************
 *
 * NAME: vUART_Init
 *
 * DESCRIPTION  Initialise UART.
 *
 * Sets baud rate, interrupt handling and flow control for the UART.
 *
 * PARAMETERS       Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC bool bUART_Init(void) {
	/* Start with Receive and transmit enabled */
	bRxEnable = TRUE;
	bTxEnable = TRUE;

	/* Modem status change interrupt is dependent upon flow control mode */
#ifdef ENABLE_HW_FLOW_CONTROL
	bModemInt = TRUE;
#endif

#ifdef ENABLE_HW_SW_FLOW_CONTROL
	bModemInt = FALSE;
#endif

	/* Note we are not currently servicing transmit interrupts */
	bTxIntServiced = FALSE;

	/* Enable UART 0 */
#ifdef ENABLE_NO_FLOW_CONTROL
	vAHI_UartSetRTSCTS(UART, FALSE);
#endif

	vAHI_UartEnable(UART);

	/* Reset UART */
	vAHI_UartReset(UART, TRUE, TRUE);
	vAHI_UartReset(UART, FALSE, FALSE);

	/* Register function that will handle UART interrupts */
#if UART == E_AHI_UART_0
	vAHI_Uart0RegisterCallback(vUART_HandleUartInterrupt);
#else
	vAHI_Uart1RegisterCallback(vUART_HandleUartInterrupt);
#endif

#if ENABLE_HW_FLOW_CONTROL
	/* Allow use of RTS/CTS pins with UART */
	vAHI_UartSetRTSCTS(UART, TRUE);
	/* Set automatic flow control */
	vUART_SetAutoFlow(UART_AUTOFLOW);
#endif

	/* Set the clock divisor register to give required baud, this has to be done
	 directly as the normal routines (in ROM) do not support all baud rates */
	vUART_SetBaudRate(UART, UART_BAUD_RATE);

	/* Set remaining settings */
	vAHI_UartSetControl(UART, FALSE, FALSE, E_AHI_UART_WORD_LEN_8, TRUE, FALSE);

#if ENABLE_HW_FLOW_CONTROL
	/* Turn on RTS */
	vUART_SetRts(bRxEnable);

	/* Is CTS bit set meaning CTS is off ? */
	if (u8AHI_UartReadModemStatus(UART) & 0x10)
	{
		/* Disable transmit */
		vUART_SetTxEnable(FALSE);
	}
	/* Is CTS bit is clear meaning CTS is on ? */
	else
	{
		/* Enable transmit */
		vUART_SetTxEnable(TRUE);
	}
#endif

	/* Turn on modem status, tx, rx interrupts */
	vAHI_UartSetInterrupt(UART, bModemInt, FALSE, TRUE, bRxEnable,E_AHI_UART_FIFO_LEVEL_1);
	return TRUE;
}

/****************************************************************************
 *
 * NAME: vUART_StartTx
 *
 * DESCRIPTION  Start transmitting from UART.
 *
 * If we are not currently servicing transmit interrupts and we have data
 * to transmit, begin transmission by writing the initial character to
 * the transmit buffer.
 *
 * PARAMETERS       Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC void vUART_StartTx(void) {
	/* Did we not service the last tx interrupt ? */
	if (bTxIntServiced == FALSE) {
		/* Has interrupt driven transmit stalled (tx fifo is empty) */
		if (u8AHI_UartReadLineStatus(UART) & E_AHI_UART_LS_THRE) {
			if (!bSerialQ_Empty(TX_QUEUE) && bTxEnable) {
				vAHI_UartWriteData(UART, u8SerialQ_RemoveItem(TX_QUEUE));
			}
		}
	}
}

/****************************************************************************
 *
 * NAME: vUART_TxCharISR
 *
 * DESCRIPTION  Transmit character interrupt service routine.
 *
 * PARAMETERS       Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC void vUART_TxCharISR(void) {
	if (!bSerialQ_Empty(TX_QUEUE) && bTxEnable) {
		vAHI_UartWriteData(UART, u8SerialQ_RemoveItem(TX_QUEUE));
		/* Note we serviced the interrupt, tx interrupts will continue */
		bTxIntServiced = TRUE;
	} else {
		/* Note we didn't service the interrupt, tx interrupts will now stop */
		bTxIntServiced = FALSE;
	}
}

/****************************************************************************
 *
 * NAME: vUART_RxCharISR
 *
 * DESCRIPTION  Receive character interrupt service routeine.
 *
 * PARAMETERS       Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC void vUART_RxCharISR(uint8 u8RxChar) /**< Received character */
{
	vSerialQ_AddItem(RX_QUEUE, u8RxChar);
}

/****************************************************************************
 *
 * NAME: vUART_SetRxEnable
 *
 * DESCRIPTION  Set receive enabled status.
 *
 * PARAMETERS       Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC void vUART_SetRxEnable(bool_t bEnable) /**< Receive interrupt status */
{
	/* Changing setting ? */
	if (bRxEnable != bEnable) {
		/* Set new value */
		bRxEnable = bEnable;

		/* Update Rx interrupt setting */
		vAHI_UartSetInterrupt(UART, bModemInt, FALSE, TRUE, bRxEnable,
				E_AHI_UART_FIFO_LEVEL_1);

		/* Not using automatic flow control ? */
#ifdef ENABLE_HW_SW_FLOW_CONTROL
		/* Set RTS manually */
		vUART_SetRts(bRxEnable);
#endif
	}
}

/****************************************************************************
 *
 * NAME: bUART_GetRxEnable
 *
 * DESCRIPTION  Get receive enabled status.
 *
 * PARAMETERS       Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC bool_t bUART_GetRxEnable(void) {
	return bRxEnable;
}

/****************************************************************************
 *
 * NAME: vUART_SetTxEnable
 *
 * DESCRIPTION  Set transmit enabled status.
 *
 * PARAMETERS       Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC void vUART_SetTxEnable(bool_t bEnable) /**< Transmit enabled status */
{
	/* Changing setting ? */
	if (bTxEnable != bEnable) {
		/* Set new value */
		bTxEnable = bEnable;
	}
}

/****************************************************************************
 *
 * NAME: bUART_GetTxEnable
 *
 * DESCRIPTION  Get transmit enabled status.
 *
 * PARAMETERS       Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC bool_t bUART_GetTxEnable(void) {
	return bTxEnable;
}

/****************************************************************************
 *
 * NAME: vUART_SetBaudRate
 *
 * DESCRIPTION  Set baud rate for UART.
 *
 * Directly using registers.
 *
 * PARAMETERS       Name            RW  Usage
 *
 * RETURNS:
 *
 ****************************************************************************/
PRIVATE void vUART_SetBaudRate(uint8 u8Uart, uint32 u32BaudRate) {
	uint16 u16Divisor;
	uint32 u32Remainder;
	uint8 u8ClocksPerBit = 16;

#if (defined UART_ENABLE_ADVANCED_BAUD_SELECTION)
	/* Defining ENABLE_ADVANCED_BAUD_SELECTION in the Makefile
	 * enables this code which searches for a clocks per bit setting
	 * that gets closest to the configured rate.
	 */
	uint32 u32CalcBaudRate = 0;
	int32 i32BaudError = 0x7FFFFFFF;

	while (abs(i32BaudError) > (int32) (u32BaudRate >> 4)) /* 6.25% (100/16) error */
	{
		if (--u8ClocksPerBit < 3) {
			return;
		}
#endif /* UART_ENABLE_ADVANCED_BAUD_SELECTION */

		/* Calculate Divisor register = 16MHz / (16 x baud rate) */
		u16Divisor = (uint16) (16000000UL
				/ ((u8ClocksPerBit + 1) * u32BaudRate));

		/* Correct for rounding errors */
		u32Remainder = (uint32) (16000000UL % ((u8ClocksPerBit + 1)
				* u32BaudRate));

		if (u32Remainder >= (((u8ClocksPerBit + 1) * u32BaudRate) / 2)) {
			u16Divisor += 1;
		}

#if (defined UART_ENABLE_ADVANCED_BAUD_SELECTION)

		u32CalcBaudRate = (16000000UL / ((u8ClocksPerBit + 1) * u16Divisor));
		i32BaudError = (int32) u32CalcBaudRate - (int32) u32BaudRate;
	}

	/* Set the calculated clocks per bit */
	vAHI_UartSetClocksPerBit(u8Uart, u8ClocksPerBit);
#endif /* UART_ENABLE_ADVANCED_BAUD_SELECTION */

	/* Set the calculated divisor */
	vAHI_UartSetBaudDivisor(u8Uart, u16Divisor);
}

/****************************************************************************
 *
 * NAME: vUART_HandleUartInterrupt
 *
 * DESCRIPTION  Handles UART interrupt.
 *
 * Adds received character to receive serial queue, if available.
 *
 * Transmits character from transmit serial queue, if available.
 *
 * Reacts to change in CTS status when not using automatic flow control.
 *
 * PARAMETERS       Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vUART_HandleUartInterrupt(uint32 u32Device, /**< Interrupting device */
uint32 u32ItemBitmap) /**< Interrupt bitmask */
{
	if (u32Device == UART_DEVICE) {
#ifdef ENABLE_HW_SW_FLOW_CONTROL
		/* Read line status */
		uint8 u8LineStatus = u8AHI_UartReadLineStatus(UART);
#endif

		/* Data to receive ? */
		if ((u32ItemBitmap & 0x000000FF) == E_AHI_UART_INT_RXDATA) {
			/* Receive character from UART */
			/*
			 * !!!!! WHERE INCOMING DATA IS PUT IN THE QUEUE !!!!!
			 */
			vUART_RxCharISR(u8AHI_UartReadData(UART));
		}
#ifdef ENABLE_HW_SW_FLOW_CONTROL
		/* Modem status changed ? */
		else if (u32ItemBitmap == E_AHI_UART_INT_MODEM)
		{
			/* Read modem status */
			uint8 u8ModemStatus = u8AHI_UartReadModemStatus(UART);
			/* Has CTS changed ? */
			if (u8ModemStatus & E_AHI_UART_MS_DCTS)
			{
				/* Is CTS bit set meaning CTS has just been cleared ? */
				if (u8ModemStatus & 0x10)
				{
					/* Disable transmit */
					vUART_SetTxEnable(FALSE);
				}
				/* Is CTS bit is clear meaning CTS has just been set ? */
				else
				{
					/* Enable transmit */
					vUART_SetTxEnable(TRUE);
					/* OK to transmit now - begin transmitting again */
					if (u8LineStatus & E_AHI_UART_LS_THRE) vUART_TxCharISR();
				}
			}
		}
#endif
		/* Ready to transmit ? */
		else if (u32ItemBitmap == E_AHI_UART_INT_TX) {
			vUART_TxCharISR();
		}
	}
}

#if ENABLE_HW_FLOW_CONTROL
/****************************************************************************
 *
 * NAME: vUART_SetRts
 *
 * DESCRIPTION  Set RTS pin.
 *
 * Directly using registers.
 *
 * PARAMETERS       Name            RW  Usage
 *
 * RETURNS:
 *
 ****************************************************************************/
PRIVATE void vUART_SetRts(bool_t bEnable) /**< RTS status */
{
	uint8 *pu8Reg;
	uint8 u8Val;

	/* Get offset to Modem Control Register */
	pu8Reg = (uint8 *)(UART_START_ADR + UART_MCR_OFFSET);
	/* Get content of MCR */
	u8Val = *pu8Reg;
	/* Enabling ? */
	if (bEnable)
	{
		/* Clear bit to set RTS */
		u8Val &= 0xFD;
	}
	else
	{
		/* Set bit to clear RTS */
		u8Val |= 0x02;
	}
	/* Write new value back to register */
	*pu8Reg = u8Val;
}

/****************************************************************************
 *
 * NAME: vUART_SetAutoFlow
 *
 * DESCRIPTION  Set automatic flow control.
 *
 * Directly using registers.
 *
 * PARAMETERS       Name            RW  Usage
 *
 * RETURNS:
 *
 ****************************************************************************/
PRIVATE void vUART_SetAutoFlow(bool_t bEnable) /**< Automatic flow control enabled */
{
	uint8 *pu8Reg;
	uint8 u8Val;

	/* Get offset to Modem Control Register */
	pu8Reg = (uint8 *)(UART_START_ADR + UART_AFC_OFFSET);
	/* Get content of MCR */
	u8Val = *pu8Reg;

	if (bEnable == TRUE)
	{
		/* Set automatic flow control */
		u8Val |= 0x13;
	}
	else
	{
		/* Clear automatic flow control */
		u8Val = 0x0;
	}

	/* Write new value back to register */
	*pu8Reg = u8Val;
}
#endif

PUBLIC void UART_vChar(char cChar)
{
#if	UART_TRACE_ENABLE
	/* UART is open ? */
	if ((u32REG_SysRead(REG_SYS_PWR_CTRL) & REG_SYSCTRL_PWRCTRL_UEN0_MASK) == REG_SYSCTRL_PWRCTRL_UEN0_MASK)
	{
		/* Write character */
		vAHI_UartWriteData(E_AHI_UART_0, cChar);
		/* Wait for buffers to empty */
		while ((u8AHI_UartReadLineStatus(E_AHI_UART_0) & E_AHI_UART_LS_THRE) == 0);
		while ((u8AHI_UartReadLineStatus(E_AHI_UART_0) & E_AHI_UART_LS_TEMT) == 0);
	}
#endif
}

PUBLIC void UART_vInit(void)
{
	/* UART0 not already enabled ? */
	if ((u32REG_SysRead(REG_SYS_PWR_CTRL) & REG_SYSCTRL_PWRCTRL_UEN0_MASK) == 0)
	{
		/* Disable use of CTS/RTS */
		vAHI_UartSetRTSCTS(E_AHI_UART_0, FALSE);
		/* Enable UART */
		vAHI_UartEnable(E_AHI_UART_0);
		/* Set settings */
		vAHI_UartSetControl(E_AHI_UART_0, E_AHI_UART_EVEN_PARITY, E_AHI_UART_PARITY_DISABLE, E_AHI_UART_WORD_LEN_8, E_AHI_UART_1_STOP_BIT, E_AHI_UART_RTS_HIGH);
		/* Set baud rate */
		//vAHI_UartSetClockDivisor(E_AHI_UART_0, E_AHI_UART_RATE_115200);
		/* Increase to 921600 baud */
		vAHI_UartSetBaudDivisor(E_AHI_UART_0, 2);
		vAHI_UartSetClocksPerBit(E_AHI_UART_0, 8);
	}
}

/****************************************************************************
 *
 * NAME: vSendUSB
 *
 * DESCRIPTION: Sends the data in parameter over USB
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vSendUSB(USBMessage* msg) {
	unsigned int counter = 0;
	if (msg != NULL)
	{
		if (msg->message_size) {
			vSerialQ_AddItem(TX_QUEUE,'~');
			vSerialQ_AddItem(TX_QUEUE,msg->message_id);
			vSerialQ_AddItem(TX_QUEUE,msg->address);
			vSerialQ_AddItem(TX_QUEUE,msg->message_size);
			while (counter < msg->message_size) {
				vSerialQ_AddItem(TX_QUEUE, msg->payload[counter]);
				counter++;
			}
			vSerialQ_AddItem(TX_QUEUE,'!');
		}
		vUART_StartTx();
	}
}

/****************************************************************************
 *
 * NAME: vSendUSB
 *
 * DESCRIPTION: Reads data coming from USB
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC bool	vReadUSB(USBMessage* msg)
{
	unsigned char i=0;
	if(u16SerialQ_Count(RX_QUEUE)>=5)
	{
		if(u8SerialQ_RemoveItem(RX_QUEUE) == '~')
		{
			msg->message_id = u8SerialQ_RemoveItem(RX_QUEUE);
			msg->address = u8SerialQ_RemoveItem(RX_QUEUE);
			msg->message_size = u8SerialQ_RemoveItem(RX_QUEUE);
			for(i=0;i<msg->message_size;i++){
				msg->payload[i] = u8SerialQ_RemoveItem(RX_QUEUE);
			}
			if(u8SerialQ_RemoveItem(RX_QUEUE) == '!')
				return TRUE;
		}
	}
	return FALSE;
}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
