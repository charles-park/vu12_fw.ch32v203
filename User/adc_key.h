/*---------------------------------------------------------------------------*/
/**
 * @file adc_key.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief CH552 ADC Key
 * @version 0.1
 * @date 2024-02-15
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifndef __ADC_KEY_H__
#define __ADC_KEY_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*---------------------------------------------------------------------------*/
#include <stdbool.h>

/*---------------------------------------------------------------------------*/
/* ch32v203 adc resoluation = 12bits */
#define ADC_RES_BITS        (1 << 12)
#define MILLIS_ADC_PERIOD   50  /* 50 ms loop */

/*---------------------------------------------------------------------------*/
#define EVENT_D_VOL_UP  1
#define EVENT_D_VOL_DN  2

#define EVENT_A_VOL_UP  3
#define EVENT_A_VOL_DN  4

#define EVENT_B_VAL_UP  5
#define EVENT_B_VAL_DN  6

#define EVENT_T_RESET   7
#define EVENT_S_RESET   8

/*---------------------------------------------------------------------------*/
/* 1000ms 동안 눌려 있고 Repeat Flag가 활성화 된 경우 Repeat주기에 맞추어 Key전송함. */
/* repeat_ms 값이 0인 경우 1번만 Key값을 전송 후 종료함. */
/*---------------------------------------------------------------------------*/
#define KEY_LONGKEY_MS  1000

/*---------------------------------------------------------------------------*/
#define KEY_VALID_F     0x8000
#define KEY_REPEAT_F    0x4000

#define KEY_PRESS_F     0x2000
#define KEY_RELEASE_F   0x1000

#define KEY_CODE_MASK   0x0FFF

#define KEY_ADC_CNT     10

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
struct adc_key {
    uint16_t    min_mv, max_mv;
    uint16_t    flags;
    uint16_t    repeat_ms;
    uint16_t    code;
};

struct adc_key_grp {
    struct adc_key  keys[KEY_ADC_CNT];
    uint16_t    ref_volt;
    uint16_t    adc_port;
    uint16_t    event;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern  uint8_t  adc_key_add    (uint16_t num, uint16_t key_code, uint16_t max_mv, uint16_t min_mv);
extern  uint8_t  adc_key_remove (uint16_t num);
extern  uint8_t  adc_key_repeat (uint16_t num, uint16_t repeat_ms);
extern  void     adc_key_info   (void);
extern  uint16_t adc_key_read   (bool b_clr);
extern  void     adc_key_loop   (void);
extern  uint8_t  adc_key_init   (uint16_t adc_port, uint16_t ref_volt);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------*/
#endif  // #define __ADC_KEY_H__
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/


