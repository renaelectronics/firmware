/************************************************************************
*
* Serial Bootloader example application for PIC18xxx devices using the
* HI-TECH C compiler.
*
*************************************************************************
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
* E. Schlunder  2009/07/16  PIC18 example application code for 
*                           serial bootloader using HITECH PIC C.
************************************************************************/

#include <htc.h>
#include <stdio.h>

// BAUDRG is calculated as  = Fosc / (4 * Desired Baudrate) - 1
// So, 4MHz / (4 * 19200) - 1 = 52 (approx.)
//#define BAUDRG 51                   // 19.2Kbps from 4MHz INTOSC
//#define BAUDRG 103                  // 115.2Kbps from 48MHz
//#define BAUDRG 89                   // 115.2Kbps from 41.66MHz
#define BAUDRG 85                   // 115.2Kbps from 40MHz
//#define BAUDRG 68                   // 115.2Kbps from 32MHz
//#define BAUDRG 16                   // 115.2Kbps from 8MHz
//#define BAUDRG 11                   // 1Mbps from 48MHz
//#define BAUDRG 9                    // 1Mbps from 40MHz
//#define BAUDRG 4                    // 2Mbps from 40MHz
//#define BAUDRG 3                    // 3Mbps from 48MHz
//#define BAUDRG 1                    // 6Mbps from 48MHz
#ifndef BAUDRG
    #define BAUDRG brg              // use bootloader autobaud BRG if constant not provided
#endif
//#define INVERT_UART                 // If you don't have an RS232 transceiver, you might want this option

void putch(unsigned char byte);

//__EEPROM_DATA(0, 1, 2, 3, 4, 5, 6, 7, 0x80, 0x7F);
void interrupt isr(void)
{
    LATD1 ^= 1;                         // blink LED
    TMR1IF = 0;                         // clear Timer 1 interrupt flag
}

void interrupt low_priority low_isr(void)
{
    if(RCIF)
    {
#ifdef INVERT_UART
        if(FERR && RC7)
#else
        if(FERR && RC7 == 0)
#endif
        {
            // receiving BREAK state, soft reboot back into Bootloader mode.
            RESET();
        }

        TXREG = RCREG;                  // echo received serial data back
        LATD ^= 0xF0;                   // blink LEDs
    }

    if(TMR0IF)
    {
        LATD0 ^= 1;                     // blink an LED
        TMR0IF = 0;                     // clear Timer 0 interrupt flag
    }
}

#ifndef SPBRGH
    #define SPBRGH SPBRGH1  // PIC18F8722
    #define BRG16 BRG161
    #define TXIF TX1IF
#endif

persistent unsigned int brg;
void main(void)
{
    PLLEN = 1;              // enable PLL for higher speed execution
#ifdef INVERT_UART
    LATC6 = 0;              // drive TXD pin low for RS-232 IDLE state
#else
    LATC6 = 1;              // drive TXD pin high for RS-232 IDLE state
#endif
    TRISC6 = 0;

    if(POR == 0)
    {
        // Power On Reset occurred, bootloader did not capture an autobaud BRG value.
        brg = 0;
        POR = 1;            // flag that future MCLR/soft resets are not POR resets
    }
    else
    {
        if(SPBRGH || SPBRG)
        {
            // save bootloader autobaud BRG value for display later.
            brg = SPBRGH;
            brg <<= 8;
            brg |= SPBRG;
        }
    }

    TMR0H = 0;
    TMR0L = 0;
    T0CON = 0b00010110;
    TMR0IF = 0;
    TMR0IE = 1;
    TMR0IP = 0;

    TMR1H = 0;
    TMR1L = 0;
    T1CON = 0b00110111;
    TMR1CS = 0;                 // internal clock (Fosc/4)
    TMR1IF = 0;
    TMR1IE = 1;
    TMR1IP = 1;                 // Timer 1 as High Priority interrupt

    RCONbits.IPEN = 1;          // enable interrupt priority
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;

    TXSTA = 0b00100100;         // Async, 8 bit, BRGH
    BRG16 = 1;
#ifdef INVERT_UART
    BAUDCON1 |= 0b00110000;     // invert UART signals
#endif
    SPBRG = (BAUDRG & 0xFF);
    SPBRGH = (BAUDRG >> 8);
    RCSTA = 0b10010000;

    RC1IP = 0;                  // USART RX interrupt as low priority interrupt
    RC1IE = 1;                  // enable USART RX interrupt

    LATD = 0;                   // turn off LEDs
    TRISD = 0;                  // enable PORTD LED outputs

    // wait for stable clock and host connection by waiting for first Timer 0 interrupt
    TMR0ON = 1;       
    while(LATD0 == 0); 

    if(brg != 0)
    {
        printf("Bootloader BRG: %d\r\n", brg);
    }
    printf("\r\nHello world from PIC microcontroller!\r\n");

    // infinite loop, interrupts will blink PORTD pins and handle UART communications.
    while(1)
    {
    }
}

void putch(unsigned char byte)
{
    while(TXIF == 0)
    {
        // spin until done transmitting
    }

    TXREG = byte;
}
