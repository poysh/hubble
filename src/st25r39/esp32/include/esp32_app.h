#ifndef ESP32_APP_H_
#define ESP32_APP_H_

#include "rfal_analogConfig.h"
#include "rfal_rf.h"
#include "rfal_dpo.h"
#include "rfal_chip.h"

void st25r_init(void);
void st25r_initIrq(void);
bool st25r_getStatus(void);
char* st25r_getUIDStr(void);
void st25r_setStatus(bool status);
void st25r_initRFAL(void);
void rfalPollerRun(void);

#endif /* ESP32_APP_H_ */
