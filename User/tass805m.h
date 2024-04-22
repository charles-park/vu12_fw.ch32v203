/*---------------------------------------------------------------------------*/
/**
 * @file tass805m.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief VU12 codec(tass805m) control
 * @version 0.1
 * @date 2024-04-16
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifndef __TASS805M_H__
#define __TASS805M_H__

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif

/*---------------------------------------------------------------------------*/
#define I2C_ADDR_CODEC          0x58

/*---------------------------------------------------------------------------*/
#define CODEC_REG_DEVICE_CTRL   0x03
#define CODEC_REG_AGAIN         0x54
#define CODEC_REG_DGAIN         0x4C

enum codec_state {
    STATE_DEEP_SLEEP = 0x0,
    STATE_SLEEP,
    STATE_HI_Z,
    STATE_PLAY,
};

/*---------------------------------------------------------------------------*/
extern  uint16_t    tass805m_write  (uint8_t reg, uint8_t *value);
extern  uint16_t    tass805m_read   (uint8_t reg, uint8_t *value);
extern  void        tass805m_mute   (void);
extern  void        tass805m_init   (void);

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#endif  // #define __TASS805M_H__
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

