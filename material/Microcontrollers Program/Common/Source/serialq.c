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
 * Maintains two circular queues:
 *
 * - The receive queue is filled with data received by the UART and is
 * emptied by the protocol transmitting the data over the radio.
 *
 * - The transmit queue is filled with data received by the protocol over
 * the radio and transmitted by the UART.
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

#include "uart.h"
#include "serialq.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define SERIALQ_MASK   0x03FFU			/**< Mask for indicies into queues */
#define SERIALQ_SIZE   SERIALQ_MASK+1	/**< Size of queues, (mask plus 1) */
#define SERIALQ_COUNT  		2			/**< Number of queues */
#define SERIALQ_FREE_LOW   64			/**< Low on free space level */
#define SERIALQ_FREE_HIGH 128			/**< High on free space level */

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/** Circular buffer */
typedef struct
{
    uint16 u16Head; 	/**< Position in queue to add to */
    uint16 u16Tail; 	/**< Position in queue to remove from */
    uint16 u16In;		/**< Input character counter */
    uint16 u16Out;		/**< Output character counter */
    uint8  u8Buff[SERIALQ_SIZE]; /**< Queue buffer */
} tsCircBuff;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE 	  tsCircBuff sRxQueue;						/**< Receive queue */
PRIVATE 	  tsCircBuff sTxQueue;					   	/**< Transmit queue */
PRIVATE const tsCircBuff *apsQueueList[SERIALQ_COUNT] 	/**  Array of queue pointers */
							= { &sRxQueue, &sTxQueue };

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
PRIVATE void vSerialQ_Flush(teQueueRef eQueue);

/****************************************************************************
 *
 * NAME: vSerialQ_Init
 *
 * DESCRIPTION  Initiliase the serial queues.
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
PUBLIC bool bSerialQ_Init(void)
{
    vSerialQ_Flush(RX_QUEUE);
    vSerialQ_Flush(TX_QUEUE);
    return TRUE;
}

/****************************************************************************
 *
 * NAME: vSerialQ_AddString
 *
 * DESCRIPTION  Add a string to a serial queue
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
PUBLIC void vSerialQ_AddString(teQueueRef  eQueue, 	/**< Queue to operate on. */
							   char 	 *psString)	/**< String to add to queue */
{
	while (! bSerialQ_Full(eQueue) && *psString != '\0')
	{
		vSerialQ_AddItem(eQueue, (uint8) *psString);
		psString++;
	}
}

/****************************************************************************
 *
 * NAME: vSerialQ_AddHex
 *
 * DESCRIPTION  Convert a number to a hex string and add to a serial queue.
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
PUBLIC void vSerialQ_AddHex(teQueueRef eQueue, 	/**< Queue to operate on. */
							uint16   u16Value,  /**< Value to convert to hex */
							uint8     u8Digits)	/**< Number of hex digits, maxumum 4 */
{
	uint8  u8Pos;
	char    sHex[5];
	char    sChar[17] = "0123456789ABCDEF";

	/* Sanity check digits */
	if (u8Digits > 4) u8Digits = 4;
	/* Do hex conversion */
	for (u8Pos = 0; u8Pos < u8Digits; u8Pos++)
	{
		sHex[u8Pos] = sChar[(u16Value >> ((u8Digits-u8Pos-1)*4)) &0xF];
		sHex[u8Pos+1] = '\0';
	}
	/* Write out string */
	vSerialQ_AddString(eQueue, sHex);
}

/****************************************************************************
 *
 * NAME: vSerialQ_AddItem
 *
 * DESCRIPTION  Add a byte to a serial queue
 *
 * If the receive queue and it begins to fill disable receive on the UART.
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
PUBLIC void vSerialQ_AddItem(teQueueRef eQueue, 	/**< Queue to operate on. */
							 uint8 	   u8Item)		/**< Byte to add to queue */
{
    tsCircBuff *psQueue;
    uint16 u16NextLocation;
    uint16 u16Tail;
    uint16 u16Free;

	/* Set pointer to the requested queue */
    psQueue = (tsCircBuff *)apsQueueList[eQueue];

	/* Get local copy of the tail - the remove function might use it */
	u16Tail = psQueue->u16Tail;

    u16NextLocation = (psQueue->u16Head + 1) & SERIALQ_MASK;

    if (u16NextLocation != u16Tail)
    {
        /* Space available in buffer so add data */
        psQueue->u8Buff[psQueue->u16Head] = u8Item;
        psQueue->u16Head = u16NextLocation;
		psQueue->u16In++;

		/* Receive queue ? */
		if (eQueue == RX_QUEUE)
		{
			/* Calculate the free characters */
			if (u16Tail > psQueue->u16Head) u16Free = u16Tail - psQueue->u16Head;
			else				            u16Free =  SERIALQ_SIZE + u16Tail - psQueue->u16Head;
			/* Did we just get low on free space ? */
			if (u16Free == SERIALQ_FREE_LOW)
			{
				/* If UART Rx currently enabled - disable UART Rx */
				if (bUART_GetRxEnable()) vUART_SetRxEnable(FALSE);
			}
		}
	}
}

