/*---------------------------------------------------------------------------*/
/**
 * @file ltc8619.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief HDMI2LVDS(LT8691C) control
 * @version 0.1
 * @date 2024-02-21
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "Arduino.h"

#include "vu12_fw.h"
#include "lt8619c.h"
#include "gpio_i2c.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t     lt8619c_i2c_write       (uint8_t reg, uint8_t d);
uint8_t     lt8619c_i2c_read        (uint8_t reg);
void        lt8619c_reset           (void);
void        lt8619c_set_hpd         (bool level);
void        lt8619c_edid_set        (uint8_t *pbuf);
bool        lt8619c_load_hdcpkey    (void);
void        lt8619c_rx_init         (void);
void        lt8619c_audio_init      (void);
void        lt8619c_rx_reset        (void);
bool        lt8619c_clk_detect      (void);
void        lt8619c_get_info        (void);
void        lt8619c_csc_conv        (void);
void        lt8619c_video_check     (void);
void        lt8619c_bt_setting      (void);
void        lt8619c_plllock_detect  (void);
bool        lt8619c_lvds_check      (void);
void        lt8619c_lvds_detect     (void);
bool        lt8619c_loop            (void);
bool        lt8619c_check_id        (void);
void        lt8619c_init            (void);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
lt8619c_rxstatus    LT8619C_RXStatus, *pLT8619C_RXStatus;

uint16_t    hActive, vActive;
uint16_t    hSyncWidth, vSyncWidth;
uint16_t    hBackPorch, vBackPorch;
uint16_t    hTotal, vTotal;
uint8_t     hSyncPol, vSyncPol, Color;
uint32_t    FrameCounter;

/*---------------------------------------------------------------------------*/
// fixed edid data
const uint8_t ONCHIP_EDID[256] = {
/*
    // https://tomverbeure.github.io/video_timings_calculator
    // Parameters
    // None, 1920, 720, 60, N, N, 8, RGB 4:4:4, N
    // CVT Timings
    const video_timing  lcd_timing = {
        111750,  // pixel clock(Khz)

        // lcd horizontal data
        96,     // hfp
        192,    // hs
        288,    // hbp
        1920,   // hact = (real view area, 960 * 2(LVDS ch) = 1920)
        2496,   // htotal = hact + hbp + hs + hfp

        // lcd vertical data
        3,      // vfp
        10,     // vs
        15,     // vbp
        720,    // vact = (real view area, 720)
        748     // vtotal = vact + vbp + vs + vfp
    };
*/
    #if 0
        CVT PC-OK, C4-OK
        Calculate EDID data [0 ~ 127]

        pix_clk = 111750

        hfp    = 96
        hs     = 192
        hbp    = 288
        hact   = 1920
        htotal = 2496

        vfp    = 3
        vs     = 10
        vbp    = 15
        vact   = 720
        vtotal = 748

        *** pix_clk cal = 112020
    #endif
    0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0e, 0xd4, 0x32, 0x31, 0x00, 0x88, 0x88, 0x88,
    0x20, 0x1c, 0x01, 0x03, 0x80, 0x1d, 0x0b, 0x78, 0x0a, 0x0d, 0xc9, 0xa0, 0x57, 0x47, 0x98, 0x27,
    0x12, 0x48, 0x4c, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xa7, 0x2b, 0x80, 0x40, 0x72, 0xd0, 0x1c, 0x20, 0x60, 0xc0,
    0x3a, 0x00, 0x80, 0x38, 0x74, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x0a,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc,
    0x00, 0x4c, 0x6f, 0x6e, 0x74, 0x69, 0x75, 0x6d, 0x20, 0x73, 0x65, 0x6d, 0x69, 0x20, 0x01, 0x29,

    // 128 ~ 256
    0x02, 0x03, 0x12, 0xf1, 0x23, 0x09, 0x07, 0x07, 0x83, 0x01, 0x00, 0x00, 0x65, 0x03, 0x0c, 0x00,
    0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf,
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t     lt8619c_i2c_write       (uint8_t reg, uint8_t d)
{
    return  i2c_send (LT8619C_ADDR, reg, &d, 1);
}

/*---------------------------------------------------------------------------*/
uint8_t     lt8619c_i2c_read        (uint8_t reg)
{
    uint8_t d;

    i2c_read (LT8619C_ADDR, reg, &d, 1);
    return  d;
}

/*---------------------------------------------------------------------------*/
void        lt8619c_reset           (void)
{
    digitalWrite (LT8619C_RESET_PORT, LOW);
    delay (100);
    digitalWrite (LT8619C_RESET_PORT, HIGH);
    delay (100);
}

/*---------------------------------------------------------------------------*/
void        lt8619c_set_hpd         (bool level)
{
    uint8_t rd;

    lt8619c_i2c_write (0xFF, 0x80);
    rd = lt8619c_i2c_read (0x06);
    lt8619c_i2c_write (0x06, level ? (rd | 0x08) : (rd & 0xF7));
}

/*---------------------------------------------------------------------------*/
void        lt8619c_edid_set        (uint8_t *pbuf)
{
    lt8619c_i2c_write (0xFF, 0x80);
    lt8619c_i2c_write (0x8E, 0x07);
    lt8619c_i2c_write (0x8F, 0x00);
    i2c_send (LT8619C_ADDR, 0x90, pbuf, 256);
    lt8619c_i2c_write (0x8E, 0x02);
}

/*---------------------------------------------------------------------------*/
bool        lt8619c_load_hdcpkey    (void)
{
    uint8_t key_done = 0, loop_cnt = 0;

    lt8619c_i2c_write (0xFF, 0x80);
    lt8619c_i2c_write (0xB2, 0x50);
    lt8619c_i2c_write (0xA3, 0x77);

    while (loop_cnt++ <= 5) {
        delay (50);
        if ((key_done = lt8619c_i2c_read (0xC0) & 0x08))    break;
    }

    lt8619c_i2c_write (0xB2, 0xD0);
    lt8619c_i2c_write (0xA3, 0x57);
    return  key_done ? true : false;
}

/*---------------------------------------------------------------------------*/
void        lt8619c_rx_init         (void)
{
    lt8619c_i2c_write (0xFF, 0x80);
    lt8619c_i2c_write (0x2C, lt8619c_i2c_read (0x2c) | 0x30);  //RGD_CLK_STABLE_OPT[1:0]
    lt8619c_i2c_write (0xFF, 0x60);
    lt8619c_i2c_write (0x04, 0xF2);
    lt8619c_i2c_write (0x83, 0x3F);
    lt8619c_i2c_write (0x80, 0x08); //use xtal_clk as sys_clk.
    lt8619c_i2c_write (0xA4, 0x10); //0x10:SDR clk,0x14: DDR clk

    // LT8619C_OUTPUTMODE ==  OUTPUT_LVDS_2_PORT
    {
        lt8619c_i2c_write (0xFF, 0x60);
        lt8619c_i2c_write (0x06, 0xE7);
        lt8619c_i2c_write (0x59, 0x40); //bit7 for VESA/JEIDA mode; bit5 for DE/SYNC mode; bit4 for 6/8bit; bit1 for port swap
        lt8619c_i2c_write (0xA0, 0x58);
        lt8619c_i2c_write (0xA4, 0x01);
        lt8619c_i2c_write (0xA8, 0x00);
        lt8619c_i2c_write (0xBA, 0x18); // LVDS out enable 0x18; LVDS out disable: 0x44
        lt8619c_i2c_write (0xC0, 0x18); // LVDS out enable 0x18; LVDS out disable: 0x44
        lt8619c_i2c_write (0xB0, 0x66); // LVDS Swing
        lt8619c_i2c_write (0xB1, 0x66); // LVDS Swing
        lt8619c_i2c_write (0xB2, 0x66); // LVDS Swing
        lt8619c_i2c_write (0xB3, 0x66); // LVDS Swing
        lt8619c_i2c_write (0xB4, 0x66); // LVDS Swing
        lt8619c_i2c_write (0xB5, 0x41);
        lt8619c_i2c_write (0xB6, 0x41);
        lt8619c_i2c_write (0xB7, 0x41);
        lt8619c_i2c_write (0xB8, 0x4C);
        lt8619c_i2c_write (0xB9, 0x41);
        lt8619c_i2c_write (0xBB, 0x41);
        lt8619c_i2c_write (0xBC, 0x41);
        lt8619c_i2c_write (0xBD, 0x41);
        lt8619c_i2c_write (0xBE, 0x4c);
        lt8619c_i2c_write (0xBF, 0x41);
        lt8619c_i2c_write (0xA0, 0x18);
        lt8619c_i2c_write (0xA1, 0xB0);
        lt8619c_i2c_write (0xA2, 0x10);
        lt8619c_i2c_write (0x5c, 0x01); //bit0=0:single port  bit0=1:dual port
    }
    lt8619c_i2c_write (0xFF, 0x60);
    lt8619c_i2c_write (0x0E, 0xFD);
    lt8619c_i2c_write (0x0E, 0xFF);
    lt8619c_i2c_write (0x0D, 0xFC);
    lt8619c_i2c_write (0x0D, 0xFF);
}

/*---------------------------------------------------------------------------*/
void        lt8619c_audio_init      (void)
{
    // Audio_Input_Mode == I2S_2CH
    {
        lt8619c_i2c_write (0xFF, 0x60);
        lt8619c_i2c_write (0x4C, 0x00);
    }
    lt8619c_i2c_write (0xFF, 0x80);
    lt8619c_i2c_write (0x5D, 0xC9);
    lt8619c_i2c_write (0x08, 0x80);
}

/*---------------------------------------------------------------------------*/
void        lt8619c_rx_reset        (void)
{
    lt8619c_i2c_write (0xFF, 0x60);
    lt8619c_i2c_write (0x0E, 0xBF); /* reset RXPLL */
    lt8619c_i2c_write (0x09, 0xFD); /* reset RXPLL Lock det*/
    delay (5);
    lt8619c_i2c_write (0x0E, 0xFF); /* release RXPLL */
    lt8619c_i2c_write (0x09, 0xFF);
    lt8619c_i2c_write (0xFF, 0x60);
    lt8619c_i2c_write (0x0E, 0xC7); /* reset PI */
    lt8619c_i2c_write (0x09, 0x0F); /* reset RX,CDR */
    delay (10);

    lt8619c_i2c_write (0x0E, 0xFF); /* release PI */
    delay (10);

    lt8619c_i2c_write (0x09, 0x8F); /* release RX */
    delay (10);

    lt8619c_i2c_write (0x09, 0xFF); /* release CDR */
    delay (50);
}

/*---------------------------------------------------------------------------*/
bool        lt8619c_clk_detect      (void)
{
    uint8_t rd;

    lt8619c_i2c_write (0xFF, 0x80);

    if (lt8619c_i2c_read (0x44) & 0x08)
    {
        if (!pLT8619C_RXStatus->flag_RXClkStable)
        {
            pLT8619C_RXStatus->flag_RXClkStable = !pLT8619C_RXStatus->flag_RXClkStable;
            lt8619c_i2c_write (0xFF,0x60);

            rd = lt8619c_i2c_read (0x97);
            lt8619c_i2c_write (0x97, rd & 0x3f);

            lt8619c_i2c_write (0xFF, 0x80);
            lt8619c_i2c_write (0x1B, 0x00);

            lt8619c_rx_reset(); delay (5);

            lt8619c_i2c_write (0xFF, 0x80);
            rd = lt8619c_i2c_read (0x87) & 0x10;

            if ( rd ) {
                pLT8619C_RXStatus->flag_RXPLLLocked = true;
#if defined (_DEBUG_LT8619C_)
                printf ("LT8619C clk detected!!!\r\n");
                printf ("LT8619C pll lock!!!\r\n");
#endif
                return true;
            } else {
                pLT8619C_RXStatus->flag_RXPLLLocked = false;
                memset (pLT8619C_RXStatus, 0, sizeof(LT8619C_RXStatus));
#if defined (_DEBUG_LT8619C_)
                printf ("LT8619C clk detected!!!\r\n");
                printf ("LT8619C pll unlock#####\r\n");
#endif
                return false;
            }
        } else {
            lt8619c_i2c_write (0xFF, 0x80);
            rd = lt8619c_i2c_read (0x87) & 0x10;

            if ( rd ) {
                pLT8619C_RXStatus->flag_RXPLLLocked = true;
                return true;
            } else {
                pLT8619C_RXStatus->flag_RXPLLLocked = true;
                memset (pLT8619C_RXStatus, 0, sizeof(LT8619C_RXStatus));
#if defined (_DEBUG_LT8619C_)
                printf ("LT8619C pll unlock!!!####$$\r\n");
#endif
                return false;
            }
        }
    } else {
#if defined (_DEBUG_LT8619C_)
        if( pLT8619C_RXStatus->flag_RXClkStable )
            printf ("LT8619C clk disappear!!!\r\n");
#endif
        memset (pLT8619C_RXStatus, 0, sizeof(LT8619C_RXStatus));
        return false;
    }
}

/*---------------------------------------------------------------------------*/
void        lt8619c_get_info        (void)
{
    uint8_t cnt, rd;

    lt8619c_i2c_write (0xFF, 0x80);
    if ( pLT8619C_RXStatus->flag_RXClkStable && pLT8619C_RXStatus->flag_RXPLLLocked ) {
#if defined (_DEBUG_LT8619C_)
        printf ("Hsync check start: \r\n");
#endif
        if ( lt8619c_i2c_read (0x13) & 0x01 ) {
#if defined (_DEBUG_LT8619C_)
            // Hsync is detected, and is stable.
            printf ("%d : LT8619C Hsync is stable.\r\n", millis());
#endif
            if ( !pLT8619C_RXStatus->Flag_HsyncStable ) {
                pLT8619C_RXStatus->Flag_HsyncStable = true;

                for (cnt = 0; cnt < 8; cnt++) {
#if defined (_DEBUG_LT8619C_)
                    printf ("Hsync check num: %d\r\n", cnt);
#endif
                    delay (20);
                    if ( !(lt8619c_i2c_read (0x13) & 0x01) ) {
#if defined (_DEBUG_LT8619C_)
                        printf ("LT8619C 8013[0]=0 !!!#####\r\n");
                        printf ("LT8619C Hsync stable Fail #####\r\n");
#endif
                        pLT8619C_RXStatus->Flag_HsyncStable = false;
                        break;
                    }
                }
                if ( pLT8619C_RXStatus->Flag_HsyncStable ) {
                    lt8619c_i2c_write (0xFF, 0x60);
                    rd = lt8619c_i2c_read (0x0D);
                    lt8619c_i2c_write (0x0D, rd & 0xF8);    //reset LVDS/BT fifo
                    lt8619c_i2c_write (0x0D, rd | 0x06);
                    lt8619c_i2c_write (0x0D, rd | 0x01);
#if defined (_DEBUG_LT8619C_)
                    printf ("LT8619C Hsync stable!!!\r\n");
#endif
                }
            }
        } else {
#if defined (_DEBUG_LT8619C_)
            if ( pLT8619C_RXStatus->Flag_HsyncStable )
                printf ("LT8619C Hsync stable to unstable#####\r\n");

            printf ("LT8619C Hsync always unstable#####\r\n");
#endif
            pLT8619C_RXStatus->Flag_HsyncStable = false;
        }
    }
    if ( pLT8619C_RXStatus->Flag_HsyncStable ) {
        rd = lt8619c_i2c_read (0x13);
        pLT8619C_RXStatus->input_hdmimode = (rd & 0x02) ? (true):(false);
        if ( pLT8619C_RXStatus->input_hdmimode ) {
            pLT8619C_RXStatus->input_vic            = lt8619c_i2c_read (0x74) & 0x7F;
            pLT8619C_RXStatus->input_colorspace     = lt8619c_i2c_read (0x71) & 0x60;
            pLT8619C_RXStatus->input_colordepth     = lt8619c_i2c_read (0x16) & 0xF0;
            pLT8619C_RXStatus->input_colorimetry    = lt8619c_i2c_read (0x72) & 0xC0;
            pLT8619C_RXStatus->input_ex_colorimetry = lt8619c_i2c_read (0x73) & 0x70;
            pLT8619C_RXStatus->input_QuantRange     = lt8619c_i2c_read (0x73) & 0x0C;
            pLT8619C_RXStatus->input_PRfactor       = lt8619c_i2c_read (0x75) & 0x0F;

            lt8619c_i2c_write (0xFF, 0x60);
            rd = lt8619c_i2c_read (0x97);
            if ( pLT8619C_RXStatus->input_PRfactor == 1 ) {
                lt8619c_i2c_write (0x97, rd | 0x40);    rd = 0x20;
            } else if ( pLT8619C_RXStatus->input_PRfactor == 3 ) {
                lt8619c_i2c_write (0x97 ,rd | 0x80);    rd = 0x60;
            } else {
                lt8619c_i2c_write (0x97, rd & 0x3F);    rd = 0x00;
            }
            lt8619c_i2c_write (0xFF, 0x80); lt8619c_i2c_write (0x1B, rd);
        } else {
            pLT8619C_RXStatus->input_vic            = 0;
            pLT8619C_RXStatus->input_colorspace     = COLOR_RGB;
            pLT8619C_RXStatus->input_colordepth     = 0;
            pLT8619C_RXStatus->input_colorimetry    = ITU_709;
            pLT8619C_RXStatus->input_ex_colorimetry = 0;
            pLT8619C_RXStatus->input_QuantRange     = FULL_RANGE;
            pLT8619C_RXStatus->input_PRfactor       = 0;
            lt8619c_i2c_write (0xFF, 0x60);
            rd = lt8619c_i2c_read (0x97);   lt8619c_i2c_write (0x97, rd & 0x3F);
            lt8619c_i2c_write (0xFF, 0x80); lt8619c_i2c_write (0x1B, 0x00);
        }
    }
}

/*---------------------------------------------------------------------------*/
void        lt8619c_csc_conv        (void)
{
    lt8619c_i2c_write (0xFF, 0x60);
    lt8619c_i2c_write (0x07, 0xFE);

    // if( LT8619C_OUTPUTCOLOR == COLOR_RGB )
    {
        if( pLT8619C_RXStatus->input_colorspace == COLOR_RGB )
        {
            lt8619c_i2c_write (0x52, 0x00);
            lt8619c_i2c_write (0x53,
                (pLT8619C_RXStatus->input_QuantRange == LIMIT_RANGE) ? 0x08 : 0x00);
        } else {
            // input_colorspace = COLOR_YCBCR444 or COLOR_YCBCR422 or <ELSE>.
            lt8619c_i2c_write (0x52,
                (pLT8619C_RXStatus->input_colorspace == COLOR_YCBCR422) ? 0x01 : 0x00);

            if ( pLT8619C_RXStatus->input_QuantRange == LIMIT_RANGE )
            {
                switch (pLT8619C_RXStatus->input_colorimetry) {
                    case ITU_601:   lt8619c_i2c_write (0x53, 0x50); break;
                    // NO_DATA, ITU_709, EXTENDED_COLORIETRY:
                    default:        lt8619c_i2c_write (0x53, 0x70); break;
                }
            } else if ( pLT8619C_RXStatus->input_QuantRange == FULL_RANGE ) {
                switch (pLT8619C_RXStatus->input_colorimetry) {
                    case ITU_601:   lt8619c_i2c_write (0x53, 0x40); break;
                    // NO_DATA, ITU_709, EXTENDED_COLORIETRY:
                    default:        lt8619c_i2c_write (0x53, 0x60); break;
                }
            } else {
                //DEFAULT_RANGE or RESERVED_VAL
                lt8619c_i2c_write (0x53, 0x60);
            }
        }
    }
}

/*---------------------------------------------------------------------------*/
void        lt8619c_video_check     (void)
{
    uint8_t rd;

    if ( !pLT8619C_RXStatus->Flag_HsyncStable ) {
        hTotal = vTotal = 0;    return;
    }

    lt8619c_i2c_write (0xFF, 0x60);
    hActive       = ((uint16_t)lt8619c_i2c_read (0x22))<<8;
    hActive      += lt8619c_i2c_read (0x23);
    vActive       = ((uint16_t)(lt8619c_i2c_read (0x20) & 0x0F))<<8;
    vActive      += lt8619c_i2c_read (0x21);

    FrameCounter  = ((uint32_t)lt8619c_i2c_read (0x10))<<16;
    FrameCounter += ((uint32_t)lt8619c_i2c_read (0x11))<<8;
    FrameCounter += lt8619c_i2c_read (0x12);

    hSyncWidth    = ((uint16_t)(lt8619c_i2c_read (0x14) & 0x0F))<<8;
    hSyncWidth   += lt8619c_i2c_read (0x15);
    vSyncWidth    = lt8619c_i2c_read (0x13);

    hBackPorch    = ((uint16_t)(lt8619c_i2c_read (0x18) & 0x0F))<<8;
    hBackPorch   += lt8619c_i2c_read (0x19);
    vBackPorch    = lt8619c_i2c_read (0x16);

    hTotal        = ((uint16_t)lt8619c_i2c_read (0x1E))<<8;
    hTotal       += lt8619c_i2c_read (0x1F);
    vTotal        = ((uint16_t)(lt8619c_i2c_read (0x1C) & 0x0F))<<8;
    vTotal       += lt8619c_i2c_read (0x1D);

    rd            = lt8619c_i2c_read (0x24);
    hSyncPol      = (rd & 0x01);
    vSyncPol      = (rd & 0x02)>>1;
}

/*---------------------------------------------------------------------------*/
void        lt8619c_bt_setting      (void)
{
    uint8_t val_6060;
    uint16_t tmp;

    if ( !pLT8619C_RXStatus->Flag_HsyncStable ) return;

    lt8619c_plllock_detect();

    lt8619c_i2c_write (0xFF, 0x60);
    val_6060 = lt8619c_i2c_read (0x60) & 0xC7;

    //set BT TX h/vsync polarity
    if ( hSyncPol ) val_6060 |= 0x20;
    if ( vSyncPol ) val_6060 |= 0x10;

    //double the value of v_active&v_total when input is interlace resolution.
    //if user needs to support interlace format not listed here, please add that interlace format info here.
    if ( pLT8619C_RXStatus->input_hdmimode ) {
        switch ( pLT8619C_RXStatus->input_vic ) {
            case 5: case 6: case 7: case 10: case 11: case 20:
                //USBSerial_println ("VIC20 ");
                val_6060 |= 0x08;   lt8619c_i2c_write (0x68, 23);
                break;
            case 21: case 22: case 25: case 26:
                //USBSerial_println ("VIC26 ");
                val_6060 |= 0x08;   lt8619c_i2c_write (0x68, 25);
                break;
            default:
                lt8619c_i2c_write (0x68, 0x00);
                break;
        }
    } else {
        //dvi input
        if ( (hActive == 1920) && (vActive == 540) ) {
            val_6060 |= 0x08;   lt8619c_i2c_write (0x68, 23);
        } else if ( (hActive == 1440) && (vActive == 240) ) {
            val_6060 |= 0x08;   lt8619c_i2c_write (0x68, 23);
        } else if ( (hActive == 1440) && (vActive == 288) ) {
            val_6060 |= 0x08;   lt8619c_i2c_write (0x68, 25);
        }
    }
    if (val_6060 & 0x08) {
        vActive <<= 1;
        if ((vTotal % 2))   vTotal = (vTotal<<1) -1;
        else                vTotal = (vTotal<<1) +1;
    }

    lt8619c_i2c_write (0x60, val_6060);
    tmp = hSyncWidth + hBackPorch;
    lt8619c_i2c_write (0x61, (uint8_t)(tmp>>8));   lt8619c_i2c_write (0x62, (uint8_t) tmp);
    tmp = hActive;
    lt8619c_i2c_write (0x63, (uint8_t)(tmp>>8));   lt8619c_i2c_write (0x64, (uint8_t) tmp);
    tmp = hTotal;
    lt8619c_i2c_write (0x65, (uint8_t)(tmp>>8));   lt8619c_i2c_write (0x66, (uint8_t) tmp);

    tmp = vSyncWidth + vBackPorch;
    lt8619c_i2c_write (0x67, (uint8_t) tmp);
    tmp = vActive;
    lt8619c_i2c_write (0x69, (uint8_t)(tmp>>8));   lt8619c_i2c_write (0x6A, (uint8_t) tmp);
    tmp = vTotal;
    lt8619c_i2c_write (0x6B, (uint8_t)(tmp>>8));   lt8619c_i2c_write (0x6C, (uint8_t) tmp);
}

/*---------------------------------------------------------------------------*/
void        lt8619c_plllock_detect  (void)
{
    uint8_t rd, cnt = 0;

    lt8619c_i2c_write (0xFF, 0x60);
    if ((lt8619c_i2c_read (0xA3) & 0x40) == 0x40) {
        lt8619c_i2c_write (0xFF, 0x80);
        while ((lt8619c_i2c_read (0x87) & 0x20) == 0x00) {
            lt8619c_i2c_write(0xFF,0x60);
            rd = lt8619c_i2c_read (0x0E);
            lt8619c_i2c_write (0x0E, rd & 0xFD);    delay (5);
            lt8619c_i2c_write (0x0E, 0xFF);
            lt8619c_i2c_write (0xFF, 0x80);

            if(cnt++ > 10)  break;
        }
    }
}

/*---------------------------------------------------------------------------*/
bool        lt8619c_lvds_check      (void)
{
    uint16_t i, x = 0;

    for ( i = 0; i < 200; i++ )
    {
        lt8619c_i2c_write (0xFF, 0x60);
        if ((lt8619c_i2c_read (0x92) & 0x01) == 0) x++;
    }
    if ((x == 200) || (x == 0)) return false;

    return true;
}

/*---------------------------------------------------------------------------*/
void        lt8619c_lvds_detect     (void)
{
    uint8_t rd, cnt = 0;

    do {
        if (!lt8619c_lvds_check ()) {
            lt8619c_i2c_write (0xFF, 0x60);
            rd = lt8619c_i2c_read (0x0D);
            lt8619c_i2c_write (0x0D, rd & 0xF8);   // reset LVDS/BT fifo
            delay (10);
            lt8619c_i2c_write (0x0D, rd | 0x06);
            lt8619c_i2c_write (0x0D, rd | 0x01);
        }
        if (cnt++ > 4) {
            memset (pLT8619C_RXStatus, 0, sizeof(LT8619C_RXStatus));
            break;
        }
    }
    while (!lt8619c_lvds_check ());
}

/*---------------------------------------------------------------------------*/
bool        lt8619c_loop            (void)
{
    bool status = false;

    if ( (status = lt8619c_clk_detect ()) ) {
        delay (50);
        lt8619c_get_info ();
        lt8619c_csc_conv ();
        lt8619c_video_check ();
        lt8619c_bt_setting ();
        // if (LT8619C_OUTPUTMODE ==  OUTPUT_LVDS_2_PORT)
        {
           lt8619c_lvds_detect ();
        }
    }
    return status;
}

/*---------------------------------------------------------------------------*/
bool        lt8619c_check_id        (void)
{
    lt8619c_i2c_write (0xFF, 0x60);

    if ( (lt8619c_i2c_read (0x00) == 0x16) && (lt8619c_i2c_read (0x01) == 0x04) )
        return true;

    USBSerial_print ("ERROR : LT8619C not found!\r\n");
    return false;
}

/*---------------------------------------------------------------------------*/
void        lt8619c_init            (void)
{
    pLT8619C_RXStatus = &LT8619C_RXStatus;
    memset(pLT8619C_RXStatus, 0, sizeof(LT8619C_RXStatus));

    lt8619c_reset ();   //lt8619c hardware reset
    delay (10);
    lt8619c_check_id(); //read chip id:0x16 0x04

    lt8619c_set_hpd (false);

    lt8619c_edid_set ((uint8_t *)ONCHIP_EDID);
    delay (300);
    lt8619c_set_hpd (true);

#if defined (USE_EXTERNAL_HDCPKEY)
    lt8619c_load_hdcpkey ();
#endif

    lt8619c_rx_init ();
    lt8619c_audio_init();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/








