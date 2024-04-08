/*---------------------------------------------------------------------------*/
/**
 * @file Arduino.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief Arduino function wrapper
 * @version 0.1
 * @date 2024-04-03
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifndef __ARDUINO_H__
#define __ARDUINO_H__

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif

/*---------------------------------------------------------------------------*/
#include "ch32v20x.h"
#include "usb_core.h"
#include "usb_prop.h"

/*---------------------------------------------------------------------------*/
#define HIGH    1
#define LOW     0

#define true    1
#define false   0

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
enum gpio_mode {
    INPUT_ANALOG = 0x0,
    // Floating
    INPUT = 0x04,
    INPUT_PULLDN = 0x28,
    INPUT_PULLUP = 0x48,
    OUTPUT_OD = 0x14,
    OUTPUT = 0x10,
    FUNC_OD = 0x1C,
    FUNC_PP = 0x18,
    MODE_END
};

#define GPIO_GROUP_MASK (0xF0)
#define GPIO_PIN_MASK   (0x0F)

#define GPIO_PIN(x)     (0x00000001 << (x & GPIO_PIN_MASK))
#define GPIO_GROUP(x)   ((x & GPIO_GROUP_MASK) >> 4)

enum gpio_groups {
    GPIO_GROUP_A = 0,
    GPIO_GROUP_B,
    GPIO_GROUP_C,
    GPIO_GROUP_D,
    GPIO_GROUP_END
};

enum gpio_pins {
    PA0 = 0,
    PA1 ,
    PA2 ,
    PA3 ,
    PA4 ,
    PA5 ,
    PA6 ,
    PA7 ,
    PA8 ,
    PA9 ,
    PA10,
    PA11,
    PA12,
    PA13,
    PA14,
    PA15,

    PB0 ,
    PB1 ,
    PB2 ,
    PB3 ,
    PB4 ,
    PB5 ,
    PB6 ,
    PB7 ,
    PB8 ,
    PB9 ,
    PB10,
    PB11,
    PB12,
    PB13,
    PB14,
    PB15,

    PC0 ,
    PC1 ,
    PC2 ,
    PC3 ,
    PC4 ,
    PC5 ,
    PC6 ,
    PC7 ,
    PC8 ,
    PC9 ,
    PC10,
    PC11,
    PC12,
    PC13,
    PC14,
    PC15,

    PD0 ,
    PD1 ,
    PD2 ,
    GPIO_PINS_END
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern void             pinMode         (enum gpio_pins pin, enum gpio_mode mode);
extern void             digitalWrite    (enum gpio_pins pin, unsigned char status);
extern unsigned char    digitalRead     (enum gpio_pins pin);
extern void             analogWrite     (enum gpio_pins pin, unsigned char value);
extern unsigned short   analogRead      (enum gpio_pins pin);
extern unsigned long    millis          (void);
extern void             delay           (unsigned long ms);
extern void             mdelay          (unsigned long ms);
extern void             udelay          (unsigned long us);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------*/
#endif  // #ifndef __ARDUINO_H__

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
