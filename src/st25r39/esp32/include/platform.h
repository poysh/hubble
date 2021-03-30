
/******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/myliberty
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/
/*! \file
 *
 *  \author 
 *
 *  \brief Platform header file. Defining platform independent functionality.
 *
 */


/*
 *      PROJECT:   
 *      $Revision: $
 *      LANGUAGE:  ISO C99
 */

/*! \file platform.h
 *
 *  \author Gustavo Patricio
 *
 *  \brief Platform specific definition layer  
 *  
 *  This should contain all platform and hardware specifics such as 
 *  GPIO assignment, system resources, locks, IRQs, etc
 *  
 *  Each distinct platform/system/board must provide this definitions 
 *  for all SW layers to use
 *  
 */

#ifndef PLATFORM_H
#define PLATFORM_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include <zephyr.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>

#include "st_errno.h"

#include "spi_driver.h"
#include "io_driver.h"
#include "hal_timer.h"
#include "esp32_config.h"
#include "esp32_app.h"
/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/
#define RFAL_ANALOG_CONFIG_CUSTOM
//#define ST25R391X_COM_SINGLETXRX

#define ST25R391X_SS_PIN                              ST25_SPI_SS_PIN
#define ST25R391X_SS_PORT															ST25_SPI_SS_PIN
#ifndef _NUCLEO_
	#define ST25R391X_INT_PIN           	              ST25_IRQ_PIN
	#define ST25R391X_INT_PORT													ST25_IRQ_PIN


	#define ST25R3916_INT_PIN           ST25DX_INT_1_Pin        /*!< GPIO pin used for ST25R3911 External Interrupt    */
	#define ST25R3916_INT_PORT          ST25DX_INT_1_Pin        /*!< GPIO port used for ST25R3911 External Interrupt   */

#else
	#define ST25R391X_INT_PIN           	            IRQ_3916_Pin
	#define ST25R391X_INT_PORT          	            IRQ_3916_GPIO_Port
#endif



#ifdef LED_FIELD_Pin
#define PLATFORM_LED_FIELD_PIN                      LED_FIELD_Pin       /*!< GPIO pin used as field LED                        */
#endif

#ifdef LED_FIELD_GPIO_Port
#define PLATFORM_LED_FIELD_PORT                     LED_FIELD_GPIO_Port /*!< GPIO port used as field LED                       */
#endif


/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/

#define platformIrqST25R3916SetCallback( cb )          
#define platformIrqST25R3916PinInitialize()

extern void st25r_mutex_lock(void);
extern void st25r_mutex_unlock(void);

#define platformProtectST25R391xComm()                do{ globalCommProtectCnt++; st25r_mutex_lock(); }while(0) /*!< Protect unique access to ST25R391x communication channel - IRQ disable on single thread environment (MCU) ; Mutex lock on a multi thread environment      */
#define platformUnprotectST25R391xComm()              do{ --globalCommProtectCnt; st25r_mutex_unlock();}while(0)                /*!< Unprotect unique access to ST25R391x communication channel - IRQ enable on a single thread environment (MCU) ; Mutex unlock on a multi thread environment */

#define platformProtectST25R391xIrqStatus()           platformProtectST25R391xComm()
#define platformUnprotectST25R391xIrqStatus()         platformUnprotectST25R391xComm()

#define platformProtectWorker()                       do {} while(0)                   /* Protect RFAL Worker/Task/Process from concurrent execution on multi thread platforms   */
#define platformUnprotectWorker()                     do {} while(0)                 /* Unprotect RFAL Worker/Task/Process from concurrent execution on multi thread platforms */


#define platformSpiSelect()                           spi_driver_cs_set();
#define platformSpiDeselect()                         spi_driver_cs_clear();

#define platformLedsInitialize()                                                                    /*!< Initializes the pins used as LEDs to outputs*/

#define platformLedOff( port, pin )                   platformGpioClear(port, pin)
#define platformLedOn( port, pin )                    platformGpioSet(port, pin)
#define platformLedToogle( port, pin )                platformGpioToogle(port, pin)

#define platformGpioSet( port, pin )                  do {} while(0)                    /*!< Turns the given GPIO High                   */
#define platformGpioClear( port, pin )                do {} while(0)                   /*!< Turns the given GPIO Low                    */
#define platformGpioToogle( port, pin )               do {} while(0)                   /*!< Toogles the given GPIO                      */
#define platformGpioIsHigh( port, pin )               (io_driver_get_pin() == 1U)    /*!< Checks if the given LED is High             */
#define platformGpioIsLow( port, pin )                (!platformGpioIsHigh(port, pin))              /*!< Checks if the given LED is Low              */

