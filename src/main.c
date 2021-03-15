#include <errno.h>
#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/spi.h>
#include <drivers/gpio.h>
#include "../st25r3911b/st25r3911b_nfca.h"

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(main);

static struct k_poll_event events[ST25R3911B_NFCA_EVENT_CNT];

void main(void)
{
	int err = 0;

	err = st25r3911b_nfca_init(events, ARRAY_SIZE(events), NULL);
	if (err) {
		printk("NFCA initialization failed err: %d.\n", err);
		return;
	}

	err = st25r3911b_nfca_field_on();
	if (err) {
		printk("Field on error %d.", err);
		return;
	}

	while (true) {
		k_poll(events, ARRAY_SIZE(events), K_FOREVER);
		err = st25r3911b_nfca_process();
		if (err) {
			printk("NFC-A process failed, err: %d.\n", err);
			return;
		}
	}
}