/****************************************************************************
 *
 * NAME: u8SerialQ_RemoveItem
 *
 * DESCRIPTION  Remove a byte from a serial queue
 *
 * If the receive queue and it begins to empty enable receive on the UART.
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
PUBLIC uint8 u8SerialQ_RemoveItem(teQueueRef eQueue) /**< Queue to operate on. */
{
    uint8 u8Item = 0;
    tsCircBuff *psQueue;
    uint16 u16Head;
    uint16 u16Free;

    psQueue = (tsCircBuff *)apsQueueList[eQueue]; /* Set pointer to the requested queue */
	/* Get local copy of the head - the add function updates it */
	u16Head = psQueue->u16Head;

    if (psQueue->u16Tail != u16Head)
    {
        /* Data available on queue so remove a single item */
        u8Item = psQueue->u8Buff[psQueue->u16Tail];
        psQueue->u16Tail = (psQueue->u16Tail + 1) & SERIALQ_MASK;
        psQueue->u16Out++;

		/* Receive queue ? */
		if (eQueue == RX_QUEUE)
		{
			/* Calculate the free characters */
			if (psQueue->u16Tail > u16Head) u16Free = psQueue->u16Tail - u16Head;
			else				   			u16Free = SERIALQ_SIZE + psQueue->u16Tail - u16Head;
			/* Did we just get a high amount of free space ? */
			if (u16Free == SERIALQ_FREE_HIGH)
			{
				/* If UART Rx currently disabled - enable UART Rx */
				if (! bUART_GetRxEnable()) vUART_SetRxEnable(TRUE);
			}
		}
    }
    return(u8Item);
}

/****************************************************************************
 *
 * NAME: bSerialQ_Empty
 *
 * DESCRIPTION  Test for empty queue
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
PUBLIC bool_t bSerialQ_Empty(teQueueRef eQueue)	/**< Queue to operate on. */
{
    bool_t bResult = FALSE;
    tsCircBuff *psQueue;

    psQueue = (tsCircBuff *)apsQueueList[eQueue];

    if (psQueue->u16Tail == psQueue->u16Head)
    {
        bResult = TRUE;
    }
    return(bResult);	/** \retval TRUE  queue is empty
    						\retval FALSE queue has data */
}

/****************************************************************************
 *
 * NAME: bSerialQ_Full
 *
 * DESCRIPTION  Test for full queue
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
PUBLIC bool_t bSerialQ_Full(teQueueRef eQueue)	 /**< Queue to operate on. */
{
    bool_t bResult = FALSE;
    tsCircBuff *psQueue;
    uint16 u16NextLocation;

    psQueue = (tsCircBuff *)apsQueueList[eQueue];

    u16NextLocation = (psQueue->u16Head + 1) & SERIALQ_MASK;

    if (u16NextLocation == psQueue->u16Tail)
    {
	    bResult = TRUE;
    }
    return(bResult); /** \retval TRUE  queue is full
    					 \retval FALSE queue has space */
}

/****************************************************************************
 *
 * NAME: u16SerialQ_Count
 *
 * DESCRIPTION  Number of characters in queue.
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
PUBLIC uint16 u16SerialQ_Count(teQueueRef eQueue)  /**< Queue to operate on. */
{
    uint16 u16Head;
    uint16 u16Tail;
    tsCircBuff *psQueue = (tsCircBuff *)apsQueueList[eQueue];

	/* Get local tail and head values, we don't want them changing under us
	   while we calculate the count */
	u16Tail = psQueue->u16Tail;
	u16Head = psQueue->u16Head;

	/* Calculate the used characters into the head value */
	if (u16Head >= u16Tail) u16Head -= u16Tail;
	else					u16Head =  SERIALQ_SIZE + u16Head - u16Tail;

    return(u16Head); /** \return Number of characters in queue */
}

/****************************************************************************
 *
 * NAME: u16SerialQ_Free
 *
 * DESCRIPTION  Amount of free space in queue.
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
PUBLIC uint16 u16SerialQ_Free(teQueueRef eQueue)   /**< Queue to operate on. */
{
    uint16 u16Head;
    uint16 u16Free;
    tsCircBuff *psQueue = (tsCircBuff *)apsQueueList[eQueue];

	/* Get local tail and head values, we don't want them changing under us
	   while we calculate the count */
	u16Free = psQueue->u16Tail;
	u16Head = psQueue->u16Head;

	/* Calculate the used characters into the head value */
	if (u16Free > u16Head) u16Free -= u16Head;
	else				   u16Free =  SERIALQ_SIZE + u16Free - u16Head;

    return(u16Free); /** \return Amount of free space in queue */
}

/****************************************************************************
 *
 * NAME: u16SerialQ_GetInCount
 *
 * DESCRIPTION  Get number of characters added to queue
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
PUBLIC uint16 u16SerialQ_GetInCount(teQueueRef eQueue)	/**< Queue to operate on. */
{
    tsCircBuff *psQueue = (tsCircBuff *)apsQueueList[eQueue];

    return(psQueue->u16In);	/** \return Count of characters added to queue */
}

/****************************************************************************
 *
 * NAME: u16SerialQ_GetOutCount
 *
 * DESCRIPTION  Get number of characters removed from queue
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
PUBLIC uint16 u16SerialQ_GetOutCount(teQueueRef eQueue)	/**< Queue to operate on. */
{
    tsCircBuff *psQueue = (tsCircBuff *)apsQueueList[eQueue];

    return(psQueue->u16Out); /** \return Count of characters removed from queue */
}

/****************************************************************************
 *
 * NAME: vSerialQ_Flush
 *
 * DESCRIPTION  Flush queue so it is effectively empty.
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
PRIVATE void vSerialQ_Flush(teQueueRef eQueue)	/**< Queue to operate on. */
{
    tsCircBuff *psQueue;

    psQueue = (tsCircBuff *)apsQueueList[eQueue]; /* Set pointer to the requested queue */

    psQueue->u16Head     = 0;
    psQueue->u16Tail     = 0;
    psQueue->u16In       = 0;
    psQueue->u16Out      = 0;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
