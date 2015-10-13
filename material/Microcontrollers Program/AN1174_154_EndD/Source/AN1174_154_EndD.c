/****************************************************************************
 *
 * MODULE:             enddevice.c
 *
 * COMPONENT:          enddevice.c,v
 *
 * VERSION:
 *
 * REVISION:           1.3
 *
 * DATED:              2006/11/06 10:03:46
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
#include "Uart.h"
#include "serialq.h"
#include "config.h"
#include "robot_functions.h"
#include "robot_timers.h"
#include "I2C.h"
#include "lsm6ds0.h"
#include "qt1070.h"
#include "Timers.h"
#include <dbg.h>
#include <dbg_uart.h>
#include "CircularBuffer.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
	E_STATE_IDLE, E_STATE_ACTIVE_SCANNING, E_STATE_ASSOCIATING, E_STATE_ASSOCIATED
} teState;

typedef struct
{
	teState eState;
	uint8 u8Channel;
	uint8 u8TxPacketSeqNb;
	uint8 u8RxPacketSeqNb;
	uint16 u16Address;
} tsEndDeviceData;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vInitSystem(void);
PRIVATE void vProcessEventQueues(void);
PRIVATE void vProcessIncomingMlme(MAC_MlmeDcfmInd_s *psMlmeInd);
PRIVATE void vProcessIncomingMcps(MAC_McpsDcfmInd_s *psMcpsInd);
PRIVATE void vProcessIncomingHwEvent(AppQApiHwInd_s *psAHI_Ind);
PRIVATE void vStartActiveScan(void);
PRIVATE void vHandleActiveScanResponse(MAC_MlmeDcfmInd_s *psMlmeInd);
PRIVATE void vStartAssociate(void);
PRIVATE void vHandleAssociateResponse(MAC_MlmeDcfmInd_s *psMlmeInd);
PRIVATE void vHandleMcpsDataInd(MAC_McpsDcfmInd_s *psMcpsInd);
PRIVATE void vHandleMcpsDataDcfm(MAC_McpsDcfmInd_s *psMcpsInd);
PRIVATE void vTransmitDataPacket(uint8 *pu8Data, uint8 u8Len, uint16 u16DestAdr);
PRIVATE void vProcessReceivedDataPacket(uint8 *pu8Data, uint8 u8Len);
PRIVATE void vMain(void);
PRIVATE void vSendMessageToCoordinator(uint8 id, uint8 size, uint8* payload);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
/* Handles from the MAC */
PRIVATE void *s_pvMac;
PRIVATE MAC_Pib_s *s_psMacPib;
PRIVATE tsEndDeviceData sEndDeviceData;
PRIVATE bool_t isUARTOpened;
PRIVATE uint8 data2Stream;
PRIVATE bool dataReadyToSend;
PRIVATE uint8 touchData;
PRIVATE uint8 tmpBuffer[30] = { 0 };
PRIVATE StatusData currentStatus;
PRIVATE uint8 messagesToSend = 0;
//PRIVATE CircularBuffer accelBuffer;
PRIVATE bool measureTime = false;
uint64 timestamp = 0;
ManipulationState manipulationState;

extern ImuRawData imuData;
extern AccelRawData acceleration;
extern GyroRawData rotation;
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

	vStartActiveScan();

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

	/* Initialise end device state */
	sEndDeviceData.eState = E_STATE_IDLE;
	sEndDeviceData.u8TxPacketSeqNb = 0;
	sEndDeviceData.u8RxPacketSeqNb = 0;

	/* Set up the MAC handles. Must be called AFTER u32AppQApiInit() */
	s_pvMac = pvAppApiGetMacHandle();
	s_psMacPib = MAC_psPibGetHandle(s_pvMac);

	/* Set Pan ID in PIB (also sets match register in hardware) */
	MAC_vPibSetPanId(s_pvMac, PAN_ID);

	/* Enable receiver to be on when idle */
	MAC_vPibSetRxOnWhenIdle(s_pvMac, TRUE, FALSE);

	while (!bAHI_Clock32MHzStable());

	/* Initialise UART */
