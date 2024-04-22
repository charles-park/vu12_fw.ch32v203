/*---------------------------------------------------------------------------*/
/**
 * @file backlight.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief VU12 PWM Backlight control
 * @version 0.1
 * @date 2024-02-16
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "Arduino.h"
#include "vu12_fw.h"
#include "backlight.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t backlight_init      (uint16_t pwm_port);
uint8_t backlight_control   (uint8_t brightness);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t PortPWM = 0;

/*---------------------------------------------------------------------------*/
//
// PWM Clock base 93Khz, div(PWM_CK_SE) = 0 ~ 255
// PWM Clock Output = PWM Clock base / div(PWM_CK_SE)
//
/*---------------------------------------------------------------------------*/
uint8_t backlight_init      (uint16_t pwm_port)
{
    PortPWM = pwm_port;
    analogWrite(PortPWM, 0);
#if defined (_DEBUG_BACKLIGHT_)
    printf ("%s : port = %d\r\n", __func__, pwm_port);
#endif
    return 1;
}

/*---------------------------------------------------------------------------*/
// 0 : OFF, 255 : Max brightness
/*---------------------------------------------------------------------------*/
uint8_t backlight_control   (uint8_t brightness)
{
    switch (brightness) {
        case 0 ... 3:
        case 255:
            pinMode (PortPWM, OUTPUT);
            digitalWrite (PortPWM, (brightness == 255) ? 1 : 0);
            break;
        default :
            analogWrite (PortPWM, (brightness));
            break;
    }
    // save backlight data
#if defined (_DEBUG_BACKLIGHT_)
    printf ("%s : brightness = %d\r\n", __func__, brightness);
#endif
    return 1;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
