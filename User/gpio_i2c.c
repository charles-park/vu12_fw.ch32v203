/*---------------------------------------------------------------------------*/
/**
 * @file gpio_i2c.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief CH552 GPIO I2C (Bitbang)
 * @version 0.1
 * @date 2024-02-20
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "Arduino.h"
#include "gpio_i2c.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
//#define GPIO_SET_DELAY  50      // micro seconds (I2C 10Khz)
#define GPIO_SET_DELAY  3      // micro seconds (I2C 200Khz)

/*---------------------------------------------------------------------------*/
// gpio low level control func
void gpio_i2c_start     (bool restart);
void gpio_i2c_stop      (void);
bool gpio_i2c_ack       (void);

bool i2c_send_bits      (uint8_t d);
uint8_t i2c_read_bits   (void);

/*---------------------------------------------------------------------------*/
// return write count
uint16_t i2c_send        (uint8_t addr, uint8_t reg, uint8_t *d, uint16_t size);
// return read count
uint16_t i2c_read        (uint8_t addr, uint8_t reg, uint8_t *d, uint16_t size);

/*---------------------------------------------------------------------------*/
uint8_t gpio_i2c_init   (uint8_t gpio_scl, uint8_t gpio_sda);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t GPIO_I2C_SCL, GPIO_I2C_SDA;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t gpio_i2c_init   (uint8_t gpio_scl, uint8_t gpio_sda)
{
    if (!gpio_scl || !gpio_sda) return 0;

    GPIO_I2C_SCL = gpio_scl, GPIO_I2C_SDA = gpio_sda;

    pinMode (GPIO_I2C_SCL, OUTPUT_OD);
    pinMode (GPIO_I2C_SDA, OUTPUT_OD);

    digitalWrite (GPIO_I2C_SCL, HIGH);  udelay(GPIO_SET_DELAY);
    digitalWrite (GPIO_I2C_SDA, HIGH);  udelay(GPIO_SET_DELAY);

    return 1;
}

/*---------------------------------------------------------------------------*/
void gpio_i2c_start     (bool restart)
{
    digitalWrite (GPIO_I2C_SDA, LOW);   udelay(GPIO_SET_DELAY);
    digitalWrite (GPIO_I2C_SCL, LOW);   udelay(GPIO_SET_DELAY);
    if (restart) {
        digitalWrite (GPIO_I2C_SDA, HIGH);  udelay(GPIO_SET_DELAY);
        digitalWrite (GPIO_I2C_SCL, HIGH);  udelay(GPIO_SET_DELAY);
        digitalWrite (GPIO_I2C_SDA, LOW);   udelay(GPIO_SET_DELAY);
        digitalWrite (GPIO_I2C_SCL, LOW);   udelay(GPIO_SET_DELAY);
    }
}

/*---------------------------------------------------------------------------*/
void gpio_i2c_stop      (void)
{
    digitalWrite (GPIO_I2C_SCL, HIGH);  udelay(GPIO_SET_DELAY);
    digitalWrite (GPIO_I2C_SDA, HIGH);  udelay(GPIO_SET_DELAY);
}

/*---------------------------------------------------------------------------*/
bool i2c_send_bits      (uint8_t wd)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        digitalWrite (GPIO_I2C_SDA, (wd & 0x80) ? HIGH : LOW);
        wd <<= 1;
        digitalWrite (GPIO_I2C_SCL, HIGH);  udelay(GPIO_SET_DELAY);
        digitalWrite (GPIO_I2C_SCL, LOW);   udelay(GPIO_SET_DELAY);
    }
    // ack check
    digitalWrite (GPIO_I2C_SCL, HIGH);  udelay(GPIO_SET_DELAY);
    i = digitalRead (GPIO_I2C_SDA);
    digitalWrite (GPIO_I2C_SCL, LOW);   udelay(GPIO_SET_DELAY);

    return i;
}

/*---------------------------------------------------------------------------*/
uint8_t i2c_read_bits   (void)
{
    uint8_t i, rd;

    for (i = 0, rd = 0; i < 8; i++) {
        digitalWrite (GPIO_I2C_SCL, HIGH);  udelay(GPIO_SET_DELAY);
        rd <<= 1;
        rd |= digitalRead (GPIO_I2C_SDA) ? 1 : 0;
        digitalWrite (GPIO_I2C_SCL, LOW);   udelay(GPIO_SET_DELAY);
    }
    return rd;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint16_t i2c_send    (uint8_t addr, uint8_t reg, uint8_t *d, uint16_t size)
{
    uint16_t i = 0;

    gpio_i2c_start (0);
    if (i2c_send_bits (addr))   goto wr_out;
    if (i2c_send_bits (reg))    goto wr_out;

    for (i = 0; i < size; i++)
        if (i2c_send_bits (d[i]))   goto wr_out;

wr_out:
    gpio_i2c_stop  ();

#if defined (_DEBUG_GPIO_I2C_)
    if (i != size) {
        printf ("%s(error) : addr = 0x%02X, reg = 0x%02X, size = %d\r\n", addr, reg, size);
        USBSerial_print ("%s(error) : i2c_addr = 0x%02X\r\n", addr);
    }
#endif
    return i;
}

/*---------------------------------------------------------------------------*/
uint16_t i2c_read    (uint8_t addr, uint8_t reg, uint8_t *d, uint16_t size)
{
    uint16_t i = 0;

    gpio_i2c_start (0);
    if (i2c_send_bits (addr))   goto rd_out;
    if (i2c_send_bits (reg))    goto rd_out;

    // Read
    gpio_i2c_start (1);
    if (i2c_send_bits (addr | I2C_READ_FLAG))   goto rd_out;

    for (i = 0; i < size; i++) {
        d[i] = i2c_read_bits ();
        // ack send except last byte.
        if (i < (size -1)) {
            digitalWrite (GPIO_I2C_SDA, LOW);   udelay(GPIO_SET_DELAY);
            digitalWrite (GPIO_I2C_SCL, HIGH);  udelay(GPIO_SET_DELAY);
            digitalWrite (GPIO_I2C_SCL, LOW);   udelay(GPIO_SET_DELAY);
            digitalWrite (GPIO_I2C_SDA, HIGH);  udelay(GPIO_SET_DELAY);
        }
    }
rd_out:
    gpio_i2c_stop  ();

#if defined (_DEBUG_GPIO_I2C_)
    if (i != size) {
        printf ("%s(error) : addr = 0x%02X, reg = 0x%02X, size = %d\r\n", addr, reg, size);
        USBSerial_print ("%s(error) : i2c_addr = 0x%02X\r\n", addr);
    }
#endif
    return i;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