#ifdef DBG_ENABLE
	DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);
	isUARTOpened = bSerialQ_Init() & bUART_Init();
#endif

	DBG_vPrintf(DEBUG_DEVICE_FUNC, "ROBOT\n");
	DBG_vPrintf(DEBUG_DEVICE_FUNC, "AppColdStart\n");
	DBG_vPrintf(DEBUG_DEVICE_FUNC, "Device_vInit\n");

	vRobotInitDIOs();
	vWaitMilliseconds(2000);
	vWaitMilliseconds(2000);
	vInitI2C();
	vRobotInitSensors();
	vRobotInitTickTimer();

	touchData = 0;
	dataReadyToSend = FALSE;
	data2Stream = STREAM_STATUS;

	manipulationState.currentState = manipulationState.previousState = NO_CONTACT_STILL;
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

		if (dataReadyToSend)
		{
			switch (data2Stream)
			{
			case STREAM_IMU:
				vSendMessageToCoordinator(IMU_RAW_DATA, sizeof(imuData), (uint8*) &imuData);
				break;
			case STREAM_ACCEL:
				vSendMessageToCoordinator(ACCEL_RAW_DATA, sizeof(acceleration), (uint8*) &acceleration);
				break;
			case STREAM_GYRO:
				vSendMessageToCoordinator(GYRO_RAW_DATA, sizeof(rotation), (uint8*) &rotation);
				break;
			case STREAM_TOUCH:
				vSendMessageToCoordinator(TOUCH_RAW_DATA, sizeof(touchData), &touchData);
				break;
			case STREAM_STATUS:
				vSendMessageToCoordinator(STATUS_DATA, sizeof(currentStatus), (uint8*) &currentStatus);
				break;
			case STREAM_TIMESTAMP:
				vSendMessageToCoordinator(TIME_DATA, sizeof(timestamp), (uint8*) &timestamp);
				break;
			default:
				break;
			}
			dataReadyToSend = false;
		}
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
	bool b;
	switch (psAHI_Ind->u32DeviceId)
	{
	case E_AHI_DEVICE_TICK_TIMER:
		if (measureTime){
			timestamp += ROBOT_TICK_PERIOD_MS;
			if(sEndDeviceData.eState == E_STATE_ASSOCIATED)
			dataReadyToSend = true;
		}
		b = bRobotUpdateManipulationState(&currentStatus);

		if (sEndDeviceData.eState == E_STATE_ASSOCIATED)
		{
			if (b)
				dataReadyToSend = true;
		}
		break;
	default:
		break;
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
PRIVATE void vProcessReceivedDataPacket(uint8 *pu8Data, uint8 u8Len)
{
	Message *tmpMsg;
	tmpMsg = (Message*) pu8Data;

	switch (tmpMsg->message_id)
	{
	case DATA_TO_STREAM:
		memcpy(&data2Stream, &(tmpMsg->payload), tmpMsg->message_size);
		break;
		//receiving the amount of messages to send
	case ASK_FOR_DATA:
		memcpy(&messagesToSend, &(tmpMsg->payload), tmpMsg->message_size);
		break;
	case SET_TIMESTAMP:
		measureTime = TRUE;
		memcpy(&timestamp, &(tmpMsg->payload), tmpMsg->message_size);
		break;

	default:
		break;
	}
}

/****************************************************************************
 *
 * NAME: vSendMessageToCoordinator
 *
 * DESCRIPTION:
 * Send the data in parameter to the coordinator of the network.
 *
 * PARAMETERS:      Name            RW  Usage
 * id : type of the message
 * size : size of the data to send
 * payload: pointer to the first byte of the data structure to send
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vSendMessageToCoordinator(uint8 id, uint8 size, uint8* payload)
{
	tmpBuffer[0] = id;

	if (size % 2 == 0)
	{
		tmpBuffer[1] = size;
		if (size > 0)
			memcpy(&tmpBuffer[2], payload, size);
		vTransmitDataPacket((uint8*) &tmpBuffer[0], size + 2, COORDINATOR_ADR);
	}
	else
	{
		tmpBuffer[1] = size + 1;
		memcpy(&tmpBuffer[2], payload, size);
		tmpBuffer[2 + size] = 0;
		vTransmitDataPacket((uint8*) &tmpBuffer[0], size + 3, COORDINATOR_ADR);
	}
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
	/* We respond to several MLME indications and confirmations, depending
	 on mode */
	switch (psMlmeInd->u8Type)
	{
	/* Deferred confirmation that the scan is complete */
	case MAC_MLME_DCFM_SCAN:
		if (sEndDeviceData.eState == E_STATE_ACTIVE_SCANNING)
		{
			vHandleActiveScanResponse(psMlmeInd);
		}
		break;

		/* Deferred confirmation that the association process is complete */
	case MAC_MLME_DCFM_ASSOCIATE:
		/* Only respond to this if associating */
		if (sEndDeviceData.eState == E_STATE_ASSOCIATING)
		{
			vHandleAssociateResponse(psMlmeInd);
		}
		break;

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
	if (sEndDeviceData.eState >= E_STATE_ASSOCIATED)
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
		/* Data transmission falied after 3 retries at MAC layer. */
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

	if (psFrame->sSrcAddr.uAddr.u16Short == COORDINATOR_ADR)
	{
		if (psFrame->au8Sdu[0] >= sEndDeviceData.u8RxPacketSeqNb)
		{
			sEndDeviceData.u8RxPacketSeqNb++;
			vProcessReceivedDataPacket(&psFrame->au8Sdu[1], (psFrame->u8SduLength) - 1);
		}
	}
}

/****************************************************************************
 *
 * NAME: vStartAssociate
 *
 * DESCRIPTION:
 * Start the association process with the network coordinator.
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * Assumes that a network has been found during the network scan.
 ****************************************************************************/
PRIVATE void vStartAssociate(void)
{
	MAC_MlmeReqRsp_s sMlmeReqRsp;
	MAC_MlmeSyncCfm_s sMlmeSyncCfm;

	DBG_vPrintf(DEBUG_DEVICE_FUNC, "vStartAssociate\n");

	sEndDeviceData.eState = E_STATE_ASSOCIATING;

	/* Create associate request. We know short address and PAN ID of
	 coordinator as this is preset and we have checked that received
	 beacon matched this */

	sMlmeReqRsp.u8Type = MAC_MLME_REQ_ASSOCIATE;
	sMlmeReqRsp.u8ParamLength = sizeof(MAC_MlmeReqAssociate_s);
	sMlmeReqRsp.uParam.sReqAssociate.u8LogicalChan = sEndDeviceData.u8Channel;
	sMlmeReqRsp.uParam.sReqAssociate.u8Capability = 0x80; /* We want short address, other features off */
	sMlmeReqRsp.uParam.sReqAssociate.u8SecurityEnable = FALSE;
	sMlmeReqRsp.uParam.sReqAssociate.sCoord.u8AddrMode = 2;
	sMlmeReqRsp.uParam.sReqAssociate.sCoord.u16PanId = PAN_ID;
	sMlmeReqRsp.uParam.sReqAssociate.sCoord.uAddr.u16Short = COORDINATOR_ADR;

	/* Put in associate request and check immediate confirm. Should be
	 deferred, in which case response is handled by event handler */
	vAppApiMlmeRequest(&sMlmeReqRsp, &sMlmeSyncCfm);
}

/****************************************************************************
 *
 * NAME: vHandleAssociateResponse
 *
 * DESCRIPTION:
 * Handle the response generated by the stack as a result of the associate
 * start request.
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
PRIVATE void vHandleAssociateResponse(MAC_MlmeDcfmInd_s *psMlmeInd)
{
	DBG_vPrintf(DEBUG_DEVICE_FUNC, "vHandleAssociateResponse\n");

	/* If successfully associated with network coordinator */
	if (psMlmeInd->uParam.sDcfmAssociate.u8Status == MAC_ENUM_SUCCESS)
	{
		sEndDeviceData.u16Address = psMlmeInd->uParam.sDcfmAssociate.u16AssocShortAddr;
		sEndDeviceData.eState = E_STATE_ASSOCIATED;

		DBG_vPrintf(DEBUG_DEVICE_FUNC, "asking for time\n");
		vSendMessageToCoordinator(ASK_FOR_TIMESTAMP, 0, NULL);
		measureTime = true;
	}
	else
	{
		vStartActiveScan();
	}
}

