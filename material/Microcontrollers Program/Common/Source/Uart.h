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
 */
/*\CHANGE HISTORY
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
#ifndef  UART_H_INCLUDED
#define  UART_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <string.h>
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/* Define which of the two available hardware UARTs and what baud rate to use */
#define UART                    E_AHI_UART_0			/**< UART to use */
#define UART_BAUD_RATE          921600					/**< Baud rate */


#if UART == E_AHI_UART_0
	#define UART_DEVICE         E_AHI_DEVICE_UART0		/**< UART device */
#else
	#define UART_DEVICE         E_AHI_DEVICE_UART1 		/**< UART device */
#endif

#ifdef ENABLE_HW_SW_FLOW_CONTROL
#define UART_AUTOFLOW			FALSE					/**< Define hardware flow control type. */
														/* FALSE app controls RTS/CTS, TRUE UART controls RTS/CTS */
#endif
/* else */
#ifdef ENABLE_HW_FLOW_CONTROL
#define UART_AUTOFLOW			TRUE					/**< Define hardware flow control type. */
														/* FALSE app controls RTS/CTS, TRUE UART controls RTS/CTS */
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct USBMessage
{
	uint8 message_id;
	uint8 address;
	uint8 message_size;
	uint8 payload[14];
}USBMessage;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC bool 	bUART_Init(void);
PUBLIC void     vUART_StartTx(void);
PUBLIC void     vUART_RxCharISR(uint8 u8RxChar);
PUBLIC void     vUART_TxCharISR(void);
PUBLIC void   	vUART_SetTxEnable(bool_t);
PUBLIC bool_t   bUART_GetTxEnable(void);
PUBLIC void   	vUART_SetRxEnable(bool_t);
PUBLIC bool_t   bUART_GetRxEnable(void);
PUBLIC void 	UART_vChar(char cChar);
PUBLIC void 	UART_vInit(void);
PUBLIC void 	vSendUSB(USBMessage* msg);
PUBLIC bool		vReadUSB(USBMessage* msg);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* UART_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
