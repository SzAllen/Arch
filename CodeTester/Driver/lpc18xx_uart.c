/**********************************************************************
* $Id$		lpc18xx_uart.c		2011-06-02
*//**
* @file		lpc18xx_uart.c
* @brief	Contains all functions support for UART firmware library on LPC18xx
* @version	1.0
* @date		02. June. 2011
* @author	NXP MCU SW Application Team
*
* Copyright(C) 2011, NXP Semiconductor
* All rights reserved.
*
***********************************************************************
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
* Permission to use, copy, modify, and distribute this software and its
* documentation is hereby granted, under NXP Semiconductors'
* relevant copyright in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
**********************************************************************/

/* Peripheral group ----------------------------------------------------------- */
/** @addtogroup UART
 * @{
 */

/* Includes ------------------------------------------------------------------- */
//#include <includes.h>
//#include "lpc18xx_scu.h"
#include "lpc18xx_uart.h"
//#include "lpc18xx_cgu.h"

/* If this source file built with example, the LPC18xx FW library configuration
 * file in each example directory ("lpc18xx_libcfg.h") must be included,
 * otherwise the default FW library configuration file must be included instead
 */

//#include "lpc18xx_libcfg.h"


void UART_TxCmd(LPC_USARTn_Type *UARTx, FunctionalState NewState)
{
}

uint32 UART_Receive(LPC_USARTn_Type *UARTx, uint8 *rxbuf, uint32 buflen, TRANSFER_BLOCK_Type flag)
{
	return buflen;
}

/*********************************************************************//**
 * @brief		Send a block of data via UART peripheral
 * @param[in]	UARTx	Selected UART peripheral used to send data, should be:
 * 					- LPC_UART0	:UART0 peripheral
 * 					- LPC_UART1	:UART1 peripheral
 * 					- LPC_UART2	:UART2 peripheral
 * 					- LPC_UART3	:UART3 peripheral
 * @param[in]	txbuf 	Pointer to Transmit buffer
 * @param[in]	buflen 	Length of Transmit buffer
 * @param[in] 	flag 	Flag used in  UART transfer, should be
 * 					- NONE_BLOCKING
 * 					- BLOCKING
 * @return 		Number of bytes sent.
 *
 * Note: when using UART in BLOCKING mode, a time-out condition is used
 * via defined symbol UART_BLOCKING_TIMEOUT.
 **********************************************************************/

LPC_USARTn_Type* LPC_USART0,*LPC_USART1,*LPC_USART2,*LPC_USART3;

uint32_t UART_Send(LPC_USARTn_Type *UARTx, uint8_t *txbuf, uint32_t buflen, TRANSFER_BLOCK_Type flag)
{
	return 1;
}

void openRS422_orRs232(uint8 flag)
{
}

void scu_pinmux(uint8_t port, uint8_t pin, uint8_t mode, uint8_t func)
{
}


