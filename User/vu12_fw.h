/*---------------------------------------------------------------------------*/
/**
 * @file vu12_fw.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief VU12 main loop
 * @version 0.1
 * @date 2024-04-03
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifndef __VU12_FW_H__
#define __VU12_FW_H__

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif

/*---------------------------------------------------------------------------*/
#include <stdbool.h>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define WDT_RELOAD_3_2_S    4000
#define WDT_RELOAD_1_S      1250

#define WDT_CLR()           IWDG_ReloadCounter()

#define PROTOCOL_RESET_STR  "@WRST#"

/*---------------------------------------------------------------------------*/
// CH32V203(RISC-V) PORT Config
/*---------------------------------------------------------------------------*/
#define PORT_LCD_RESET      PA6
#define PORT_LCD_STDBY      PA7

#define PORT_CODEC_PWREN    PA9

#define PORT_I2C_SCL        PB6
#define PORT_I2C_SDA        PB7

#define PORT_LT8619C_RESET  PA5
#define PORT_BACKLIGHT_PWM  PA1

#define PORT_TOUCH_RESET    PB0

#define PORT_ALIVE_LED      PA4

#define PORT_ADC_KEY        PA0

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define PERIOD_LT8619C_LOOP     1000

// #define D_VOL_DEFAULT   0x30    // 0 dB, 0xFF mute, 소리가 너무 큼
#define D_VOL_DEFAULT       0x80    // -40 dB(default)
#define A_VOL_DEFAULT       0x00    // 0 dB
#define B_VAL_DEFAULT       0x80    // middle brightness

// #define MIN_BRIGHTNESS      10
#define HDMI_SIGNAL_STABLE  1

/*---------------------------------------------------------------------------*/
extern  uint8_t DigitalVolume, AnalogVolume, Brightness;

extern  void touch_reset    (uint8_t d);
extern  void watchdog_setup (uint16_t reload);

/*---------------------------------------------------------------------------*/
extern  void setup (void);
extern  void loop (void);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------*/
#endif  // #ifndef __VU12_FW_H__
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
