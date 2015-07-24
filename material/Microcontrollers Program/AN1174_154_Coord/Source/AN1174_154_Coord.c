/****************************************************************************
 *
 * MODULE:             coordinator.c
 *
 * COMPONENT:          coordinator.c,v
 *
 * VERSION:
 *
 * REVISION:
 *
 * DATED:
 *
 * STATUS:             Exp
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5148, JN5142, JN5139].
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

 * Copyright NXP B.V. 2012. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <AppHardwareApi.h>
#include <AppQueueApi.h>
#include <mac_sap.h>
#include <mac_pib.h>
#include <string.h>
#include "Timers.h"
#include <dbg.h>
#include <dbg_uart.h>
#include "Uart.h"
#include "serialq.h"
#include "config.h"
#include "dongle_functions.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
	E_STATE_IDLE, E_STATE_ENERGY_SCANNING, E_STATE_COORDINATOR_STARTED,
} teState;

/* Data type for storing data related to all end devices that have associated */
typedef struct
{
	uint16 u16ShortAdr;
	uint32 u32ExtAdrL;
	uint32 u32ExtAdrH;
} tsEndDeviceData;

typedef struct
{
	/* Data related to associated end devices */
	uint16 u16NbrEndDevices;
	tsEndDeviceData sEndDeviceData[MAX_END_DEVICES];

	teState eState;

	uint8 u8Channel;
	uint8 u8TxPacketSeqNb;
	uint8 u8RxPacketSeqNb;

} tsCoordinatorData;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vInitSystem(void);
PRIVATE void vStartEnergyScan(void);
PRIVATE void vStartCoordinator(void);
PRIVATE void vProcessEventQueues(void);
PRIVATE void vProcessIncomingMlme(MAC_MlmeDcfmInd_s *psMlmeInd);
PRIVATE void vProcessIncomingMcps(MAC_McpsDcfmInd_s *psMcpsInd);
PRIVATE void vProcessIncomingHwEvent(AppQApiHwInd_s *psAHI_Ind);
PRIVATE void vHandleNodeAssociation(MAC_MlmeDcfmInd_s *psMlmeInd);
PRIVATE void vHandleEnergyScanResponse(MAC_MlmeDcfmInd_s *psMlmeInd);
PRIVATE void vHandleMcpsDataInd(MAC_McpsDcfmInd_s *psMcpsInd);
PRIVATE void vHandleMcpsDataDcfm(MAC_McpsDcfmInd_s *psMcpsInd);
PRIVATE void vTransmitDataPacket(uint8 *pu8Data, uint8 u8Len, uint16 u16DestAdr);
PRIVATE void vProcessReceivedDataPacket(uint8 *pu8Data, uint8 u8Len, uint16 sourceAddr);
PRIVATE void vMain(void);
PRIVATE void vSendMessageToTangible(uint8 dest, uint8 id, uint8 size, uint8* payload);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
/* Handles from the MAC */
PRIVATE void *s_pvMac;
PRIVATE MAC_Pib_s *s_psMacPib;
PRIVATE tsCoordinatorData sCoordinatorData;
PRIVATE bool isUARTOpened;
PRIVATE bool dataForUSB;
PRIVATE USBMessage outgoingUSBMessage, incomingUSBMessage;
PUBLIC uint8 tmpData[100] =
{ 0 };
PRIVATE uint64 timestamp = 0;
PRIVATE bool measureTime = TRUE;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: AppColdStart
 *
 * DESCRIPTION:
 * Entry point for application from boot loader. Initialises system and runs
 * main loop.
 *
 * RETURNS:
 * Never returns.
 *
 ****************************************************************************/
PUBLIC void AppColdStart(void)
{
	/* Disable watchdog if enabled by default */
#ifdef WATCHDOG_ENABLED
	vAHI_WatchdogStop();
#endif

	vInitSystem();

	vStartEnergyScan();

	vMain();
}

/****************************************************************************
 *
 * NAME: AppWarmStart
 *
 * DESCRIPTION:
 * Entry point for application from boot loader. Simply jumps to AppColdStart
 * as, in this instance, application will never warm start.
 *
 * RETURNS:
 * Never returns.
 *
 ****************************************************************************/
