/************************************************************************
* Copyright (c) 2009,  Microchip Technology Inc.
*
* Microchip licenses this software to you solely for use with Microchip
* products.  The software is owned by Microchip and its licensors, and
* is protected under applicable copyright laws.  All rights reserved.
*
* SOFTWARE IS PROVIDED "AS IS."  MICROCHIP EXPRESSLY DISCLAIMS ANY
* WARRANTY OF ANY KIND, WHETHER EXPRESS OR IMPLIED, INCLUDING BUT
* NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.  IN NO EVENT SHALL
* MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
* CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, HARM TO YOUR
* EQUIPMENT, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY
* OR SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED
* TO ANY DEFENSE THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION,
* OR OTHER SIMILAR COSTS.
*
* To the fullest extent allowed by law, Microchip and its licensors
* liability shall not exceed the amount of fees, if any, that you
* have paid directly to Microchip to use this software.
*
* MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE
* OF THESE TERMS.
*
* Author        Date        Comment
*************************************************************************
* E. Schlunder  2009/05/29  PIC16F example application code for 
*                           serial bootloader using HITECH PIC C.
************************************************************************/

#include <htc.h>
#include <stdio.h>

volatile unsigned char leds = 0;
void interrupt isr(void)
{
    static signed char tick_count = -50;
    T0IF = 0;
    if(tick_count++ > 20)
    {
        tick_count = 0;
        leds ^= 1;
        PORTB = leds;
    }
}

__EEPROM_DATA(0, 1, 2, 3, 4, 5, 6, 7);

void putch(unsigned char byte)
{
    while(TXIF == 0);       // spin until done transmitting
    TXREG = byte;
}

// For BRG16 mode:
// BAUDRG is calculated as  = Fosc / (4 * Desired Baudrate) - 1
// So, 4MHz / (4 * 19200) - 1 = 51 (approx.)
//#define USE_BRG16
//#define BAUDRG 51           // 19.2Kbps from 4MHz (BRG16 = 1)
//#define BAUDRG 33           // 115.2Kbps from 16MHz (BRG16 = 1)
//#define BAUDRG 33           // 115.2Kbps from 16MHz (BRG16 = 1)
//#define BAUDRG 42           // 115.2Kbps from 19.6608MHz (BRG16 = 1)
//#define BAUDRG 63           // 115.2Kbps from 29.4912MHz (BRG16 = 1)
//#define BAUDRG 68           // 115.2Kbps from 32MHz (BRG16 = 1)

// For non-BRG16 mode:
// BAUDRG is calculated as  = Fosc / (16 * Desired Baudrate) - 1
// So, 4MHz / (16 * 19200) - 1 = 12 (approx.)
//#define BAUDRG 12           // 115.2Kbps from 24MHz or 19.2Kbps from 4MHz (BRG16 = 0)
#define BAUDRG 10           // 115.2Kbps from 19.6608MHz (BRG16 = 0)

#ifndef BAUDRG
    #define BAUDRG brg      // use bootloader autobaud BRG if constant not provided
#endif

unsigned char data;
persistent unsigned int brg;
void main(void)
{
    TRISB = 0b11010000;
    PORTB = 0x0F;

    if(!POR)
    {
        // Power On Reset occurred, bootloader did not capture an autobaud BRG value.
        brg = 0;
        POR = 1;       // flag that future MCLR/soft resets are not POR resets
    }
    else
    {
#ifdef USE_BRG16
        if(SPBRG || SPBRGH)
        {
            // save bootloader autobaud BRG value for display later.
            brg = SPBRGH;
            brg <<= 8;
            brg |= SPBRG;
        }
#else
        if(SPBRG)
        {
            // save bootloader autobaud BRG value for display later.
            brg = SPBRG;
        }
#endif
    }

    SPBRG   = (BAUDRG & 0xFF);
#ifdef USE_BRG16
    BAUDCTL = 0;
    BRG16 = 1;
    SPBRGH = (BAUDRG >> 8);
#endif

    TXSTA   = 0b00100100;
    RCSTA   = 0b10010000;

    // configure timer 0 for maximum prescaler and enable interrupt
    TMR0 = 0;
    T0IE = 1;
    OPTION = 0b11010111;
    ei();

    // wait for stable clock and host connection by waiting for first Timer 0 interrupt
    while(leds == 0);

    if(brg != 0)
    {
        printf("Bootloader BRG: %d\r\n", brg);
    }
    printf("Hello world from PIC microcontroller!\r\n");
    while (1)
    {
        if(RCIF)
        {
            if(FERR && (RC7 == 0))
            {
                // RXD BREAK state detected, switch back to Bootloader mode.
                di();                   // disable interrupts
                #asm
                    clrf    _PCLATH     // jump back into bootloader
                    goto    0           // (must only be done from main() to avoid call stack buildup)
                #endasm
            }

            data = RCREG;
            putch(data);
            switch(data)
            {
                case '1':
                    leds ^= (1<<0);
                    break;
                case '2':
                    leds ^= (1<<1);
                    break;
                case '3':
                    leds ^= (1<<2);
                    break;
                default:
                    leds ^= (1<<3);
                    break;
            }

            PORTB = leds;
        }
    }    
}