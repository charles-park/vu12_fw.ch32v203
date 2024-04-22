/*---------------------------------------------------------------------------*/
/**
 * @file eeprom.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief VU12 Config data control
 * @version 0.1
 * @date 2024-02-16
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "Arduino.h"
#include "vu12_fw.h"
#include "eeprom.h"

#include <string.h>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint32_t    flash_page_addr     (uint32_t page);
uint8_t     eeprom_init         (bool force_init);
void        eeprom_cfg_read     (uint32_t page_addr);
uint8_t     eeprom_cfg_write    (char cfg_type, uint8_t val);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint32_t WriteCfgPage = 0;

/*---------------------------------------------------------------------------*/
uint32_t flash_page_addr (uint32_t page)
{
    return (FLASH_CFG_ADDR + page * FLASH_PAGE_SIZE);
}

/*---------------------------------------------------------------------------*/
void eeprom_read (uint32_t page_addr, uint32_t *r_buf)
{
    for (int32_t i = 0; i < FLASH_PAGE_SIZE / sizeof(uint32_t); i++) {
        r_buf [i] = (*(uint32_t *)(page_addr + (i * 4)));
    }
#if defined(_DEBUG_EEPROM_)
    printf ("%s : page addr = 0x%08X\r\n", __func__, page_addr);

    for (int32_t i = 0; i < FLASH_PAGE_SIZE / sizeof(uint32_t); i++) {
        if (i && ((i % 8) == 0))    printf ("\r\n");
        printf ("0x%08X ", r_buf[i]);
    }
    printf ("\r\n");
#endif
}

/*---------------------------------------------------------------------------*/
void eeprom_write (uint32_t page_addr, uint32_t *w_buf)
{
    FLASH_Unlock_Fast ();
    FLASH_ErasePage_Fast   (page_addr);
    FLASH_ProgramPage_Fast (page_addr, w_buf);
    FLASH_Lock_Fast ();
#if defined(_DEBUG_EEPROM_)
    printf ("%s : page addr = 0x%08X\r\n", __func__, page_addr);

    for (int32_t i = 0; i < FLASH_PAGE_SIZE / sizeof(uint32_t); i++) {
        if (i && ((i % 8) == 0))    printf ("\r\n");
        printf ("0x%08X ", w_buf[i]);
    }
    printf ("\r\n");
#endif
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void eeprom_cfg_read    (uint32_t page_addr)
{
    uint32_t rw_buf[FLASH_PAGE_SIZE / sizeof(uint32_t)];

    memset (rw_buf, 0, sizeof(rw_buf));
    eeprom_read (page_addr, rw_buf);

    DigitalVolume   = rw_buf [OFFSET_D_VOL];
    AnalogVolume    = rw_buf [OFFSET_A_VOL];
    Brightness      = rw_buf [OFFSET_B_VAL];
}

/*---------------------------------------------------------------------------*/
uint8_t eeprom_init     (bool force_init)
{
    uint32_t rw_buf[FLASH_PAGE_SIZE / sizeof(uint32_t)];

    memset (rw_buf, 0, sizeof(rw_buf));
    eeprom_read (FLASH_CFG_ADDR, rw_buf);
    if (rw_buf[OFFSET_SIGNATURE] == CFG_SIGNATURE)
        WriteCfgPage = rw_buf[OFFSET_CFG_PAGE];
    else
        WriteCfgPage = 0;

    if (!WriteCfgPage || force_init) {
        memset (rw_buf, 0, sizeof(rw_buf));
        rw_buf [OFFSET_SIGNATURE] = CFG_SIGNATURE;
        WriteCfgPage = WriteCfgPage ? WriteCfgPage : 1;
        rw_buf [OFFSET_CFG_PAGE]  = WriteCfgPage;
        eeprom_write (FLASH_CFG_ADDR, rw_buf);

        memset (rw_buf, 0, sizeof(rw_buf));
        rw_buf [OFFSET_D_VOL] = D_VOL_DEFAULT;
        rw_buf [OFFSET_A_VOL] = A_VOL_DEFAULT;
        rw_buf [OFFSET_B_VAL] = B_VAL_DEFAULT;
        eeprom_write (flash_page_addr(WriteCfgPage), rw_buf);
        /* eeprom error or force init */
        printf ("%s(%d) page_addr = 0x%08x\r\n",
                __func__, __LINE__, flash_page_addr(WriteCfgPage));
    }
    eeprom_cfg_read (flash_page_addr(WriteCfgPage));
    return (flash_page_addr(WriteCfgPage) < FLASH_END_ADDR) ? 1 : 0;
}

/*---------------------------------------------------------------------------*/
uint8_t eeprom_cfg_write    (char cfg_type, uint8_t val)
{
    uint32_t rw_buf[FLASH_PAGE_SIZE / sizeof(uint32_t)];

    memset (rw_buf, 0, sizeof(rw_buf));
    eeprom_read (flash_page_addr(WriteCfgPage), rw_buf);

    // read only check
    if (flash_page_addr(WriteCfgPage) >= FLASH_END_ADDR)
        return 0;

    switch (cfg_type) {
        case    'D':    rw_buf [OFFSET_D_VOL] = DigitalVolume  = val;   break;
        case    'A':    rw_buf [OFFSET_A_VOL] = AnalogVolume   = val;   break;
        case    'B':    rw_buf [OFFSET_B_VAL] = Brightness     = val;   break;
        default:
            return 0;
    }
    eeprom_write (flash_page_addr(WriteCfgPage), rw_buf);

    memset (rw_buf, 0, sizeof(rw_buf));
    eeprom_read (flash_page_addr(WriteCfgPage), rw_buf);

    if ((rw_buf [OFFSET_D_VOL] != DigitalVolume) ||
        (rw_buf [OFFSET_A_VOL] != AnalogVolume)  ||
        (rw_buf [OFFSET_B_VAL] != Brightness)) {
        if (flash_page_addr(WriteCfgPage +1) < FLASH_END_ADDR) {
            WriteCfgPage += 1;
            memset (rw_buf, 0, sizeof(rw_buf));
            rw_buf [OFFSET_SIGNATURE] = CFG_SIGNATURE;
            rw_buf [OFFSET_CFG_PAGE]  = WriteCfgPage;
            eeprom_write (FLASH_CFG_ADDR, rw_buf);

            memset (rw_buf, 0, sizeof(rw_buf));
            rw_buf [OFFSET_D_VOL] = DigitalVolume;
            rw_buf [OFFSET_A_VOL] = AnalogVolume;
            rw_buf [OFFSET_B_VAL] = Brightness;
            eeprom_write (flash_page_addr(WriteCfgPage), rw_buf);
            return 1;
        }
        return 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
