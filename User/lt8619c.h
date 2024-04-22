/*---------------------------------------------------------------------------*/
/**
 * @file ltc8619.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief HDMI2LVDS(LT8691C) control
 * @version 0.1
 * @date 2024-02-21
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
#ifndef __LT8619C_H__
#define __LT8619C_H__

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define I2C_ADDR_LT8619C    0x64

/*---------------------------------------------------------------------------*/
#define LT8619C_ADDR        I2C_ADDR_LT8619C
#define LT8619C_RESET_PORT  PORT_LT8619C_RESET

#define USE_EXTERNAL_HDCPKEY

/*---------------------------------------------------------------------------*/
typedef struct {
    uint32_t pix_clk;
    uint16_t hfp;
    uint16_t hs;
    uint16_t hbp;
    uint16_t hact;
    uint16_t htotal;
    uint16_t vfp;
    uint16_t vs;
    uint16_t vbp;
    uint16_t vact;
    uint16_t vtotal;
}   video_timing;

/*---------------------------------------------------------------------------*/
enum LT8619C_OUTPUTMODE_ENUM
{
    OUTPUT_RGB888 = 0,
    OUTPUT_RGB666,
    OUTPUT_RGB565,
    OUTPUT_YCBCR444,
    OUTPUT_YCBCR422_16BIT,
    OUTPUT_YCBCR422_20BIT,
    OUTPUT_YCBCR422_24BIT,
    OUTPUT_BT656_8BIT,
    OUTPUT_BT656_10BIT,
    OUTPUT_BT656_12BIT,
    OUTPUT_BT1120_16BIT,
    OUTPUT_BT1120_20BIT,
    OUTPUT_BT1120_24BIT,
    OUTPUT_LVDS_2_PORT,
    OUTPUT_LVDS_1_PORT
};
#define LT8619C_OUTPUTMODE  OUTPUT_LVDS_2_PORT

/*---------------------------------------------------------------------------*/
enum LT8619C_AUDIOINPUT_MODE
{
    I2S_2CH = 0,
    SPDIF
};
#define Audio_Input_Mode I2S_2CH

/*---------------------------------------------------------------------------*/
enum INPUT_COLORSPACE
{
    COLOR_RGB = 0x00,
    COLOR_YCBCR444 = 0x40,
    COLOR_YCBCR422 = 0x20
};
#define LT8619C_OUTPUTCOLOR COLOR_RGB

/*---------------------------------------------------------------------------*/
enum INPUT_COLORIETRY
{
    NO_DATA = 0x00,
    ITU_601 = 0x40,
    ITU_709 = 0x80,
    EXTENDED_COLORIETRY = 0xc0
};

enum INPUT_EX_COLORIETRY
{
    xvYCC601 = 0x00,
    xvYCC709 = 0x10
    //FUTURE_COLORIETRY
};

enum INPUT_QUANTRANGE
{
    DEFAULT_RANGE   = 0x00,
    LIMIT_RANGE     = 0x04,
    FULL_RANGE      = 0x08,
    RESERVED_VAL    = 0xc0
};

/*---------------------------------------------------------------------------*/
typedef struct {
    bool        flag_RXClkStable;
    bool        flag_RXClkDetected;
    bool        flag_RXPLLLocked;
    bool        Flag_HsyncStable;
    bool        input_hdmimode;
    uint8_t     input_vic;
    uint8_t     input_colorspace;
    uint8_t     input_colordepth;
    uint8_t     input_colorimetry;
    uint8_t     input_ex_colorimetry;
    uint8_t     input_QuantRange;
    uint8_t     input_PRfactor;
    uint8_t     input_videoindex;
    uint32_t    ClkFreqValCurrent;
    uint32_t    ClkFreqValPrevious;
}   lt8619c_rxstatus;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern  uint8_t     lt8619c_i2c_write       (uint8_t reg, uint8_t d);
extern  uint8_t     lt8619c_i2c_read        (uint8_t reg);
extern  void        lt8619c_reset           (void);
extern  void        lt8619c_set_hpd         (bool level);
extern  void        lt8619c_edid_set        (uint8_t *pbuf);
extern  bool        lt8619c_load_hdcpkey    (void);
extern  void        lt8619c_rx_init         (void);
extern  void        lt8619c_audio_init      (void);
extern  void        lt8619c_rx_reset        (void);
extern  bool        lt8619c_clk_detect      (void);
extern  void        lt8619c_get_info        (void);
extern  void        lt8619c_csc_conv        (void);
extern  void        lt8619c_video_check     (void);
extern  void        lt8619c_bt_setting      (void);
extern  void        lt8619c_plllock_detect  (void);
extern  bool        lt8619c_lvds_check      (void);
extern  void        lt8619c_lvds_detect     (void);
extern  bool        lt8619c_loop            (void);
extern  bool        lt8619c_check_id        (void);
extern  void        lt8619c_init            (void);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#endif  // #define __LT8619C_H__

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
