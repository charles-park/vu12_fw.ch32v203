/*---------------------------------------------------------------------------*/
/**
 * @file tass805m.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief VU12 codec(tass805m) control
 * @version 0.1
 * @date 2024-04-16
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "Arduino.h"
#include "vu12_fw.h"
#include "gpio_i2c.h"
#include "tass805m.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint16_t tass805m_write (uint8_t reg, uint8_t *value)
{
    uint16_t ret = i2c_send (I2C_ADDR_CODEC, reg, value, 1);
#if defined (_DEBUG_TASS805M_)
    printf ("%s(%s) : reg = 0x%02X, value = %d\r\n",
        __func__, (ret == 1) ? "Success" : "Error", reg, *value);
#endif
    return ret;
}

/*---------------------------------------------------------------------------*/
uint16_t tass805m_read (uint8_t reg, uint8_t *value)
{
    uint16_t ret = i2c_read (I2C_ADDR_CODEC, reg, value, 1);
#if defined (_DEBUG_TASS805M_)
    printf ("%s(%s) : reg = 0x%02X, value = %d\r\n",
        __func__, (ret == 1) ? "Success" : "Error", reg, *value);
#endif
    return ret;
}

/*---------------------------------------------------------------------------*/
void tass805m_init (void)
{
    uint8_t wd;
    // codec change mode powerdown(disable) to deep sleep mode.
    // changing modes requires a 10ms delay.
    digitalWrite (PORT_CODEC_PWREN, HIGH);  delay(10);

    // codec change mode deep sleep to hi-z mode (DSP enable)
    // changing modes requires a 10ms delay.
    wd = STATE_HI_Z;
    tass805m_write (CODEC_REG_DEVICE_CTRL, &wd);   delay(10);

    // set volume data
    tass805m_write (CODEC_REG_DGAIN, &DigitalVolume);
    tass805m_write (CODEC_REG_AGAIN, &AnalogVolume);

    // codec change mode hi-z to play mode
    // changing modes requires a 10ms delay.
    wd = STATE_PLAY;
    tass805m_write (CODEC_REG_DEVICE_CTRL, &wd);   delay(10);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
