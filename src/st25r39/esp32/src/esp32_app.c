#include <stdio.h>
#include "lib_wrapper.h"
#include "esp32_app.h"
#include "esp32_config.h"
#include "esp32_st25r.h"

#include "rfal_nfcb.h"
#include "rfal_nfcf.h"
#include "rfal_nfcv.h"
#include "rfal_st25tb.h"
#include "st25r3916_irq.h"
#include <string.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(esp32_app);

/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */
#define RFAL_POLLER_DEVICES 4 /* Number of devices supported */

#define RFAL_POLLER_FOUND_NONE 0x00 /* No device found Flag        */
#define RFAL_POLLER_FOUND_A 0x01 /* NFC-A device found Flag     */
#define RFAL_POLLER_FOUND_B 0x02 /* NFC-B device found Flag     */
#define RFAL_POLLER_FOUND_F 0x04 /* NFC-F device found Flag     */
#define RFAL_POLLER_FOUND_V 0x08 /* NFC-V device Flag           */
#define RFAL_POLLER_FOUND_ST25TB 0x10 /* ST25TB device found flag */

#define ST25_IRQ_SIGNAL_VALUE (1 << 1)
/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */

/*! Main state                                                                            */
typedef enum
{
	RFAL_POLLER_STATE_INIT = 0, /* Initialize state            */
	RFAL_POLLER_STATE_TECHDETECT = 1, /* Technology Detection state  */
	RFAL_POLLER_STATE_COLAVOIDANCE = 2, /* Collision Avoidance state   */
	RFAL_POLLER_STATE_DEACTIVATION = 9 /* Deactivation state          */
} exampleRfalPollerState;

/*! Device interface                                                                      */
typedef enum
{
	RFAL_POLLER_INTERFACE_RF = 0, /* RF Frame interface          */
	RFAL_POLLER_INTERFACE_ISODEP = 1, /* ISO-DEP interface           */
	RFAL_POLLER_INTERFACE_NFCDEP = 2 /* NFC-DEP interface           */
} exampleRfalPollerRfInterface;

/*! Device struct containing all its details                                              */
typedef struct
{
	BSP_NFCTAG_Protocol_Id_t type; /* Device's type                */
	union
	{
		rfalNfcaListenDevice nfca; /* NFC-A Listen Device instance */
		rfalNfcbListenDevice nfcb; /* NFC-B Listen Device instance */
		rfalNfcfListenDevice nfcf; /* NFC-F Listen Device instance */
		rfalNfcvListenDevice nfcv; /* NFC-V Listen Device instance */
		rfalSt25tbListenDevice st25tb; /* NFC-V Listen Device instance */
	} dev; /* Device's instance            */
	union
	{
		rfalIsoDepDevice isoDep; /* ISO-DEP instance             */
		rfalNfcDepDevice nfcDep; /* NFC-DEP instance             */
	} proto; /* Device's protocol            */
	uint8_t NdefSupport;
	exampleRfalPollerRfInterface rfInterface; /* Device's interface           */
} exampleRfalPollerDevice;

/** Detection mode for the demo */
typedef enum
{
	DETECT_MODE_POLL = 0, /** Continuous polling for tags */
	DETECT_MODE_WAKEUP = 1, /** Waiting for the ST25R3916 wakeup detection */
	DETECT_MODE_AWAKEN = 2 /** Awaken by the ST25R3916 wakeup feature */
} detectMode_t;

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */
static uint8_t gDevCnt; /* Number of devices found                         */
static exampleRfalPollerDevice
	gDevList[RFAL_POLLER_DEVICES]; /* Device List                                     */
static exampleRfalPollerState gState; /* Main state                                      */
static uint8_t gTechsFound; /* Technologies found bitmask                      */
static char gUIDStr[30];
static bool gUIDStatus;
exampleRfalPollerDevice* gActiveDev; /* Active device pointer                           */

uint8_t globalCommProtectCnt;

static detectMode_t detectMode = DETECT_MODE_POLL; /* Current tag detection method */

static rfalDpoEntry dpoSetup[] = {
	// new antenna board
	{.rfoRes = 0, .inc = 255, .dec = 115},
	{.rfoRes = 2, .inc = 100, .dec = 0x00}};

