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
#define PERIOD_LT8619C_LOOP 1000

#define D_VOL_DEFAULT       0x50    // -16 dB(default) (24db - value * 0.5)
#define A_VOL_DEFAULT       0x00    // 0 dB
#define B_VAL_DEFAULT       0xFF    // middle brightness

// Analog volume range 0 ~ 31 (0 ~ 0x1F)
#define A_VOL_MASK          0x1F

// HDMI2LVDS Signal status
enum {
    eSTATUS_NO_SIGNAL = 0,
    eSTATUS_SIGNAL_DETECT,
    eSTATUS_AUDIO_INIT,
    eSTATUS_BACKLIGHT_INIT,
    eSTATUS_SIGNAL_STABLE,
    eSTATUS_END
};

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
