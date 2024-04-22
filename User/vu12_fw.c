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
#include "tass805m.h"
#include "eeprom.h"
#include "adc_key.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t DigitalVolume, AnalogVolume, Brightness;
uint32_t MillisCheck = 0;
uint8_t HDMI_Signal = eSTATUS_NO_SIGNAL;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void blink_status_led (void)
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
void watchdog_setup (uint16_t reload)
{
    // 40Khz(LSI Clk) / Prescaler / Reloadcounter (Decrement)
    // 3.2s IWDG reset T = 4000 / (40000 / 32 = 1250)
    IWDG_WriteAccessCmd (IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler (IWDG_Prescaler_32);
    IWDG_SetReload (reload);
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
void setup() {
    port_init ();

    // Wait serial port ready.
    USBSerial_print ("\r\n*** BOOT ***\r\n");
    // F/W & Debug info display
    USBSerial_print ("DEBUG_UART : PORT = %d, BUAD = %d\r\n",
        _DEBUG_UART_PORT_, _DEBUG_UART_BAUD_);
    USBSerial_print ("FW_VERSION : %s\r\n", _FW_VERSION_STR_);
    USBSerial_print ("Build Date = "__DATE__" " __TIME__ "\r\n");

    // get platform save data from eeprom
    // DigitalVolume, AnalogVolume, Brigntness
    eeprom_init (false);

    // codec init
    tass805m_init ();

    // hdmi2lvds init
    lt8619c_init ();

    HDMI_Signal = lt8619c_loop() > 0 ?
                    eSTATUS_SIGNAL_DETECT : eSTATUS_NO_SIGNAL;
    printf ("Boot HDMI_Signal = %d\r\n", HDMI_Signal);

    // backlight (94Khz init), Default brightness = OFF
    backlight_init (PORT_BACKLIGHT_PWM);

    // ADC key init : adc_key_init (uint16_t adc_port, uint16_t ref_volt)
    adc_key_init (PORT_ADC_KEY, 3300);

    /* VU12 System watchdog enable */
    watchdog_setup (WDT_RELOAD_3_2_S);

    /* Protocol F/W Start */
    USBSerial_println ("");
    USBSerial_print ("@S-OK#\r\n");
}

/*---------------------------------------------------------------------------*/
void loop() {
    /* system watchdog */
    WDT_CLR();

    /* serial data read & write check */
    if (USBSerial_available())
        protocol_data_check();

    /* lt8619c check loop (1 sec) */
    if (MillisCheck + PERIOD_LT8619C_LOOP < millis()) {
        // lt8619 status check
        if (!lt8619c_loop()) {
            backlight_control (0);  tass805m_mute();
            HDMI_Signal = eSTATUS_NO_SIGNAL;
            blink_status_led ();
        } else
            digitalWrite (PORT_ALIVE_LED, LOW);

        switch (HDMI_Signal) {
            case     eSTATUS_NO_SIGNAL:
                HDMI_Signal = (HDMI_Signal == eSTATUS_NO_SIGNAL) ?
                        eSTATUS_SIGNAL_DETECT : eSTATUS_SIGNAL_STABLE;
                break;
            case eSTATUS_SIGNAL_DETECT:
                lt8619c_init ();
                HDMI_Signal = eSTATUS_AUDIO_INIT;
                break;
            case eSTATUS_AUDIO_INIT:
                tass805m_write (CODEC_REG_DGAIN, &DigitalVolume);
                HDMI_Signal = eSTATUS_BACKLIGHT_INIT;
                break;
            case eSTATUS_BACKLIGHT_INIT:
                backlight_control (Brightness);
                HDMI_Signal = eSTATUS_SIGNAL_STABLE;
                break;
            default :
                break;
        }
        MillisCheck = millis ();
    }
    /* adc key process */
    adc_key_loop ();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
