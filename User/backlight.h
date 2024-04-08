/*---------------------------------------------------------------------------*/
/**
 * @file backlight.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief VU12 PWM Backlight control
 * @version 0.1
 * @date 2024-02-16
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifndef __BACKLIGHT_H__
#define __BACKLIGHT_H__

/*---------------------------------------------------------------------------*/
extern  uint8_t backlight_init      (uint16_t pwm_port);;
extern  uint8_t backlight_control   (uint8_t brightness);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#endif  // #define __BACKLIGHT_H__
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/


