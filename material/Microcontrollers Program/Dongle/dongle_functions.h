#ifndef DONGLE_FUNCTIONS_H_
#define DONGLE_FUNCTIONS_H_

#include <jendefs.h>
#include <AppHardwareApi.h>
#include <PeripheralRegs.h>
#include "Config.h"
#include <dbg.h>
#include <dbg_uart.h>

/* Timer data */
#define DONGLE_TICK_FREQUENCY_HZ		1000UL
#define DONGLE_TICK_PERIOD_MS         	1000UL / DONGLE_TICK_FREQUENCY_HZ	/**< Timer period */
#define DONGLE_TICK_PERIOD_COUNT     	(16000UL * DONGLE_TICK_PERIOD_MS)


void vDongleInitDIOs();
void vDongleSetLED(bool state);
void vDongleInitTickTimer(void);

#endif /* DONGLE_FUNCTIONS_H_ */
