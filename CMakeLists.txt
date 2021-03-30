# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

target_sources(app PRIVATE src/main.c
  src/st25r39/lib_nfc/lib_NDEF/src/lib_NDEF_AAR.c 
  src/st25r39/lib_nfc/lib_NDEF/src/lib_NDEF_Bluetooth.c 
  src/st25r39/lib_nfc/lib_NDEF/src/lib_NDEF_Email.c 
  src/st25r39/lib_nfc/lib_NDEF/src/lib_NDEF_Geo.c 
  src/st25r39/lib_nfc/lib_NDEF/src/lib_NDEF_Handover.c 
  src/st25r39/lib_nfc/lib_NDEF/src/lib_NDEF_MyApp.c 
  src/st25r39/lib_nfc/lib_NDEF/src/lib_NDEF_SMS.c 
  src/st25r39/lib_nfc/lib_NDEF/src/lib_NDEF_Text.c 
  src/st25r39/lib_nfc/lib_NDEF/src/lib_NDEF_URI.c 
  src/st25r39/lib_nfc/lib_NDEF/src/lib_NDEF_Vcard.c 
  src/st25r39/lib_nfc/lib_NDEF/src/lib_NDEF_Wifi.c 
  src/st25r39/lib_nfc/lib_NDEF/src/lib_NDEF.c 
  src/st25r39/lib_nfc/lib_NDEF/src/lib_wrapper.c 
  src/st25r39/lib_nfc/lib_NDEF/src/tagtype3_wrapper.c 
  src/st25r39/lib_nfc/lib_NDEF/src/tagtype4_wrapper.c 
  src/st25r39/lib_nfc/lib_NDEF/src/tagtype5_wrapper.c 
  src/st25r39/esp32/src/hal_timer.c 
  src/st25r39/esp32/src/esp32_app.c 
  src/st25r39/esp32/src/esp32_st25r.c 
  src/st25r39/esp32/src/rfal_analog_config_custom_mb1414_b_v1.c 
  src/st25r39/RFAL/source/st25r3916/rfal_rfst25r3916.c 
  src/st25r39/RFAL/source/st25r3916/st25r3916_aat.c 
  src/st25r39/RFAL/source/st25r3916/st25r3916_com.c 
  src/st25r39/RFAL/source/st25r3916/st25r3916_irq.c 
  src/st25r39/RFAL/source/st25r3916/st25r3916_led.c 
  src/st25r39/RFAL/source/st25r3916/st25r3916.c 
  src/st25r39/RFAL/source/rfal_analogConfig.c 
  src/st25r39/RFAL/source/rfal_crc.c 
  src/st25r39/RFAL/source/rfal_dpo.c 
  src/st25r39/RFAL/source/rfal_iso15693_2.c 
  src/st25r39/RFAL/source/rfal_isoDep.c 
  src/st25r39/RFAL/source/rfal_nfca.c 
  src/st25r39/RFAL/source/rfal_nfcb.c 
  src/st25r39/RFAL/source/rfal_nfcDep.c 
  src/st25r39/RFAL/source/rfal_nfcf.c 
  src/st25r39/RFAL/source/rfal_nfcv.c 
  src/st25r39/RFAL/source/rfal_st25tb.c 
  src/st25r39/RFAL/source/rfal_t1t.c 
  src/st25r39/utils/source/st_error.c 
  src/hal/io_driver.c 
  src/hal/spi_driver.c
)

zephyr_include_directories(
  src/st25r39/lib_nfc/common/inc
  src/st25r39/lib_nfc/lib_NDEF/inc 
  src/st25r39/esp32/include 
  src/st25r39/RFAL/include 
  src/st25r39/RFAL/source/st25r3916 
  src/st25r39/RFAL/source 
  src/st25r39/utils/include 
  src/hal
)