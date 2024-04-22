/*---------------------------------------------------------------------------*/
/**
 * @file protocol.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief VU12 Serial protocol control
 * @version 0.1
 * @date 2024-02-16
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "Arduino.h"

#include "vu12_fw.h"
#include "gpio_i2c.h"
#include "tass805m.h"
#include "backlight.h"
#include "protocol.h"
#include "eeprom.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void protocol_data_send     (char cmd, uint8_t data);
void protocol_data_check    (void);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define PROTOCOL_SIZE   6
unsigned char Protocol[PROTOCOL_SIZE];

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void protocol_data_send     (char cmd, uint8_t data)
{
    char f, m, l;

#if defined(_DEBUG_PROTOCOL_)
    printf ("Received cmd : %c, Response data: %d (0x%02x)\r\n",
                cmd, data, data);
#endif
    f = ('0' + (data / 100));   data %= 100;
    m = ('0' + (data / 10 ));   data %= 10 ;
    l = ('0' + (data      ));

    USBSerial_print("%c%c%c%c%c%c\r\n", '@', cmd, f, m, l, '#');
}

/*---------------------------------------------------------------------------*/
void protocol_data_check    (void)
{
    char i;
    /* protocol q */
    for (i = 0; i < PROTOCOL_SIZE-1; i++)   Protocol[i] = Protocol[i+1];
    Protocol[PROTOCOL_SIZE-1] = USBSerial_read();

#if defined(_DEBUG_PROTOCOL_)
    printf ("%c", (char)Protocol[PROTOCOL_SIZE-1]);
    if ((char)Protocol[PROTOCOL_SIZE-1] == '\r')
        printf ("\n");
    fflush (stdout);
#endif
    /* Header & Tail check */
    if ((Protocol[0] == '@') && (Protocol[PROTOCOL_SIZE-1] == '#')) {
#if defined(_DEBUG_PROTOCOL_)
    printf ("\r\n");
#endif
        uint8_t data =
            (Protocol[2] - '0') * 100 + (Protocol[3] - '0') * 10 + Protocol[4] - '0';

        switch (Protocol[1]) {
            /* Digital volume request */
            case    'D':
                if (eeprom_cfg_write (Protocol[1], Protocol[2], data))
                    tass805m_write (CODEC_REG_DGAIN, &DigitalVolume);
                data = DigitalVolume;
                break;
            /* Analog volume request */
            case    'A':
                if (eeprom_cfg_write (Protocol[1], Protocol[2], (data > 0x1F) ? 0x1F : data))
                    tass805m_write (CODEC_REG_AGAIN, &AnalogVolume);
                data = AnalogVolume;
                break;
            /* Brightness value request */
            case    'B':
                if (eeprom_cfg_write (Protocol[1], Protocol[2], data))
                    backlight_control (Brightness);
                data = Brightness;
                break;

            /* Firmware Version request */
            case    'F':
                USBSerial_println("@%s#", _FW_VERSION_STR_);    // Makefile
#if defined(_DEBUG_PROTOCOL_)
                printf ("cmd : %c, @%s#",Protocol[1], _FW_VERSION_STR_);
#endif
                return;
            /* System self reset (watchdog timout reset) */
            case    'R':
            /* Factory Init, Default config set */
            case    'I':
                if (Protocol[1] == 'I')
                    eeprom_init (true);

#if defined(_DEBUG_PROTOCOL_)
                printf ("cmd : %c, %s\r\n",
                    Protocol[1], Protocol[1] == 'I' ? "EEPROM Init & Reboot" : "Reboot");
#endif
                USBSerial_println(PROTOCOL_RESET_STR);
                // watchdog reset (watch set 1 sec)
                watchdog_setup (WDT_RELOAD_1_S);
                USBSerial_flush();  while (1);
                break;

            /* Touch controller reset */
            case    'T':
                touch_reset (data);
                break;
            default:
#if defined(_DEBUG_PROTOCOL_)
                printf ("Unknown command!\r\n");
#endif
                return;
        }
        protocol_data_send (Protocol[1], data);
    }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