#define platformTimerCreate( t )                      timerCalculateTimer(t)                        /*!< Create a timer with the given time (ms)     */
#define platformTimerIsExpired( timer )               timerIsExpired(timer)                         /*!< Checks if the given timer is expired        */
#define platformDelay( t )                            k_sleep( K_MSEC(t))                                /*!< Performs a delay for the given time (ms)    */

#define platformGetSysTick()                          k_cycle_get_32()                                 /*!< Get System Tick ( 1 tick = 1 ms)            */

#define platformSpiTxRx( txBuf, rxBuf, len )          spi_driver_transfer(txBuf, rxBuf, len)                    /*!< SPI transceive                              */


#define platformI2CTx( txBuf, len, last, txOnly )                                                   /*!< I2C Transmit                                */
#define platformI2CRx( txBuf, len )                                                                 /*!< I2C Receive                                 */
#define platformI2CStart()                                                                          /*!< I2C Start condition                         */
#define platformI2CStop()                                                                           /*!< I2C Stop condition                          */
#define platformI2CRepeatStart()                                                                    /*!< I2C Repeat Start                            */
#define platformI2CSlaveAddrWR(add)                                                                 /*!< I2C Slave address for Write operation       */
#define platformI2CSlaveAddrRD(add)                                                                 /*!< I2C Slave address for Read operation        */

#define platformLog(...)                                                                         /*!< Log method                                  */


/*
******************************************************************************
* GLOBAL VARIABLES
******************************************************************************
*/
extern uint8_t globalCommProtectCnt;                      /* Global Protection Counter provided per platform - instanciated in main.c    */

/*
******************************************************************************
* RFAL FEATURES CONFIGURATION
******************************************************************************
*/

#define RFAL_FEATURE_LISTEN_MODE               1                   /*!< Enable/Disable RFAL support for Listen Mode                               */
#define RFAL_FEATURE_WAKEUP_MODE               1                   /*!< Enable/Disable RFAL support for the Wake-Up mode                          */
#define RFAL_FEATURE_NFCA                      1                   /*!< Enable/Disable RFAL support for NFC-A (ISO14443A)                         */
#define RFAL_FEATURE_NFCB                      1                   /*!< Enable/Disable RFAL support for NFC-B (ISO14443B)                         */
#define RFAL_FEATURE_NFCF                      1                   /*!< Enable/Disable RFAL support for NFC-F (FeliCa)                            */
#define RFAL_FEATURE_NFCV                      1                   /*!< Enable/Disable RFAL support for NFC-V (ISO15693)                          */
#define RFAL_FEATURE_T1T                       1                   /*!< Enable/Disable RFAL support for T1T (Topaz)                               */
#define RFAL_FEATURE_ST25TB                    1                   /*!< Enable/Disable RFAL support for ST25TB                                    */
#define RFAL_FEATURE_DYNAMIC_ANALOG_CONFIG     1                   /*!< Enable/Disable Analog Configs to be dynamically updated (RAM)             */
#define RFAL_FEATURE_DPO                       1                   /*!< Enable/Disable RFAL dynamic power support                                 */
#define RFAL_FEATURE_ISO_DEP                   1                   /*!< Enable/Disable RFAL support for ISO-DEP (ISO14443-4)                      */
#define RFAL_FEATURE_ISO_DEP_POLL              1                   /*!< Enable/Disable RFAL support for Poller mode (PCD) ISO-DEP (ISO14443-4)    */
#define RFAL_FEATURE_ISO_DEP_LISTEN            1                   /*!< Enable/Disable RFAL support for Listen mode (PICC) ISO-DEP (ISO14443-4)   */
#define RFAL_FEATURE_NFC_DEP                   1                   /*!< Enable/Disable RFAL support for NFC-DEP (NFCIP1/P2P)                      */


#define RFAL_FEATURE_ISO_DEP_IBLOCK_MAX_LEN    256                    /*!< ISO-DEP I-Block max length. Please use values as defined by rfalIsoDepFSx */
#define RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN      1024                   /*!< ISO-DEP APDU max length. Please use multiples of I-Block max length       */

#endif /* PLATFORM_H */


