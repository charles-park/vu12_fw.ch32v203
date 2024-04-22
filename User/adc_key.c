/*---------------------------------------------------------------------------*/
/**
 * @file adc_key.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief CH552 ADC Key
 * @version 0.1
 * @date 2024-02-15
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "Arduino.h"
#include "vu12_fw.h"
#include <string.h>

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
uint8_t  adc_key_add    (uint16_t num, uint16_t key_code, uint16_t max_mv, uint16_t min_mv);
uint8_t  adc_key_remove (uint16_t num);
uint8_t  adc_key_repeat (uint16_t num, uint16_t repeat_ms);
uint16_t adc_key_read   (bool b_clr);
void     adc_key_check  (void);
void     adc_key_loop   (void);
uint8_t  adc_key_init   (uint16_t adc_port, uint16_t ref_volt);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
struct adc_key_grp  KeyGrp;

uint64_t MillisCheckADC = 0;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t adc_key_init    (uint16_t adc_port, uint16_t ref_volt)
{
    KeyGrp.adc_port = adc_port; KeyGrp.ref_volt = ref_volt;
    KeyGrp.event    = 0;
    pinMode(adc_port, INPUT_ANALOG);

 // adc_key_add  (key num(0~9), key_event_code (1~255), key_adc_max_mv, key_adc_min_volt);
    adc_key_add (0, EVENT_D_VOL_UP, 3300, 3200);   // 3300
    adc_key_add (1, EVENT_A_VOL_UP, 3100, 2900);   // 3000
    adc_key_add (2, EVENT_B_VAL_UP, 2800, 2600);   // 2700
    // touch reset
    adc_key_add (3, EVENT_T_RESET , 2400, 2200);   // 2320

    adc_key_add (4, EVENT_D_VOL_DN, 2100, 1900);   // 1980
    adc_key_add (5, EVENT_A_VOL_DN, 1700, 1500);   // 1650
    adc_key_add (6, EVENT_B_VAL_DN, 1400, 1200);   // 1320
    // system reboot
    adc_key_add (7, EVENT_S_RESET , 1100, 900);    // 1000
    return 1;
}

/*---------------------------------------------------------------------------*/
uint8_t adc_key_add     (uint16_t num, uint16_t key_code, uint16_t max_mv, uint16_t min_mv)
{
    if (num >= KEY_ADC_CNT) return 0;

    KeyGrp.keys[num].flags     = KEY_VALID_F;
    KeyGrp.keys[num].max_mv    = KeyGrp.ref_volt <= max_mv ? KeyGrp.ref_volt : max_mv;
    KeyGrp.keys[num].min_mv    = min_mv;
    KeyGrp.keys[num].code      = key_code;
    KeyGrp.keys[num].repeat_ms = 0;
    return 1;
}

/*---------------------------------------------------------------------------*/
uint8_t adc_key_remove  (uint16_t num)
{
    if (num >= KEY_ADC_CNT) return 0;

    memset (&KeyGrp.keys[num], 0, sizeof (struct adc_key));
    return 1;
}

/*---------------------------------------------------------------------------*/
uint8_t adc_key_repeat  (uint16_t num, uint16_t repeat_ms)
{
    if (num >= KEY_ADC_CNT) return 0;

    KeyGrp.keys[num].repeat_ms = repeat_ms;
    return 1;
}

/*---------------------------------------------------------------------------*/
uint16_t adc_key_read   (bool b_clr)
{
    uint16_t key_code;
    if ((KeyGrp.event & KEY_PRESS_F) && (KeyGrp.event & KEY_RELEASE_F))
        key_code = (KeyGrp.event & KEY_CODE_MASK);
    else
        key_code = 0;

    if (b_clr)  KeyGrp.event = 0;

    return  key_code;
}

/*---------------------------------------------------------------------------*/
void adc_key_check (void)
{
    /* Key event check */
    if (adc_key_read (0)) {
        uint8_t key_code = adc_key_read (1);
        switch (key_code) {
            case EVENT_D_VOL_UP:    case EVENT_D_VOL_DN:
                DigitalVolume = (key_code == EVENT_D_VOL_UP) ?
                                DigitalVolume +1 : DigitalVolume -1;
                tass805m_write (CODEC_REG_DGAIN, &DigitalVolume);
                eeprom_cfg_write ('D', DigitalVolume);
#if defined(_DEBUG_ADC_KEY_)
                printf ("%s : Digital volume = %d\r\n", __func__, DigitalVolume);
#endif
                break;

            case EVENT_A_VOL_UP:    case EVENT_A_VOL_DN:
                AnalogVolume = (key_code == EVENT_A_VOL_UP) ?
                                AnalogVolume +1 : AnalogVolume -1;
                AnalogVolume &= A_VOL_MASK;
                tass805m_write (CODEC_REG_AGAIN, &AnalogVolume);
                eeprom_cfg_write ('A', AnalogVolume);
#if defined(_DEBUG_ADC_KEY_)
                printf ("%s : Analog volume = %d\r\n", __func__, AnalogVolume);
#endif
                break;

            case EVENT_B_VAL_UP:    case EVENT_B_VAL_DN:
                Brightness = (key_code == EVENT_B_VAL_UP) ?
                                Brightness +1 : Brightness -1;
                backlight_control (Brightness);
                eeprom_cfg_write ('B', Brightness);
#if defined(_DEBUG_ADC_KEY_)
                printf ("%s : Brightness value = %d\r\n", __func__, Brightness);
#endif
                break;

            case EVENT_S_RESET:
#if defined(_DEBUG_ADC_KEY_)
                printf ("%s : System reboot\r\n", __func__);
#endif
                // backlight off, audio off
                backlight_control(0);   tass805m_mute();
                // watchdog reset (watch set 1 sec)
                watchdog_setup (WDT_RELOAD_1_S);
                while (1);
                break;

            case EVENT_T_RESET:
#if defined(_DEBUG_ADC_KEY_)
                printf ("%s : Touch reset\r\n", __func__);
#endif
                touch_reset (200);
                break;
        }
    }
}

/*---------------------------------------------------------------------------*/
void adc_key_loop       (void)
{
    uint8_t i;
    uint32_t adc_mv;

    if(MillisCheckADC + MILLIS_ADC_PERIOD < millis()) {
        MillisCheckADC = millis ();
        adc_mv =
            ((uint32_t)KeyGrp.ref_volt * (uint32_t)analogRead(KeyGrp.adc_port)) / ADC_RES_BITS;

        for (i = 0; i < KEY_ADC_CNT; i++) {
            if ((adc_mv >= KeyGrp.keys[i].min_mv) &&
                (adc_mv <= KeyGrp.keys[i].max_mv) &&
                (KeyGrp.keys[i].flags & KEY_VALID_F)) {

                KeyGrp.event = (KeyGrp.keys[i].code | KEY_PRESS_F);
            }
            if (!adc_mv) {
                if ((KeyGrp.event & KEY_PRESS_F) && ((KeyGrp.event & KEY_CODE_MASK) == KeyGrp.keys[i].code))
                    KeyGrp.event |=  (KEY_RELEASE_F);
            }
        }
        adc_key_check();
    }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
