#include <zephyr.h>
#include <drivers/gpio.h>
#include "esp32_config.h"
#include "st25r3916_irq.h"
#include <logging/log.h>
LOG_MODULE_REGISTER(io_driver);

static struct device* irq_dev;

static struct gpio_callback irq_cb_data;

void irq_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    st25r3916Isr();
}

int io_driver_init(void)
{
    int ret = 0;
    irq_dev = (struct device* )device_get_binding("GPIO_0");
	if (!irq_dev) {
		LOG_ERR("Cannot find GPIO device %s!", "GPIO_0");
		return -ENXIO;
	}

	ret = gpio_pin_configure(irq_dev, ST25_IRQ_PIN, GPIO_INT_EDGE_RISING);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, "GPIO_0", ST25_IRQ_PIN);
		return ret;
	}

	ret = gpio_pin_interrupt_configure(irq_dev, ST25_IRQ_PIN, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, "GPIO_0", ST25_IRQ_PIN);
		return ret;
	}

	gpio_init_callback(&irq_cb_data, irq_handler, BIT(ST25_IRQ_PIN));
	gpio_add_callback(irq_dev, &irq_cb_data);
    
    LOG_INF("Set up button at %s pin %d", "GPIO_0", ST25_IRQ_PIN);
    return 0;
}

int io_driver_get_pin(void)
{
    return gpio_pin_get(irq_dev, ST25_IRQ_PIN);
}