PUBLIC void AppWarmStart(void)
{
	DBG_vPrintf(DEBUG_DEVICE_FUNC, "AppWarmStart\n");
	AppColdStart();
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vInitSystem
 *
 * DESCRIPTION:
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vInitSystem(void)
{
	/* Setup interface to MAC */
	(void) u32AHI_Init();
	(void) u32AppQApiInit(NULL, NULL, NULL);

	/* Initialise coordinator state */
	sCoordinatorData.eState = E_STATE_IDLE;
	sCoordinatorData.u8TxPacketSeqNb = 0;
	sCoordinatorData.u8RxPacketSeqNb = 0;
	sCoordinatorData.u16NbrEndDevices = 0;

	/* Set up the MAC handles. Must be called AFTER u32AppQApiInit() */
	s_pvMac = pvAppApiGetMacHandle();
	s_psMacPib = MAC_psPibGetHandle(s_pvMac);

	/* Set Pan ID and short address in PIB (also sets match registers in hardware) */
	MAC_vPibSetPanId(s_pvMac, PAN_ID);
	MAC_vPibSetShortAddr(s_pvMac, COORDINATOR_ADR);

	/* Enable receiver to be on when idle */
	MAC_vPibSetRxOnWhenIdle(s_pvMac, TRUE, FALSE);

	/* Allow nodes to associate */
	s_psMacPib->bAssociationPermit = 1;

	/* Wait for clock to stablise */
	while (bAHI_Clock32MHzStable() == FALSE)
		;

	/* Initialise UART */
#ifdef DBG_ENABLE
	DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);
#endif
	isUARTOpened = bSerialQ_Init() & bUART_Init();

	DBG_vPrintf(DEBUG_DEVICE_FUNC, "DONGLE\n");
	DBG_vPrintf(DEBUG_DEVICE_FUNC, "AppColdStart\n");
	DBG_vPrintf(DEBUG_DEVICE_FUNC, "Device_vInit\n");

	dataForUSB = false;

	vDongleInitDIOs();
	vDongleInitTickTimer();
}

/****************************************************************************
 *
 * NAME: vMain
 *
 * DESCRIPTION:
 * Main looping function, main code must be executed here
 *
 * RETURNS:
 * Never returns.
 *
 ****************************************************************************/
PRIVATE void vMain(void)
{
	while (1)
	{
		vProcessEventQueues();

		if (isUARTOpened)
		{
			if (dataForUSB)
			{
				vSendUSB(&outgoingUSBMessage);
				dataForUSB = false;
			}

			if (vReadUSB(&incomingUSBMessage))
			{
				vSendMessageToTangible(incomingUSBMessage.address, incomingUSBMessage.message_id,
						incomingUSBMessage.message_size, incomingUSBMessage.payload);
			}
		}
	}
}

/****************************************************************************
 *
 * NAME: vProcessReceivedDataPacket
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 *
 * RETURNS:
 *
 * NOTES:
 ****************************************************************************/
PRIVATE void vProcessReceivedDataPacket(uint8 *pu8Data, uint8 u8Len, uint16 sourceAddr)
{
	Message *tmpMsg = (Message*) pu8Data;
	uint64 deviceTime = 0;

	switch (tmpMsg->message_id)
	{
	case ASK_FOR_TIMESTAMP:
		vSendMessageToTangible((uint8) sourceAddr, SET_TIMESTAMP, sizeof(timestamp), (uint8*) &timestamp);
		measureTime = TRUE;
		break;
	case TIME_DATA:
		memcpy(&deviceTime, &(tmpMsg->payload), tmpMsg->message_size);
		DBG_vPrintf(true,"%d-%d\n",(uint32)deviceTime,(uint32)timestamp);
		break;
	default:
		outgoingUSBMessage.message_id = tmpMsg->message_id;
		outgoingUSBMessage.message_size = tmpMsg->message_size;
		outgoingUSBMessage.address = (uint8) sourceAddr;
		memcpy(&outgoingUSBMessage.payload, &(tmpMsg->payload), tmpMsg->message_size);
		dataForUSB=true;
//		if (outgoingUSBMessage.message_id == STATUS_DATA)
//		{
//			DBG_vPrintf(true, "%d status: T = ", sourceAddr);
//			switch (outgoingUSBMessage.payload[0])
//			{
//			case FALL:
//				DBG_vPrintf(true, "FALL - G = ");
//				break;
//			case TOUCH:
//				DBG_vPrintf(true, "TOUCH - G = ");
//				break;
//			case RELEASE:
//				DBG_vPrintf(true, "RELEASE - G = ");
//				break;
//			case CONTACT_STILL_GRIP_CHANGED:
//				DBG_vPrintf(true, "CONTACT STILL GRIP CHANGED - G = ");
//				break;
//			case MOVE:
//				DBG_vPrintf(true, "MOVE - G = ");
//				break;
//			case STOP:
//				DBG_vPrintf(true, "STOP - G = ");
//				break;
//			case CONTACT_MOVING_GRIP_CHANGED:
//				DBG_vPrintf(true, "CONTACT MOVING GRIP CHANGED - G = ");
//				break;
//			case THROW:
//				DBG_vPrintf(true, "THROW - G = ");
//				break;
//			case HALT:
//				DBG_vPrintf(true, "HALT - G = ");
//				break;
//			case HIT_CATCH:
//				DBG_vPrintf(true, "CATCH - G = ");
//				break;
//			default:
//				break;
//			}
//
//			DBG_vPrintf(true, "%d\n", outgoingUSBMessage.payload[1]);
//		}
		break;
	}
}