K_MUTEX_DEFINE(st25mutex);
K_MUTEX_DEFINE(st25lowmutex);

static bool rfalPollerTechDetection(void);
static bool rfalPollerCollResolution(void);
static bool rfalPollerDeactivate(void);

static void rfalPreTransceiveCb(void)
{
	rfalDpoAdjust();
}

static void displayTag(int index, char* type, uint8_t* uid)
{
	char str[30] = "";
	char uid_str[30] = "";
	if(index > 3)
		return;

	if(uid != NULL)
	{
		int len = sprintf(uid_str,
						  "%s %02X:%02X:%02X:%02X:%02X:%02X:%02X",
						  type,
						  uid[0],
						  uid[1],
						  uid[2],
						  uid[3],
						  uid[4],
						  uid[5],
						  uid[6]);
		uid_str[len] = '\0';
		if(uid[0] == 0xE0)
		{
			len = sprintf(gUIDStr,
						  "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
						  uid[0],
						  uid[1],
						  uid[2],
						  uid[3],
						  uid[4],
						  uid[5],
						  uid[6],
						  uid[7]);
		}
		else
		{
			len = sprintf(gUIDStr,
						  "E0:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
						  uid[0],
						  uid[1],
						  uid[2],
						  uid[3],
						  uid[4],
						  uid[5],
						  uid[6]);
		}
		gUIDStr[len] = '\0';
		gUIDStatus = true;
	}
	else
	{
		strcpy(str, "                  ");
	}
	LOG_INF("UUID %s", gUIDStr);
}

void st25r_initRFAL(void)
{
	/* RFAL initalisation */
	rfalAnalogConfigInitialize();
	if(rfalInitialize() != ERR_NONE)
	{
		LOG_ERR("Init error!");
		return;
	}
	LOG_INF("rfalInitialize");
	/* DPO setup */
	rfalDpoInitialize();
	rfalDpoSetMeasureCallback(rfalChipMeasureAmplitude);
	rfalDpoTableWrite(dpoSetup, sizeof(dpoSetup) / sizeof(rfalDpoEntry));
	rfalDpoSetEnabled(true);
	rfalSetPreTxRxCallback(&rfalPreTransceiveCb);
}

/*!
 * \brief Poller Technology Detection
 *
 * This method implements the Technology Detection / Poll for different
 * device technologies.
 *
 * \return true         : One or more devices have been detected
 * \return false         : No device have been detected
 *
 */
static bool rfalPollerTechDetection(void)
{
	ReturnCode err = ERR_NONE;
	rfalNfcaSensRes sensRes;
	rfalNfcbSensbRes sensbRes;
	rfalNfcvInventoryRes invRes;
	uint8_t sensbResLen;

	gTechsFound = RFAL_POLLER_FOUND_NONE;

	/*******************************************************************************/
	/* NFC-A Technology Detection                                                  */
	/*******************************************************************************/

	rfalNfcaPollerInitialize(); /* Initialize RFAL for NFC-A */
	err = rfalFieldOnAndStartGT(); /* Turns the Field On and starts GT timer */
	err = rfalNfcaPollerTechnologyDetection(RFAL_COMPLIANCE_MODE_NFC,
											&sensRes); /* Poll for NFC-A devices */
	if(err == ERR_NONE)
	{
		gTechsFound |= RFAL_POLLER_FOUND_A;
	}

	/*******************************************************************************/
	/* NFC-B Technology Detection                                                  */
	/*******************************************************************************/

	rfalNfcbPollerInitialize(); /* Initialize RFAL for NFC-B */
	rfalFieldOnAndStartGT(); /* As field is already On only starts GT timer */

	err = rfalNfcbPollerTechnologyDetection(
		RFAL_COMPLIANCE_MODE_NFC, &sensbRes, &sensbResLen); /* Poll for NFC-B devices */
	if(err == ERR_NONE)
	{
		gTechsFound |= RFAL_POLLER_FOUND_B;
	}

	/*******************************************************************************/
	/* ST25TB Technology Detection                                                  */
	/*******************************************************************************/

	rfalSt25tbPollerInitialize(); /* Initialize RFAL for NFC-B */
	rfalFieldOnAndStartGT();
	uint8_t chipId;
	/* As field is already On only starts GT timer */
	err = rfalSt25tbPollerCheckPresence(&chipId);
	if(err == ERR_NONE)
	{
		gTechsFound |= RFAL_POLLER_FOUND_ST25TB;
	}

	/*******************************************************************************/
	/* NFC-F Technology Detection                                                  */
	/*******************************************************************************/

	rfalNfcfPollerInitialize(RFAL_BR_212); /* Initialize RFAL for NFC-F */
	rfalFieldOnAndStartGT(); /* As field is already On only starts GT timer */

	err = rfalNfcfPollerCheckPresence(); /* Poll for NFC-F devices */
	if(err == ERR_NONE)
	{
		gTechsFound |= RFAL_POLLER_FOUND_F;
	}

	/*******************************************************************************/
	/* NFC-V Technology Detection                                                  */
	/*******************************************************************************/

	rfalNfcvPollerInitialize(); /* Initialize RFAL for NFC-V */
	rfalFieldOnAndStartGT(); /* As field is already On only starts GT timer */

	err = rfalNfcvPollerCheckPresence(&invRes); /* Poll for NFC-V devices */
	if(err == ERR_NONE)
	{
		gTechsFound |= RFAL_POLLER_FOUND_V;
	}

	return (gTechsFound != RFAL_POLLER_FOUND_NONE);
}

