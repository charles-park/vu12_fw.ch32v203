/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_endp.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/08/08
 * Description        : Endpoint routines
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "hw_config.h"
#include "usb_istr.h"
#include "usb_pwr.h"
#include "usb_prop.h"
//#include "UART.h"

#include <string.h>
#include <stdarg.h>

uint8_t USBD_Endp3_Busy = 0;
uint16_t USB_Rx_Cnt=0;

/*********************************************************************
 * @fn      EP2_IN_Callback
 *
 * @brief  Endpoint 1 IN.
 *
 * @return  none
 */
void EP1_IN_Callback (void)
{

}

/*********************************************************************
 * @fn      EP2_OUT_Callback
 *
 * @brief  Endpoint 2 OUT.
 *
 * @return  none
 */
uint8_t USBSerialSP = 0, USBSerialEP = 0;
uint8_t USBSerialBuffer [DEF_USBD_MAX_PACK_SIZE * 2] = {0,};

uint8_t USBSerial_read (void)
{
	uint8_t ch = USBSerialBuffer [USBSerialSP];
	USBSerialSP++;	USBSerialSP %= sizeof (USBSerialBuffer);
	return	ch;
}

uint16_t USBSerial_available (void)
{
	if (USBSerialSP != USBSerialEP) {
		if (USBSerialSP > USBSerialEP)
			return (USBSerialEP + sizeof(USBSerialBuffer) - USBSerialSP);
		else
			return (USBSerialEP - USBSerialSP);
	}
	return 0;
}

void EP2_OUT_Callback (void)
{
	uint32_t len;
	uint8_t buf[64];

	len = GetEPRxCount ( EP2_OUT & 0x7F );
	PMAToUserBufferCopy ( buf, GetEPRxAddr( EP2_OUT & 0x7F ), len );

	// copy usb serial buffer from usb user buffer.
	{
		uint16_t i;
		for (i = 0; i < len; i++) {
			USBSerialBuffer [USBSerialEP] = buf [i];
			USBSerialEP++;	USBSerialEP %= sizeof (USBSerialBuffer);
			if (USBSerialEP == USBSerialSP) {
				USBSerialSP++;	USBSerialSP %= sizeof (USBSerialBuffer);
			}
		}
	}
//	USBD_ENDPx_DataUp( ENDP3, buf, len); // loop back
	SetEPRxValid( ENDP2 );
}

/*********************************************************************
 * @fn      EP3_IN_Callback
 *
 * @brief  Endpoint 3 IN.
 *
 * @return  none
 */
uint8_t USBSerialWrSP = 0, USBSerialWrEP = 0;
uint8_t USBSerialWrBuffer [DEF_USBD_MAX_PACK_SIZE * 2] = {0,};

void EP3_IN_Callback (void)
{
	USBD_Endp3_Busy = 0;
	USBSerialWrSP++;	USBSerialWrSP %= sizeof (USBSerialWrBuffer);
	if (USBSerialWrSP != USBSerialWrEP) {
		USBD_ENDPx_DataUp( ENDP3, &USBSerialWrBuffer[USBSerialWrSP], 1 );
	}
	//Uart.USB_Up_IngFlag = 0x00;
}

/*********************************************************************
 * @fn      USBD_ENDPx_DataUp
 *
 * @brief  USBD ENDPx DataUp Function
 *
 * @param   endp - endpoint num.
 *          *pbuf - A pointer points to data.
 *          len - data length to transmit.
 *
 * @return  data up status.
 */
uint8_t USBD_ENDPx_DataUp( uint8_t endp, uint8_t *pbuf, uint16_t len )
{
	if( endp == ENDP3 )
	{
		if (USBD_Endp3_Busy)
		{
			return USB_ERROR;
		}
		USB_SIL_Write( EP3_IN, pbuf, len );
		USBD_Endp3_Busy = 1;
		SetEPTxStatus( ENDP3, EP_TX_VALID );
	}
	else
	{
		return USB_ERROR;
	}
	return USB_SUCCESS;
}

uint16_t USBSerial_print (char *fmt, ...)
{
	uint8_t msg [DEF_USBD_MAX_PACK_SIZE];
	va_list va;
	uint16_t str_cnt, i;


	memset (msg, 0, sizeof(msg));

	va_start (va, fmt);
	vsprintf (msg, fmt, va);
	va_end (va);

	str_cnt = strlen(msg);

	for (i = 0; i < str_cnt; i++) {
		USBSerialWrBuffer [USBSerialWrEP] = msg [i];
		USBSerialWrEP++;	USBSerialWrEP %= sizeof (USBSerialWrBuffer);
		if (USBSerialWrSP == USBSerialWrEP) {
			USBSerialWrSP++;	USBSerialWrSP %= sizeof (USBSerialWrBuffer);
		}
	}
	if (!USBD_Endp3_Busy)
		USBD_ENDPx_DataUp( ENDP3, &USBSerialWrBuffer[USBSerialWrSP], 1);
	return str_cnt;
}

uint16_t USBSerial_println (char *fmt, ...)
{
	uint16_t str_cnt;
	str_cnt  = USBSerial_print (fmt);
	str_cnt += USBSerial_print ("\r\n");
	return str_cnt;
}

void USBSerial_flush (void)
{
	/* wait sp == ep */
	while (USBSerialWrEP != USBSerialWrSP);
}