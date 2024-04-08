/*---------------------------------------------------------------------------*/
/**
 * @file eeprom.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief VU12 Config data control
 * @version 0.1
 * @date 2024-02-16
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifndef __EEPROM_H__
#define __EEPROM_H__

/*---------------------------------------------------------------------------*/
#define FLASH_PAGE_SIZE     0x100   /* 256 bytes */
#define FLASH_CFG_ADDR      0x08007800
#define FLASH_END_ADDR      0x08008000

#define CFG_SIGNATURE       0xA5A55A5A

// 0x08007000 config signature.
#define OFFSET_SIGNATURE    0
// 0x08007004 current cfg page
#define OFFSET_CFG_PAGE     1

#define OFFSET_D_VOL        0
#define OFFSET_A_VOL        1
#define OFFSET_B_VAL        2

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
#define EEPROM_CFG_OFFSET(x)    (x * EEPROM_CFG_SIZE + EEPROM_CFG_START)

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern  uint8_t eeprom_init     (bool force_init);
extern  void    eeprom_cfg_read (uint32_t page_addr);
extern  uint8_t eeprom_cfg_write(char data, char read, uint8_t val);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#endif  // #define __EEPROM_H__
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/


