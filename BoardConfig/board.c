 /*---------------------------------------------------------------------------*/
/**
 * @file board.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief board init (system low level init)
 * @version 0.1
 * @date 2024-04-03
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
#include "debug.h"
#include "board.h"
#include "ch32v20x.h"

#include "usb_core.h"
#include "usb_init.h"
#include "hw_config.h"

/*---------------------------------------------------------------------------*/
/*
    @fn      TIM3_IRQHandler(void)

    @brief   This function handles TIM3 Update event. (1ms irq event)

    @return  none
 */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
volatile u32 msTickCount = 0;
volatile s32 ADCCalValue = 0;

/*---------------------------------------------------------------------------*/
void Timer3_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void Timer3_Handler(void)
{
    msTickCount++;
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);
}

/*---------------------------------------------------------------------------*/
// TIM3 Update interrupt
/*---------------------------------------------------------------------------*/
void Timer3_irq_init (void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};

    RCC_APB1PeriphClockCmd ( RCC_APB1Periph_TIM3, ENABLE );

    /* IRQ_Hz  = (CPU_CLOCK / TIM_CKD_DIV2) / (TIM_Prescaler + TIM_Period) */
    // Prescaler = (96000000 / 2 * 1000(Hz))
    TIM_TimeBaseInitStructure.TIM_Period = 1;
    TIM_TimeBaseInitStructure.TIM_Prescaler = SystemCoreClock / (2 * 1000);

    /* TIM_CKD_DIV1, 2, 4 */
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV2;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down;

    TIM_TimeBaseInit ( TIM3, &TIM_TimeBaseInitStructure);
    TIM_Cmd ( TIM3, ENABLE );

    // Interrupt enable
    TIM_ITConfig  (TIM3, TIM_IT_Update, ENABLE);
    SetVTFIRQ ((u32)Timer3_Handler, TIM3_IRQn, 0, ENABLE);
    NVIC_EnableIRQ (TIM3_IRQn);
}

/*---------------------------------------------------------------------------*/
/* PWM output clock 93.75 Khz */
/* Freq = Systemclk / (PWM_DEFAULT_ARR) / (PWM_DEFAULT_PCS +1) */
/*---------------------------------------------------------------------------*/
#define PWM_DEFAULT_ARR 256
//#define PWM_DEFAULT_PCS 3 - Fail (005) 93.75Khz
//#define PWM_DEFAULT_PCS 9   // 38Khz (Analog dimming)
//#define PWM_DEFAULT_PCS 2   // 125Khz (Analog dimming)
//#define PWM_DEFAULT_PCS 5   // 125Khz (Analog dimming)

//#define PWM_DEFAULT_PCS 15  // 23 Khz..OK

#define PWM_DEFAULT_PCS 15  // 23 Khz..OK

void PWM_init (enum gpio_pins pin, u8 ccp)
{
    TIM_TypeDef *TIM;
    u8 ch;
    TIM_OCInitTypeDef TIM_OCInitStructure={0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};

    switch (pin) {
        case PA0:   case PA15:  TIM = TIM2; ch = 1; break;
        case PA1:   TIM = TIM2; ch = 2; break;
        case PA2:   TIM = TIM2; ch = 3; break;
        case PA3:   TIM = TIM2; ch = 4; break;

        case PA8:   TIM = TIM1; ch = 1; break;
        case PA9:   TIM = TIM1; ch = 2; break;
        case PA10:  TIM = TIM1; ch = 3; break;
        case PA11:  TIM = TIM1; ch = 4; break;

        case PA6:   case PB4:   TIM = TIM3; ch = 1; break;
        case PA7:   case PB5:   TIM = TIM3; ch = 2; break;
        case PB0:   TIM = TIM3; ch = 3; break;
        case PB1:   TIM = TIM3; ch = 4; break;

        case PB6:   TIM = TIM4; ch = 1; break;
        case PB7:   TIM = TIM4; ch = 2; break;
        case PB8:   TIM = TIM4; ch = 3; break;
        case PB9:   TIM = TIM4; ch = 4; break;

        default:    return;
    }

    /* Timer3 disable */
    if (TIM == TIM3) {
        printf ("%s : TIMER3 is used internally.\r\n");
        printf ("%s : TIMER3 cannot be used for any other purpose.\r\n");
        return;
    }
    if (TIM == TIM1)    RCC_APB1PeriphClockCmd( RCC_APB2Periph_TIM1, ENABLE );
    if (TIM == TIM2)    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );
    if (TIM == TIM3)    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM3, ENABLE );
    if (TIM == TIM4)    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM4, ENABLE );

    pinMode (pin, FUNC_PP);
    TIM_TimeBaseInitStructure.TIM_Period = PWM_DEFAULT_ARR;
    TIM_TimeBaseInitStructure.TIM_Prescaler = PWM_DEFAULT_PCS;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down;
    TIM_TimeBaseInit ( TIM, &TIM_TimeBaseInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = ccp;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    switch (ch) {
        case 1: TIM_OC1Init ( TIM, &TIM_OCInitStructure );
                TIM_OC1PreloadConfig ( TIM, TIM_OCPreload_Disable );    break;
        case 2: TIM_OC2Init ( TIM, &TIM_OCInitStructure );
                TIM_OC2PreloadConfig ( TIM, TIM_OCPreload_Disable );    break;
        case 3: TIM_OC3Init ( TIM, &TIM_OCInitStructure );
                TIM_OC3PreloadConfig ( TIM, TIM_OCPreload_Disable );    break;
        case 4: TIM_OC4Init ( TIM, &TIM_OCInitStructure );
                TIM_OC4PreloadConfig ( TIM, TIM_OCPreload_Disable );    break;
        default :   return;
    }
    TIM_ARRPreloadConfig ( TIM, ENABLE );
    TIM_CtrlPWMOutputs (TIM, ENABLE );
    TIM_Cmd ( TIM, ENABLE );
}

/*---------------------------------------------------------------------------*/
void ADC_init (void)
{
    ADC_InitTypeDef  ADC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_DMACmd(ADC1, DISABLE);
    ADC_Cmd(ADC1, ENABLE);

    ADC_BufferCmd(ADC1, DISABLE); //disable buffer
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    ADCCalValue = Get_CalibrationValue(ADC1);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*
    VU12 Hardware config

    SYSTEM Memory setting : Ld/Link.ld (CH32VG6U6)
        FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 32K
        RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 10K

    SYSTEM Clock setting : BoardConfig/system_ch32v20x.c (internal RC clock)
        #define SYSCLK_FREQ_96MHz_HSI  96000000

    SYSTEM Debug UART setting : Debug/debug.h (USART2)
        #define DEBUG   DEBUG_UART2

    Debug UART Baudrate : BoardConfig/board.c
        USART_Printf_Init(115200);

    SYSTEM millis() function : BoardConfig/board.c
        Timer3_irq_init();

    SYSTEM PWM Setting (93.75Khz, 0 ~ 255 range) : BoardConfig/board.c
        PWM_init();
*/
/*---------------------------------------------------------------------------*/
void board_init (void)
{
    // USB CDC(ttyACM) Init (PA 11/12)
    Set_USBConfig();
    USB_Init();
    USB_Interrupts_Config();

    // timer irq init (TIM3_CH4, 1ms, millis())
    Timer3_irq_init ();

    // adc init. adc calibration
    ADC_init ();

    // i2c init (GPIO, PB 6/7)
    // spi init (PA 15, PB 3/4/5)
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
