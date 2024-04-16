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
#include "Arduino.h"
#include "board.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
GPIO_TypeDef *gpio_reg_addr (enum gpio_pins pin)
{
    // clock enable
    switch (GPIO_GROUP(pin)) {
        case GPIO_GROUP_A:
            RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
            return GPIOA;
            break;
        case GPIO_GROUP_B:
            RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB, ENABLE);
            return GPIOB;
            break;
        case GPIO_GROUP_C:
            RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOC, ENABLE);
            return GPIOC;
            break;
        case GPIO_GROUP_D:
            RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOD, ENABLE);
            return GPIOD;
            break;
        default :
            return 0;
    }
}

/*---------------------------------------------------------------------------*/
void pinMode (enum gpio_pins pin, enum gpio_mode mode)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_TypeDef *GPIO_REG = gpio_reg_addr (pin);

    // Default GPIO Speed
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Mode = mode;
    if ((mode == FUNC_OD) || (mode == FUNC_PP))
	    RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );

    GPIO_InitStructure.GPIO_Pin  = GPIO_PIN(pin);

    GPIO_Init (GPIO_REG, &GPIO_InitStructure);
    GPIO_ResetBits (GPIO_REG, GPIO_InitStructure.GPIO_Pin);
}

/*---------------------------------------------------------------------------*/
void digitalWrite (enum gpio_pins pin, unsigned char status)
{
    GPIO_TypeDef *GPIO_REG = gpio_reg_addr (pin);

    status ? GPIO_SetBits (GPIO_REG, GPIO_PIN(pin)) : GPIO_ResetBits (GPIO_REG, GPIO_PIN(pin));
}

/*---------------------------------------------------------------------------*/
unsigned char digitalRead (enum gpio_pins pin)
{
    GPIO_TypeDef *GPIO_REG = gpio_reg_addr (pin);

    return GPIO_ReadInputDataBit(GPIO_REG, GPIO_PIN(pin)) ? 1 : 0;
}

/*---------------------------------------------------------------------------*/
// PWM control
/*---------------------------------------------------------------------------*/
void analogWrite (enum gpio_pins pin, unsigned char value)
{
    // BoardConfig/board.c
    PWM_init (pin, value);
}

/*---------------------------------------------------------------------------*/
// ADC
/*---------------------------------------------------------------------------*/
unsigned short Get_ConversionVal (short val)
{
    if((val + ADCCalValue) < 0)
        return 0;
    if(((ADCCalValue + val) > 4095) || (val==4095))
        return 4095;
    return (val + ADCCalValue);
}

/*---------------------------------------------------------------------------*/
unsigned short analogRead (enum gpio_pins pin)
{
    unsigned short ch = pin & GPIO_PIN_MASK, adc_value;

    switch (pin) {
        /* ADC0 ~ ADC7 */
        case PA0 ... PA7:
            break;
        /* ADC8, ADC9 */
        case PB0: case PB1:
            ch += 8;
            break;
        default :
            return 0;
    }
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

    adc_value = ADC_GetConversionValue(ADC1);

    return Get_ConversionVal(adc_value);
}

/*---------------------------------------------------------------------------*/
unsigned long millis (void)
{
    return (unsigned long)msTickCount;
}

/*---------------------------------------------------------------------------*/
void delay (unsigned long ms)
{
    Delay_Ms (ms);
}

/*---------------------------------------------------------------------------*/
void mdelay (unsigned long ms)
{
    Delay_Ms (ms);
}

/*---------------------------------------------------------------------------*/
void udelay (unsigned long us)
{
    Delay_Us (us);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
