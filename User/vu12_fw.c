/*---------------------------------------------------------------------------*/
/**
 * @file vu12_fw.ino
 * @author charles-park (charles.park@hardkernel.com)
 * @brief VU12_HDMI2LVDS (1920x720) Display Project (CH32V203 RISC-V)
 * @version 0.1
 * @date 2024-02-14
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "Arduino.h"
#include "vu12_fw.h"

/*---------------------------------------------------------------------------*/
#include "protocol.h"
#include "backlight.h"
#include "gpio_i2c.h"
#include "lt8619c.h"
#include "eeprom.h"
#include "adc_key.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t DigitalVolume, AnalogVolume, Brightness;
uint32_t MillisCheck = 0;
uint8_t HDMI_Signal = 0;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void alive_led (void)
{
    digitalWrite (PORT_ALIVE_LED, !digitalRead(PORT_ALIVE_LED));
}

/*---------------------------------------------------------------------------*/
void touch_reset (uint8_t d)
{
    digitalWrite (PORT_TOUCH_RESET, LOW);
    delay (d + 50);
    digitalWrite (PORT_TOUCH_RESET, HIGH);
}

/*---------------------------------------------------------------------------*/
void watchdog_setup (void)
{
    // 40Khz(LSI Clk) / Prescaler / Reloadcounter (Decrement)
    // 3.2s IWDG reset
    IWDG_WriteAccessCmd (IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler (IWDG_Prescaler_32);
    IWDG_SetReload (4000);
    IWDG_ReloadCounter ();
    IWDG_Enable ();
}

/*---------------------------------------------------------------------------*/
void port_init (void)
{
    // Board Alive led init
    pinMode (PORT_ALIVE_LED, OUTPUT);   digitalWrite (PORT_ALIVE_LED, LOW);
    // lcd reset
    pinMode (PORT_LCD_RESET, OUTPUT);   digitalWrite (PORT_LCD_RESET, LOW);
    // lcd stdby
    pinMode (PORT_LCD_STDBY, OUTPUT);   digitalWrite (PORT_LCD_STDBY, LOW);

    // lcd init timing
    delay(10);  digitalWrite (PORT_LCD_RESET, HIGH);
    delay(50);  digitalWrite (PORT_LCD_STDBY, HIGH);
    delay(150);

    // codec
    pinMode (PORT_CODEC_PWREN, OUTPUT); digitalWrite (PORT_CODEC_PWREN, LOW);
    // touch
    pinMode (PORT_TOUCH_RESET, OUTPUT); touch_reset (100);
    // pwm
    pinMode (PORT_BACKLIGHT_PWM, OUTPUT);
    digitalWrite (PORT_BACKLIGHT_PWM, LOW);

    // I2C Port init (GPIO Bit-bang)
    gpio_i2c_init (PORT_I2C_SCL, PORT_I2C_SDA);
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
    i2c_send (I2C_ADDR_CODEC, CODEC_REG_DEVICE_CTRL, &wd, 1);   delay(10);

    // set volume data
    i2c_send (I2C_ADDR_CODEC, CODEC_REG_DGAIN, &DigitalVolume, 1);
    i2c_send (I2C_ADDR_CODEC, CODEC_REG_AGAIN, &AnalogVolume,  1);

    // codec change mode hi-z to play mode
    // changing modes requires a 10ms delay.
    wd = STATE_PLAY;
    i2c_send (I2C_ADDR_CODEC, CODEC_REG_DEVICE_CTRL, &wd, 1);   delay(10);
}

/*---------------------------------------------------------------------------*/
void key_init (void)
{
//    adc_key_add  (key num(0~9), key_event_code (1~255), key_adc_max_mv, key_adc_min_volt);
    adc_key_init (PORT_ADC_KEY, 3300);
    adc_key_add  (0, EVENT_D_VOL_UP, 3300, 3200);   // 3300
    adc_key_add  (1, EVENT_A_VOL_UP, 3100, 2900);   // 3000
    adc_key_add  (2, EVENT_B_VAL_UP, 2800, 2600);   // 2700
    // touch reset
    adc_key_add  (3, EVENT_T_RESET , 2400, 2200);   // 2320

    adc_key_add  (4, EVENT_D_VOL_DN, 2100, 1900);   // 1980
    adc_key_add  (5, EVENT_A_VOL_DN, 1700, 1500);   // 1650
    adc_key_add  (6, EVENT_B_VAL_DN, 1400, 1200);   // 1320
    // system reboot
    adc_key_add  (7, EVENT_S_RESET , 1100, 900);    // 1000
}

/*---------------------------------------------------------------------------*/
void setup() {
    port_init ();

    // Wait serial port ready.
    delay(1000);

    /* USB Serial data init, boot msg send */
    USBSerial_println(__DATE__" " __TIME__);
    USBSerial_println("@S-OK#");

    // get platform save data from eeprom
    // DigitalVolume, AnalogVolume, Brigntness
    eeprom_init (false);

    // codec init
    tass805m_init ();

    // hdmi2lvds init
    lt8619c_init ();

    // backlight (94Khz init)
    backlight_init (PORT_BACKLIGHT_PWM);

    // ADC key init
    key_init();

    /* VU12 System watchdog enable */
    watchdog_setup ();
}

/*---------------------------------------------------------------------------*/
void loop() {
    /* system watchdog */
    WDT_CLR();

    /* serial data check */
    if (USBSerial_available())
        protocol_data_check();

    /* lt8619c check loop (1 sec) */
    if (MillisCheck + PERIOD_LT8619C_LOOP < millis()) {
        if (!lt8619c_loop()) {
            backlight_control (0);  HDMI_Signal = 0;
            alive_led ();
        } else {
            if (HDMI_Signal > HDMI_SIGNAL_STABLE)
                backlight_control (Brightness);
            else {
                if (!HDMI_Signal)
                    lt8619c_init ();

                HDMI_Signal++;
            }
            digitalWrite (PORT_ALIVE_LED, LOW);
        }
        MillisCheck = millis ();

        // gpio i2c test (weather board 2, bme150 rev read)
        {
            uint8_t rev = 0;
            if (i2c_read (0x76<<1, 0xD0, &rev, 1))
                printf ("MillisCheck = %d, bme150 rev = 0x%02x\r\n", MillisCheck, rev);
        }
    }
    /* adc key process */
    adc_key_loop ();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