/****************************************************************************
 *
 * NAME: vProcessIncomingHwEvent
 *
 * DESCRIPTION:
 * Process any hardware events.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  psAHI_Ind
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vProcessIncomingHwEvent(AppQApiHwInd_s *psAHI_Ind)
{
	static uint16 x = 0;
	uint8 quantityToSend = 0;
	uint16 addressToAsk;

	switch (psAHI_Ind->u32DeviceId)
	{
	case E_AHI_DEVICE_TICK_TIMER:
		if (sCoordinatorData.u16NbrEndDevices == 0)
			vDongleSetLED(ON);
		else
			vDongleSetLED(OFF);
		if (measureTime)
			timestamp += DONGLE_TICK_PERIOD_MS;

		if (sCoordinatorData.u16NbrEndDevices == 0)
			vDongleSetLED(OFF);
		else
			vDongleSetLED(ON);

		break;
	default:
		break;
	}
	x++;
}

/****************************************************************************
 *
 * NAME: vTransmitDataPacket
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vSendMessageToTangible(uint8 dest, uint8 id, uint8 size, uint8* payload)
{
	tmpData[0] = id;

	if (size % 2 == 0)
	{
		tmpData[1] = size;
		if (size > 0)
			memcpy(&tmpData[2], payload, size);
		vTransmitDataPacket((uint8*) &tmpData[0], size + 2, (uint16) dest);
	}
	else
	{
		tmpData[1] = size + 1;
		memcpy(&tmpData[2], payload, size);
		tmpData[2 + size] = 0;
		vTransmitDataPacket((uint8*) &tmpData[0], size + 3, (uint16) dest);
	}
}

/****************************************************************************
 *
 * NAME: vHandleNodeAssociation
 *
 * DESCRIPTION:
 * Handle request by node to join the network.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  psMlmeInd
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vHandleNodeAssociation(MAC_MlmeDcfmInd_s *psMlmeInd)
{
	uint16 u16ShortAdr = 0xffff;
	uint16 u16EndDeviceIndex;

	MAC_MlmeReqRsp_s sMlmeReqRsp;
	MAC_MlmeSyncCfm_s sMlmeSyncCfm;

	DBG_vPrintf(DEBUG_DEVICE_FUNC, "vHandleNodeAssociation\n");

	if (sCoordinatorData.u16NbrEndDevices < MAX_END_DEVICES)
	{
		/* Store end device address data */
		u16EndDeviceIndex = sCoordinatorData.u16NbrEndDevices;
		u16ShortAdr = END_DEVICE_START_ADR + sCoordinatorData.u16NbrEndDevices;

		sCoordinatorData.sEndDeviceData[u16EndDeviceIndex].u16ShortAdr = u16ShortAdr;

		sCoordinatorData.sEndDeviceData[u16EndDeviceIndex].u32ExtAdrL =
				psMlmeInd->uParam.sIndAssociate.sDeviceAddr.u32L;

		sCoordinatorData.sEndDeviceData[u16EndDeviceIndex].u32ExtAdrH =
				psMlmeInd->uParam.sIndAssociate.sDeviceAddr.u32H;
		sMlmeReqRsp.uParam.sRspAssociate.u8Status = 0; /* Access granted */
		sCoordinatorData.u16NbrEndDevices++;

