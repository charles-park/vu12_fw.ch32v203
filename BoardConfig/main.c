/*---------------------------------------------------------------------------*/
/**
 * @file main.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief VU12 main
 * @version 0.1
 * @date 2024-04-03
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
#include "debug.h"
#include "board.h"

/*---------------------------------------------------------------------------*/
extern void setup (void);
extern void loop  (void);

/*---------------------------------------------------------------------------*/
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    // System delay func init (Delay_Ms, Delay_Us)
    Delay_Init();

    // ADC, Interrupt init (BoardConfig/board.c)
    board_init();

    // init config
    setup ();

    // Main app loop run...
    while (1)   loop();
    printf ("APP Error!!! Escape from main loop!!!\r\n");
}

/*---------------------------------------------------------------------------*/
