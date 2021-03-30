#include <errno.h>
#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/spi.h>
#include <drivers/gpio.h>
#include "esp32_app.h"
#include "rfal_rf.h"
#include "io_driver.h"
#include "spi_driver.h"

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(main);

void main(void)
{
	spi_driver_init();
	st25r_initRFAL();
	while(1)
	{
		rfalPollerRun();
		rfalWorker();
	}
}
