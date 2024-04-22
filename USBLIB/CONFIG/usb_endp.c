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

#include "Arduino.h"
#include <string.h>
#include <stdarg.h>

/*---------------------------------------------------------------------------*/
uint8_t USBD_Endp3_Busy = 0;

/* 256 bytes */
uint8_t	USBSerialRxBuffer[DEF_USBD_MAX_PACK_SIZE * 4] = {0,};
uint16_t USBSerialRxSP = 0, USBSerialRxEP = 0;

/* 256 bytes */
uint8_t	USBSerialTxBuffer[DEF_USBD_MAX_PACK_SIZE * 4] = {0,};
uint16_t USBSerialTxSP = 0, USBSerialTxEP = 0;

/*---------------------------------------------------------------------------*/
void USBSerialRxBufferWrite (uint8_t *buf, uint16_t len)
{
	uint16_t i;
	for (i = 0; i < len; i++) {
		USBSerialRxBuffer [USBSerialRxEP] = buf [i];
		USBSerialRxEP++;	USBSerialRxEP %= sizeof (USBSerialRxBuffer);
		if (USBSerialRxEP == USBSerialRxSP) {
			USBSerialRxSP++;	USBSerialRxSP %= sizeof (USBSerialRxBuffer);
		}
	}
}

/*---------------------------------------------------------------------------*/
void USBSerialTxBufferWrite (uint8_t *buf, uint16_t len)
{
	int16_t i;
	for (i = 0; i < len; i++) {
		USBSerialTxBuffer [USBSerialTxEP] = buf [i];
		USBSerialTxEP++;	USBSerialTxEP %= sizeof (USBSerialTxBuffer);
		if (USBSerialTxSP == USBSerialTxEP) {
			USBSerialTxSP++;	USBSerialTxSP %= sizeof (USBSerialTxBuffer);
		}
	}
}

/*---------------------------------------------------------------------------*/
uint8_t USBSerialTxBufferCheck (void)
{
	if ((USBSerialTxSP != USBSerialTxEP) && (!USBD_Endp3_Busy)) {
		USBD_ENDPx_DataUp( ENDP3, &USBSerialTxBuffer[USBSerialTxSP], 1 );
		USBSerialTxSP++;	USBSerialTxSP %= sizeof (USBSerialTxBuffer);
	}
	return USBD_Endp3_Busy;
}

/*---------------------------------------------------------------------------*/
uint16_t USBSerialRxBufferCheck (void)
{
	if (USBSerialRxSP != USBSerialRxEP) {
		if (USBSerialRxSP > USBSerialRxEP)
			return (USBSerialRxEP + sizeof(USBSerialRxBuffer) - USBSerialRxSP);
		else
			return (USBSerialRxEP - USBSerialRxSP);
	}
	return 0;
}

/*---------------------------------------------------------------------------*/
void USBSerial_flush (void)
{
	uint16_t w = 0;
	/* wait sp == ep */
	while ((w++ < 10) && (USBSerialTxEP != USBSerialTxSP)) {
		if (USBSerialTxBufferCheck ())	udelay(100);
		else							w = 0;
	}
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t USBSerial_read (void)
{
	uint8_t ch = USBSerialRxBuffer [USBSerialRxSP];
	USBSerialRxSP++;	USBSerialRxSP %= sizeof (USBSerialRxBuffer);
	return	ch;
}

/*---------------------------------------------------------------------------*/
uint16_t USBSerial_available (void)
{
	// Tx buffer check
	USBSerialTxBufferCheck ();

	// Rx buffer check
	return USBSerialRxBufferCheck();
}

/*---------------------------------------------------------------------------*/
uint16_t USBSerial_print (char *fmt, ...)
{
	uint8_t buf [DEF_USBD_MAX_PACK_SIZE * 2];
	va_list va;
	uint16_t len;

	memset (buf, 0, sizeof(buf));

	va_start (va, fmt);
	vsprintf (buf, fmt, va);
	va_end (va);

	len = strlen(buf);

	USBSerialTxBufferWrite (buf, len);
	return len;
}

/*---------------------------------------------------------------------------*/
uint16_t USBSerial_println (char *fmt, ...)
{
	uint16_t len;
	len  = USBSerial_print (fmt);
	len += USBSerial_print ("\r\n");
	return len;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
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
void EP2_OUT_Callback (void)
{
	uint32_t len;
	uint8_t buf[64];

	len = GetEPRxCount ( EP2_OUT & 0x7F );
	PMAToUserBufferCopy ( buf, GetEPRxAddr( EP2_OUT & 0x7F ), len );

	// copy usb serial buffer from usb user buffer.
	USBSerialRxBufferWrite (buf, len);

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
void EP3_IN_Callback (void)
{
	USBD_Endp3_Busy = 0;
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