/*!
 ******************************************************************************
 * \brief Poller Collision Resolution
 *
 * This method implements the Collision Resolution on all technologies that
 * have been detected before.
 *
 * \return true         : One or more devices identified
 * \return false        : No device have been identified
 *
 ******************************************************************************
 */
static bool rfalPollerCollResolution(void)
{
	uint8_t i;
	uint8_t devCnt;
	ReturnCode err = ERR_NONE;

	/*******************************************************************************/
	/* NFC-A Collision Resolution                                                  */
	/*******************************************************************************/
	if(gTechsFound &
	   RFAL_POLLER_FOUND_A) /* If a NFC-A device was found/detected, perform Collision Resolution */
	{
		rfalNfcaListenDevice nfcaDevList[RFAL_POLLER_DEVICES];

		rfalNfcaPollerInitialize();
		err = rfalNfcaPollerFullCollisionResolution(
			RFAL_COMPLIANCE_MODE_NFC, (RFAL_POLLER_DEVICES - gDevCnt), nfcaDevList, &devCnt);
		if((err == ERR_NONE) && (devCnt != 0))
		{

			for(i = 0; i < devCnt;
				i++) /* Copy devices found form local Nfca list into global device list */
			{
				displayTag(gDevCnt, "NFC-A", nfcaDevList[i].nfcId1);
				gDevList[gDevCnt].type = BSP_NFCTAG_NFCA;
				gDevList[gDevCnt].dev.nfca = nfcaDevList[i];
				gDevCnt++;
			}
		}
	}

	/*******************************************************************************/
	/* NFC-B Collision Resolution                                                  */
	/*******************************************************************************/
	if(gTechsFound &
	   RFAL_POLLER_FOUND_B) /* If a NFC-A device was found/detected, perform Collision Resolution */
	{
		rfalNfcbListenDevice nfcbDevList[RFAL_POLLER_DEVICES];

		rfalNfcbPollerInitialize();
		err = rfalNfcbPollerCollisionResolution(
			RFAL_COMPLIANCE_MODE_NFC, (RFAL_POLLER_DEVICES - gDevCnt), nfcbDevList, &devCnt);
		if((err == ERR_NONE) && (devCnt != 0))
		{
			for(i = 0; i < devCnt;
				i++) /* Copy devices found form local Nfcb list into global device list */
			{
				displayTag(gDevCnt, "NFC-B", nfcbDevList[i].sensbRes.nfcid0);
				gDevList[gDevCnt].type = BSP_NFCTAG_NFCB;
				gDevList[gDevCnt].dev.nfcb = nfcbDevList[i];
				gDevCnt++;
			}
		}
	}

	/*******************************************************************************/
	/* ST25TB Collision Resolution                                                  */
	/*******************************************************************************/
	if(gTechsFound &
	   RFAL_POLLER_FOUND_ST25TB) /* If a NFC-A device was found/detected, perform Collision Resolution */
	{
		rfalSt25tbListenDevice st25tbDevList[RFAL_POLLER_DEVICES];
		rfalSt25tbPollerInitialize();
		err = rfalSt25tbPollerCollisionResolution(
			(RFAL_POLLER_DEVICES - gDevCnt), st25tbDevList, &devCnt);
		if((err == ERR_NONE) && (devCnt != 0))
		{
			for(i = 0; i < devCnt;
				i++) /* Copy devices found form local Nfcb list into global device list */
			{
				displayTag(gDevCnt, "ST25TB", st25tbDevList[i].UID);
				gDevList[gDevCnt].type = BSP_NFCTAG_ST25TB;
				gDevList[gDevCnt].dev.st25tb = st25tbDevList[i];
				gDevCnt++;
			}
		}
	}

	/*******************************************************************************/
	/* NFC-F Collision Resolution                                                  */
	/*******************************************************************************/
	if(gTechsFound &
	   RFAL_POLLER_FOUND_F) /* If a NFC-F device was found/detected, perform Collision Resolution */
	{
		rfalNfcfListenDevice nfcfDevList[RFAL_POLLER_DEVICES];

		rfalNfcfPollerInitialize(RFAL_BR_212);
		err = rfalNfcfPollerCollisionResolution(
			RFAL_COMPLIANCE_MODE_NFC, (RFAL_POLLER_DEVICES - gDevCnt), nfcfDevList, &devCnt);
		if((err == ERR_NONE) && (devCnt != 0))
		{
			for(i = 0; i < devCnt;
				i++) /* Copy devices found form local Nfcf list into global device list */
			{
				displayTag(gDevCnt, "NFC-F", nfcfDevList[i].sensfRes.NFCID2);
				gDevList[gDevCnt].type = BSP_NFCTAG_NFCF;
				gDevList[gDevCnt].dev.nfcf = nfcfDevList[i];
				gDevCnt++;
			}
		}
	}

	/*******************************************************************************/
	/* NFC-V Collision Resolution                                                  */
	/*******************************************************************************/
	if(gTechsFound &
	   RFAL_POLLER_FOUND_V) /* If a NFC-F device was found/detected, perform Collision Resolution */
	{
		rfalNfcvListenDevice nfcvDevList[RFAL_POLLER_DEVICES];

		rfalNfcvPollerInitialize();
		err = rfalNfcvPollerCollisionResolution(
			(RFAL_POLLER_DEVICES - gDevCnt), nfcvDevList, &devCnt);
		if((err == ERR_NONE) && (devCnt != 0))
		{
			for(i = 0; i < devCnt;
				i++) /* Copy devices found form local Nfcf list into global device list */
			{
				uint8_t uid[RFAL_NFCV_UID_LEN];
				for(int uid_i = 0; uid_i < RFAL_NFCV_UID_LEN; uid_i++)
					uid[uid_i] = nfcvDevList[i].InvRes.UID[RFAL_NFCV_UID_LEN - uid_i - 1];
				displayTag(gDevCnt, "NFC-V", uid);
				gDevList[gDevCnt].type = BSP_NFCTAG_NFCV;
				gDevList[gDevCnt].dev.nfcv = nfcvDevList[i];
				gDevCnt++;
			}
		}
	}

	return (gDevCnt > 0);
}

