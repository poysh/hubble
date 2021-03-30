#include <zephyr.h>
#include <drivers/spi.h>
#include <drivers/gpio.h>
#include "esp32_config.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(spi_driver);

/* SPI hardware configuration. */
static const struct spi_config spi_cfg =  {
	.frequency = 10000000,
	.operation = (SPI_OP_MODE_MASTER | SPI_WORD_SET(8) |
		      SPI_TRANSFER_MSB | SPI_LINES_SINGLE |
		      SPI_MODE_CPHA),
	.cs = NULL
};

static struct device* spi_dev;
static struct device* cs_dev;

int spi_driver_init(void)
{
	spi_dev = (struct device*)device_get_binding("SPI_3");
	if (!spi_dev) {
		LOG_ERR("Cannot find SPI device %s!", "SPI_3");
		return -ENXIO;
	}

	cs_dev = (struct device*)device_get_binding("GPIO_0");
	if (!spi_dev) {
		LOG_ERR("Cannot find GPIO device %s!", "GPIO_0");
		return -ENXIO;
	}

    gpio_pin_configure(cs_dev, ST25_SPI_SS_PIN, GPIO_OUTPUT_HIGH);

    return 0;
}

void spi_driver_transfer(uint8_t* txbuf, uint8_t* rxbuf, uint8_t length)
{
	const struct spi_buf tx_buf = {
		.buf = txbuf,
		.len = (txbuf != NULL ? length : 0)
	};
	const struct spi_buf_set tx = {
		.buffers = &tx_buf,
		.count = (txbuf != NULL ? length : 0)
	};

	/* Read register value. */
	const struct spi_buf rx_bufs = {
		.buf = rxbuf,
		.len = (rxbuf != NULL ? length : 0)
	};

	const struct spi_buf_set rx = {
		.buffers = &rx_bufs,
		.count = (rxbuf != NULL ? 1 : 0)
	};

	int err = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);
	if (err) {
		LOG_ERR("SPI reg read failed, err: %d.", err);
		return;
	}
}

void spi_driver_cs_set(void)
{
    gpio_pin_set(cs_dev, ST25_SPI_SS_PIN, 0U);
}

void spi_driver_cs_clear(void)
{
    gpio_pin_set(cs_dev, ST25_SPI_SS_PIN, 1U);
}