/****************************************************************************
 *
 * NAME: vStartActiveScan
 *
 * DESCRIPTION:
 * Start a scan to search for a network to join.
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
PRIVATE void vStartActiveScan(void)
{
	MAC_MlmeReqRsp_s sMlmeReqRsp;
	MAC_MlmeSyncCfm_s sMlmeSyncCfm;

	DBG_vPrintf(DEBUG_DEVICE_FUNC, "vStartActiveScan\n");

	sEndDeviceData.eState = E_STATE_ACTIVE_SCANNING;

	/* Request scan */
	sMlmeReqRsp.u8Type = MAC_MLME_REQ_SCAN;
	sMlmeReqRsp.u8ParamLength = sizeof(MAC_MlmeReqScan_s);
	sMlmeReqRsp.uParam.sReqScan.u8ScanType = MAC_MLME_SCAN_TYPE_ACTIVE;
	sMlmeReqRsp.uParam.sReqScan.u32ScanChannels = SCAN_CHANNELS;
	sMlmeReqRsp.uParam.sReqScan.u8ScanDuration = ACTIVE_SCAN_DURATION;

	vAppApiMlmeRequest(&sMlmeReqRsp, &sMlmeSyncCfm);
}

/****************************************************************************
 *
 * NAME: vHandleActiveScanResponse
 *
 * DESCRIPTION:
 * Handle the reponse generated by the stack as a result of the network scan.
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
PRIVATE void vHandleActiveScanResponse(MAC_MlmeDcfmInd_s *psMlmeInd)
{
	MAC_PanDescr_s *psPanDesc;
	uint8 i;

	if (psMlmeInd->uParam.sDcfmScan.u8ScanType == MAC_MLME_SCAN_TYPE_ACTIVE)
	{
		if (psMlmeInd->uParam.sDcfmScan.u8Status == MAC_ENUM_SUCCESS)
		{
			i = 0;

			while (i < psMlmeInd->uParam.sDcfmScan.u8ResultListSize)
			{
				psPanDesc = &psMlmeInd->uParam.sDcfmScan.uList.asPanDescr[i];

				if ((psPanDesc->sCoord.u16PanId == PAN_ID) && (psPanDesc->sCoord.u8AddrMode == 2)
						&& (psPanDesc->sCoord.uAddr.u16Short == COORDINATOR_ADR)
						/* Check it is accepting association requests */
						&& (psPanDesc->u16SuperframeSpec & 0x8000))
				{
					sEndDeviceData.u8Channel = psPanDesc->u8LogicalChan;
					vStartAssociate();
					return;
				}
				i++;
			}
		}
		vStartActiveScan();
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
	sMcpsReqRsp.uParam.sReqData.sFrame.sSrcAddr.uAddr.u16Short = sEndDeviceData.u16Address;
	/* Use short address for destination */
	sMcpsReqRsp.uParam.sReqData.sFrame.sDstAddr.u8AddrMode = 2;
	sMcpsReqRsp.uParam.sReqData.sFrame.sDstAddr.u16PanId = PAN_ID;
	sMcpsReqRsp.uParam.sReqData.sFrame.sDstAddr.uAddr.u16Short = u16DestAdr;
	/* Frame requires ack but not security, indirect transmit or GTS */
	sMcpsReqRsp.uParam.sReqData.sFrame.u8TxOptions = MAC_TX_OPTION_ACK;

	pu8Payload = sMcpsReqRsp.uParam.sReqData.sFrame.au8Sdu;

	pu8Payload[0] = sEndDeviceData.u8TxPacketSeqNb++;

	for (i = 1; i < (u8Len + 1); i++)
	{
		pu8Payload[i] = *pu8Data++;
	}

	/* Set frame length */
	sMcpsReqRsp.uParam.sReqData.sFrame.u8SduLength = i;

	/* Request transmit */
	vAppApiMcpsRequest(&sMcpsReqRsp, &sMcpsSyncCfm);
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

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