//        vDongleSetLED(ON);
	}
	else
	{
		sMlmeReqRsp.uParam.sRspAssociate.u8Status = 2; /* Denied */
	}

	/* Create association response */
	sMlmeReqRsp.u8Type = MAC_MLME_RSP_ASSOCIATE;
	sMlmeReqRsp.u8ParamLength = sizeof(MAC_MlmeRspAssociate_s);
	sMlmeReqRsp.uParam.sRspAssociate.sDeviceAddr.u32H = psMlmeInd->uParam.sIndAssociate.sDeviceAddr.u32H;
	sMlmeReqRsp.uParam.sRspAssociate.sDeviceAddr.u32L = psMlmeInd->uParam.sIndAssociate.sDeviceAddr.u32L;
	sMlmeReqRsp.uParam.sRspAssociate.u16AssocShortAddr = u16ShortAdr;

	sMlmeReqRsp.uParam.sRspAssociate.u8SecurityEnable = FALSE;

	/* Send association response. There is no confirmation for an association
	 response, hence no need to check */
	vAppApiMlmeRequest(&sMlmeReqRsp, &sMlmeSyncCfm);
}

/****************************************************************************
 *
 * NAME: vStartEnergyScan
 *
 * DESCRIPTION:
 * Starts an enery sacn on the channels specified.
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vStartEnergyScan(void)
{
	/* Structures used to hold data for MLME request and response */
	MAC_MlmeReqRsp_s sMlmeReqRsp;
	MAC_MlmeSyncCfm_s sMlmeSyncCfm;

	DBG_vPrintf(DEBUG_DEVICE_FUNC, "vStartEnergyScan\n");

	sCoordinatorData.eState = E_STATE_ENERGY_SCANNING;

	/* Start energy detect scan */
	sMlmeReqRsp.u8Type = MAC_MLME_REQ_SCAN;
	sMlmeReqRsp.u8ParamLength = sizeof(MAC_MlmeReqStart_s);
	sMlmeReqRsp.uParam.sReqScan.u8ScanType = MAC_MLME_SCAN_TYPE_ENERGY_DETECT;
	sMlmeReqRsp.uParam.sReqScan.u32ScanChannels = SCAN_CHANNELS;
	sMlmeReqRsp.uParam.sReqScan.u8ScanDuration = ENERGY_SCAN_DURATION;

	vAppApiMlmeRequest(&sMlmeReqRsp, &sMlmeSyncCfm);
}

/****************************************************************************
 *
 * NAME: vHandleEnergyScanResponse
 *
 * DESCRIPTION:
 * Selects a channel with low enery content for use by the wireless UART.
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vHandleEnergyScanResponse(MAC_MlmeDcfmInd_s *psMlmeInd)
{
	uint8 i = 0;
	uint8 u8MinEnergy;

	DBG_vPrintf(DEBUG_DEVICE_FUNC, "vHandleEnergyScanResponse\n");

	u8MinEnergy = (psMlmeInd->uParam.sDcfmScan.uList.au8EnergyDetect[0]);

	sCoordinatorData.u8Channel = CHANNEL_MIN;

	/* Search list to find quietest channel */
	while (i < psMlmeInd->uParam.sDcfmScan.u8ResultListSize)
	{
		if ((psMlmeInd->uParam.sDcfmScan.uList.au8EnergyDetect[i]) < u8MinEnergy)
		{
			u8MinEnergy = (psMlmeInd->uParam.sDcfmScan.uList.au8EnergyDetect[i]);
			sCoordinatorData.u8Channel = i + CHANNEL_MIN;
		}
		i++;
	}
	vStartCoordinator();
}

