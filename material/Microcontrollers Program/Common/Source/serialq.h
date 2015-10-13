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
 *\DESCRIPTION         Circular queue for serial port use.
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
#ifndef  SERIALQ_H_INCLUDED
#define  SERIALQ_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <jendefs.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/* Queue references */
typedef enum
{
	RX_QUEUE = 0,	/**< Receive queue reference */
	TX_QUEUE		/**< Transmit queue reference */
}   teQueueRef;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC bool     bSerialQ_Init(void);
PUBLIC void     vSerialQ_AddItem(teQueueRef, uint8);
PUBLIC uint8   u8SerialQ_RemoveItem(teQueueRef);
PUBLIC bool_t   bSerialQ_Full(teQueueRef);
PUBLIC bool_t   bSerialQ_Empty(teQueueRef);
PUBLIC void     vSerialQ_AddString(teQueueRef, char *);
PUBLIC void 	vSerialQ_AddHex(teQueueRef, uint16, uint8);
PUBLIC uint16 u16SerialQ_Count(teQueueRef eQueue);
PUBLIC uint16 u16SerialQ_Free(teQueueRef eQueue);
PUBLIC uint16 u16SerialQ_GetInCount(teQueueRef eQueue);
PUBLIC uint16 u16SerialQ_GetOutCount(teQueueRef eQueue);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* SERIALQ_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