/*!
 ******************************************************************************
 * \brief Poller NFC DEP Deactivate
 *
 * This method Deactivates the device if a deactivation procedure exists
 *
 * \return true         : Deactivation successful
 * \return false        : Deactivation failed
 *
 ******************************************************************************
 */
static bool rfalPollerDeactivate(void)
{
	if(gActiveDev != NULL) /* Check if a device has been activated */
	{
		switch(gActiveDev->rfInterface)
		{
		/*******************************************************************************/
		case RFAL_POLLER_INTERFACE_RF:
			break; /* No specific deactivation to be performed */

			/*******************************************************************************/
		case RFAL_POLLER_INTERFACE_ISODEP:
			rfalIsoDepDeselect(); /* Send a Deselect to device */
			break;

			/*******************************************************************************/
		case RFAL_POLLER_INTERFACE_NFCDEP:
			rfalNfcDepRLS(); /* Send a Release to device */
			break;

		default:
			return false;
		}
		LOG_INF("Device deactivated \r\n");
	}

	return true;
}

/*!
 ******************************************************************************
 * \brief Passive Poller Run
 *
 * This method implements the main state machine going thought all the
 * different activities that a Reader/Poller device (PCD) needs to perform.
 *
 *
 ******************************************************************************
 */
void rfalPollerRun(void)
{
	/* Initialize RFAL */
	LOG_INF("RFAL Poller started");

	for(;;)
	{	
		rfalWorker(); /* Execute RFAL process */
		if(detectMode == DETECT_MODE_WAKEUP)
		{
			if(!rfalWakeUpModeHasWoke())
			{
				// still sleeping, don't do nothing...
				continue;
			}
			// exit wake up mode
			detectMode = DETECT_MODE_AWAKEN;
			rfalWakeUpModeStop();
		}
		switch(gState)
		{
		/*******************************************************************************/
		case RFAL_POLLER_STATE_INIT:

			gTechsFound = RFAL_POLLER_FOUND_NONE;
			gActiveDev = NULL;
			gDevCnt = 0;

			gState = RFAL_POLLER_STATE_TECHDETECT;
			break;

			/*******************************************************************************/
		case RFAL_POLLER_STATE_TECHDETECT:

			if(!rfalPollerTechDetection()) /* Poll for nearby devices in different technologies */
			{
				gState = RFAL_POLLER_STATE_DEACTIVATION; /* If no device was found, restart loop */
				break;
			}

			gState =
				RFAL_POLLER_STATE_COLAVOIDANCE; /* One or more devices found, go to Collision Avoidance */
			break;

			/*******************************************************************************/
		case RFAL_POLLER_STATE_COLAVOIDANCE:
			// add delay to avoid NFC-V Anticol frames back to back
			if(!rfalPollerCollResolution()) /* Resolve any eventual collision */
			{
				gState =
					RFAL_POLLER_STATE_DEACTIVATION; /* If Collision Resolution was unable to retrieve any device, restart loop */
				break;
			}
			LOG_INF("Found %x", gTechsFound);
			LOG_INF("Device(s) found: %d \r\n", gDevCnt);

			gState = RFAL_POLLER_STATE_DEACTIVATION;

			break;

			/*******************************************************************************/
		case RFAL_POLLER_STATE_DEACTIVATION:
			rfalPollerDeactivate(); /* If a card has been activated, properly deactivate the device */
			rfalFieldOff(); /* Turn the Field Off powering down any device nearby */
			k_sleep(K_MSEC(100));
			if((detectMode == DETECT_MODE_AWAKEN) && (gDevCnt == 0))
			{
				// no more tags, restart wakeup mode
				detectMode = DETECT_MODE_WAKEUP;
				rfalFieldOff(); /* Turns the Field On and starts GT timer */
				k_sleep(K_MSEC(100));
				rfalWakeUpModeStart(NULL);
			}

			gState = RFAL_POLLER_STATE_INIT; /* Restart the loop */
			break;

			/*******************************************************************************/
		default:
			return;
		}
		k_sleep(K_MSEC(10));
	}
}

bool st25r_getStatus(void)
{
	return gUIDStatus;
}

char* st25r_getUIDStr(void)
{
	return gUIDStr;
}

void st25r_setStatus(bool status)
{
	gUIDStatus = status;
}

void st25r_mutex_lock(void)
{
  k_mutex_lock(&st25mutex, K_FOREVER);
}

void st25r_mutex_unlock(void)
{
  k_mutex_unlock(&st25mutex);
}

void st25r_lowlayer_mutex_lock(void)
{
  k_mutex_lock(&st25lowmutex, K_FOREVER);
}

void st25r_lowlayer_mutex_unlock(void)
{
  k_mutex_unlock(&st25lowmutex);
}