/****************************************************************************
 *
 * NAME: vStartCoordinator
 *
 * DESCRIPTION:
 * Starts the network by configuring the controller board to act as the PAN
 * coordinator.
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * TRUE if network was started successfully otherwise FALSE
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vStartCoordinator(void)
{
	/* Structures used to hold data for MLME request and response */
	MAC_MlmeReqRsp_s sMlmeReqRsp;
	MAC_MlmeSyncCfm_s sMlmeSyncCfm;

	DBG_vPrintf(DEBUG_DEVICE_FUNC, "vStartCoordinator\n");

	sCoordinatorData.eState = E_STATE_COORDINATOR_STARTED;

	/* Start Pan */
	sMlmeReqRsp.u8Type = MAC_MLME_REQ_START;
	sMlmeReqRsp.u8ParamLength = sizeof(MAC_MlmeReqStart_s);
	sMlmeReqRsp.uParam.sReqStart.u16PanId = PAN_ID;
	sMlmeReqRsp.uParam.sReqStart.u8Channel = sCoordinatorData.u8Channel;
	sMlmeReqRsp.uParam.sReqStart.u8BeaconOrder = 0x0F;
	sMlmeReqRsp.uParam.sReqStart.u8SuperframeOrder = 0x0F;
	sMlmeReqRsp.uParam.sReqStart.u8PanCoordinator = TRUE;
	sMlmeReqRsp.uParam.sReqStart.u8BatteryLifeExt = FALSE;
	sMlmeReqRsp.uParam.sReqStart.u8Realignment = FALSE;
	sMlmeReqRsp.uParam.sReqStart.u8SecurityEnable = FALSE;

	vAppApiMlmeRequest(&sMlmeReqRsp, &sMlmeSyncCfm);
}

/****************************************************************************
 *
 * NAME: vProcessEventQueues
 *
 * DESCRIPTION:
 * Check each of the three event queues and process and items found.
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vProcessEventQueues(void)
{
	MAC_MlmeDcfmInd_s *psMlmeInd;
	MAC_McpsDcfmInd_s *psMcpsInd;
	AppQApiHwInd_s *psAHI_Ind;

	/* Check for anything on the MCPS upward queue */
	do
	{
		psMcpsInd = psAppQApiReadMcpsInd();
		if (psMcpsInd != NULL)
		{
			vProcessIncomingMcps(psMcpsInd);
			vAppQApiReturnMcpsIndBuffer(psMcpsInd);
		}
	} while (psMcpsInd != NULL);

	/* Check for anything on the MLME upward queue */
	do
	{
		psMlmeInd = psAppQApiReadMlmeInd();
		if (psMlmeInd != NULL)
		{
			vProcessIncomingMlme(psMlmeInd);
			vAppQApiReturnMlmeIndBuffer(psMlmeInd);
		}
	} while (psMlmeInd != NULL);

	/* Check for anything on the AHI upward queue */
	do
	{
		psAHI_Ind = psAppQApiReadHwInd();
		if (psAHI_Ind != NULL)
		{
			vProcessIncomingHwEvent(psAHI_Ind);
			vAppQApiReturnHwIndBuffer(psAHI_Ind);
		}
	} while (psAHI_Ind != NULL);
}

/****************************************************************************
 *
 * NAME: vProcessIncomingMlme
 *
 * DESCRIPTION:
 * Process any incoming managment events from the stack.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  psMlmeInd
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vProcessIncomingMlme(MAC_MlmeDcfmInd_s *psMlmeInd)
{
	switch (psMlmeInd->u8Type)
	{
	case MAC_MLME_IND_ASSOCIATE: /* Incoming association request */
		if (sCoordinatorData.eState == E_STATE_COORDINATOR_STARTED)
		{
			vHandleNodeAssociation(psMlmeInd);
		}
		break;

	case MAC_MLME_DCFM_SCAN: /* Incoming scan results */
		if (psMlmeInd->uParam.sDcfmScan.u8ScanType == MAC_MLME_SCAN_TYPE_ENERGY_DETECT)
		{
			if (sCoordinatorData.eState == E_STATE_ENERGY_SCANNING)
			{
				/* Process energy scan results and start device as coordinator */
				vHandleEnergyScanResponse(psMlmeInd);
			}
		}
		break;
	case MAC_MLME_IND_DISASSOCIATE: /**< Use with tagMAC_MlmeIndDisassociate_s */
		DBG_vPrintf(DEBUG_DEVICE_FUNC, "DISASSOCIATE\n");
		break;
	case MAC_MLME_IND_SYNC_LOSS:
		DBG_vPrintf(DEBUG_DEVICE_FUNC, "sync loss\n");

		break;/**< Use with tagMAC_MlmeIndSyncLoss_s */
	default:
		break;
	}
}

