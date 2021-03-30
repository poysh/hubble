#ifndef SPI_DRIVER_H_
#define SPI_DRIVER_H_

int spi_driver_init(void);
void spi_driver_transfer(uint8_t* txbuf, uint8_t* rxbuf, uint8_t length);
void spi_driver_cs_set(void);
void spi_driver_cs_clear(void);

#endif /* SPI_DRIVER_H_ */