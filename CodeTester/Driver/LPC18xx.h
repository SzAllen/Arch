
/****************************************************************************************************//**
 * @file     LPC18xx.h
 *
 * @status   RELEASE
 *
 * @brief    CMSIS Cortex-M3 Core Peripheral Access Layer Header File for
 *           default LPC18xx Device Series
 *
 * @version  V19
 * @date     9. December 2011
 * 
 * @note     Generated with SVDConv V2.6 Build 6c  on Friday, 09.12.2011 13:49:19
 *
 *           from CMSIS SVD File 'LPC18xxv19.xml' Version 19,
 *           created on Friday, 09.12.2011 21:49:13, last modified on Friday, 09.12.2011 21:49:13
 *
 *******************************************************************************************************/



/** @addtogroup Template
  * @{
  */

#ifndef __LPC18XX_H__
#define __LPC18XX_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "lpc_types.h"
#include "typedef.h"

typedef struct _LPC_USARTn_Type
{                            /*!< (@ 0x400xx000) USARTn Structure       */

  union {
     uint32 DLL;                      /*!< (@ 0x400xx000) Divisor Latch LSB. Least significant byte of the baud rate divisor value. The full divisor is used to generate a baud rate from the fractional rate divider (DLAB = 1). */
      uint32 THR;                      /*!< (@ 0x400xx000) Transmit Holding Register. The next character to be transmitted is written here (DLAB = 0). */
      uint32 RBR;                      /*!< (@ 0x400xx000) Receiver Buffer Register. Contains the next received character to be read (DLAB = 0). */
  };

  union {
     uint32 IER;                      /*!< (@ 0x400xx004) Interrupt Enable Register. Contains individual interrupt enable bits for the 7 potential UART interrupts (DLAB = 0). */
     uint32 DLM;                      /*!< (@ 0x400xx004) Divisor Latch MSB. Most significant byte of the baud rate divisor value. The full divisor is used to generate a baud rate from the fractional rate divider (DLAB = 1). */
  };

  union {
      uint32 FCR;                      /*!< (@ 0x400xx008) FIFO Control Register. Controls UART FIFO usage and modes. */
      uint32 IIR;                      /*!< (@ 0x400xx008) Interrupt ID Register. Identifies which interrupt(s) are pending. */
  };
   uint32 LCR;                        /*!< (@ 0x400xx00C) Line Control Register. Contains controls for frame formatting and break generation. */
    uint32 RESERVED0[1];
    uint32 LSR;                        /*!< (@ 0x400xx014) Line Status Register. Contains flags for transmit and receive status, including line errors. */
    uint32 RESERVED1[1];
   uint32 SCR;                        /*!< (@ 0x400xx01C) Scratch Pad Register. Eight-bit temporary storage for software. */
   uint32 ACR;                        /*!< (@ 0x400xx020) Auto-baud Control Register. Contains controls for the auto-baud feature. */
   uint32 ICR;                        /*!< (@ 0x400xx024) IrDA control register (UART3 only) */
   uint32 FDR;                        /*!< (@ 0x400xx028) Fractional Divider Register. Generates a clock input for the baud rate divider. */
   uint32 OSR;                        /*!< (@ 0x400xx02C) Oversampling Register. Controls the degree of oversampling during each bit time. */
    uint32 RESERVED2[4];
   uint32 HDEN;                       /*!< (@ 0x400xx03C) Half-duplex enable Register */
    uint32 RESERVED3[1];
   uint32 SCICTRL;                    /*!< (@ 0x400xx048) Smart card interface control register */
   uint32 RS485CTRL;                  /*!< (@ 0x400xx04C) RS-485/EIA-485 Control. Contains controls to configure various aspects of RS-485/EIA-485 modes. */
   uint32 RS485ADRMATCH;              /*!< (@ 0x400xx050) RS-485/EIA-485 address match. Contains the address match value for RS-485/EIA-485 mode. */
   uint32 RS485DLY;                   /*!< (@ 0x400xx054) RS-485/EIA-485 direction control delay. */
   uint32 SYNCCTRL;                   /*!< (@ 0x400xx058) Synchronous mode control register. */
   uint32 TER;                        /*!< (@ 0x400xx05C) Transmit Enable Register. Turns off UART transmitter for use with software flow control. */
} LPC_USARTn_Type;

#ifdef __cplusplus
}
#endif


#endif  // __LPC18XX_H__