/****************************************************************************
 *
 * NAME: vProcessIncomingData
 *
 * DESCRIPTION:
 * Process incoming data events from the stack.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  psMcpsInd
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vProcessIncomingMcps(MAC_McpsDcfmInd_s *psMcpsInd)
{
	/* Only handle incoming data events one device has been started as a
	 coordinator */
	if (sCoordinatorData.eState >= E_STATE_COORDINATOR_STARTED)
	{
		switch (psMcpsInd->u8Type)
		{
		case MAC_MCPS_IND_DATA: /* Incoming data frame */
			vHandleMcpsDataInd(psMcpsInd);
			break;
		case MAC_MCPS_DCFM_DATA: /* Incoming acknowledgement or ack timeout */
			vHandleMcpsDataDcfm(psMcpsInd);
			break;
		default:
			break;
		}
	}
}

/****************************************************************************
 *
 * NAME: vHandleMcpsDataDcfm
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 *
 * RETURNS:
 *
 * NOTES:
 ****************************************************************************/
PRIVATE void vHandleMcpsDataDcfm(MAC_McpsDcfmInd_s *psMcpsInd)
{
	if (psMcpsInd->uParam.sDcfmData.u8Status == MAC_ENUM_SUCCESS)
	{
		/* Data frame transmission successful */
	}
	else
	{
		DBG_vPrintf(DEBUG_DEVICE_FUNC, "tranmission failed\n");
		/* Data transmission failed after 3 retries at MAC layer. */
	}
}

/****************************************************************************
 *
 * NAME: vHandleMcpsDataInd
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 *
 * RETURNS:
 *
 * NOTES:
 ****************************************************************************/
PRIVATE void vHandleMcpsDataInd(MAC_McpsDcfmInd_s *psMcpsInd)
{
	MAC_RxFrameData_s *psFrame;

	psFrame = &psMcpsInd->uParam.sIndData.sFrame;

	/* Check application layer sequence number of frame and reject if it is
	 the same as the last frame, i.e. same frame has been received more
	 than once. */
//	if (psFrame->au8Sdu[0] >= sCoordinatorData.u8RxPacketSeqNb)
	{
		sCoordinatorData.u8RxPacketSeqNb++;
		vProcessReceivedDataPacket(&psFrame->au8Sdu[1], (psFrame->u8SduLength) - 1, psFrame->sSrcAddr.uAddr.u16Short);
	}
}

/****************************************************************************
 *
 * NAME: vTransmitDataPacket
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vTransmitDataPacket(uint8 *pu8Data, uint8 u8Len, uint16 u16DestAdr)
{
	MAC_McpsReqRsp_s sMcpsReqRsp;
	MAC_McpsSyncCfm_s sMcpsSyncCfm;
	uint8 *pu8Payload, i = 0;

	/* Create frame transmission request */
	sMcpsReqRsp.u8Type = MAC_MCPS_REQ_DATA;
	sMcpsReqRsp.u8ParamLength = sizeof(MAC_McpsReqData_s);
	/* Set handle so we can match confirmation to request */
	sMcpsReqRsp.uParam.sReqData.u8Handle = 1;
	/* Use short address for source */
	sMcpsReqRsp.uParam.sReqData.sFrame.sSrcAddr.u8AddrMode = 2;
	sMcpsReqRsp.uParam.sReqData.sFrame.sSrcAddr.u16PanId = PAN_ID;
	sMcpsReqRsp.uParam.sReqData.sFrame.sSrcAddr.uAddr.u16Short =
	COORDINATOR_ADR;
	/* Use short address for destination */
	sMcpsReqRsp.uParam.sReqData.sFrame.sDstAddr.u8AddrMode = 2;
	sMcpsReqRsp.uParam.sReqData.sFrame.sDstAddr.u16PanId = PAN_ID;
	sMcpsReqRsp.uParam.sReqData.sFrame.sDstAddr.uAddr.u16Short = u16DestAdr;
	/* Frame requires ack but not security, indirect transmit or GTS */
	sMcpsReqRsp.uParam.sReqData.sFrame.u8TxOptions = MAC_TX_OPTION_ACK;

	pu8Payload = sMcpsReqRsp.uParam.sReqData.sFrame.au8Sdu;

	pu8Payload[0] = sCoordinatorData.u8TxPacketSeqNb++;

	for (i = 1; i < (u8Len + 1); i++)
	{
		pu8Payload[i] = *pu8Data++;
	}

	/* Set frame length */
	sMcpsReqRsp.uParam.sReqData.sFrame.u8SduLength = i;

	/* Request transmit */
	vAppApiMcpsRequest(&sMcpsReqRsp, &sMcpsSyncCfm);
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
