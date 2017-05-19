/************************************************************************
* Copyright (c) 2010,  Microchip Technology Inc.
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
* E. Schlunder  2010/07/22  PIC12F example application code for 
*                           serial bootloader using HI-TECH PIC C.
************************************************************************/

// To force the HI-TECH C compiler to avoid placing application
// firmware code on top of bootloader firmware, we modified the 
// Project -> Build Options -> Global tab so that the bootloader 
// firmware region is excluded from application use: "default,-63B-800"
// 
// Refer to AN1310 application note documentation for full details.

#include <htc.h>
#include <stdio.h>

#define bitset(var, bit) ((var) |= 1 << (bit))
#define bitclr(var, bit) ((var) &= ~(1 << (bit)))

#define RX_PIN  0
#define TX_PIN  1
#define LED1_PIN 2       // GP1 pin will be toggled on and off continuously by main loop
#define LED2_PIN 3       // GP2 pin will be toggled on and off continuously by Timer 0 interrupt
//#define INVERT_UART       // if you don't have an RS-232 transceiver, you might want this.

void wait(void);

volatile unsigned exitApplication = 0;
volatile unsigned char leds = 0;

void interrupt isr(void)
{
    static signed char tick_count = -30;
    if(INTCON & (1<<2))     // interrupt on timer 0
    {
        bitclr(INTCON, 2);      // clear T0IF (timer 0 interrupt flag)
        if(tick_count++ > 10)
        {
            tick_count = 0;
            leds ^= (1<<LED2_PIN);  // blink LED 2
            GPIO = leds;
        }
    }

    if(INTCON & (1<<0))     // interrupt on change
    {
#ifdef INVERT_UART
        if((GPIO & (1<<RX_PIN)) != 0)
#else
        if((GPIO & (1<<RX_PIN)) == 0)
#endif
        {
            // Host PC might be requesting bootloader mode
            exitApplication = 1;
        }
        else
        {
            // Host PC is not requesting bootloader mode afterall
            exitApplication = 0;
        }
        bitclr(INTCON, 0);      // clear GPIF (GPIO interrupt on change flag)
    }
}

void main(void)
{
	// configure all pins as inputs, except the LED_PIN's and TX_PIN
	TRISIO = ~((1<<LED1_PIN) | (1<<LED2_PIN) | (1<<TX_PIN)); 
    IOC = (1<<RX_PIN);

    // configure timer 0 for maximum prescaler and enable interrupt
    TMR0 = 0;
    OPTION_REG = 0b11010111;

    // T0IE = 1 (timer 0 interrupt enable)
    // GPIE = 1 (GPIO interrupt on change enable)
    // GIE = 1 (global interrupt enable)
    INTCON = (1<<7) | (1<<5) | (1<<3);

    while(!exitApplication)
	{
        leds ^= (1<<LED1_PIN);      // blink LED 1
        GPIO = leds;
        wait();
	}

    // Reset application and possibly enter bootloader mode if break state holds.
    //
    // Note that we must do this GOTO from the main() routine to avoid leaving 
    // junk return addresses on the hardware call stack. 
    _asm 
        // disable interrupts
        clrf    _INTCON

        // jump back into bootloader
        clrf    _PCLATH
        goto 0
    _endasm;
}

void wait(void)
{
	unsigned char i = 0, j = 0;

    while(i++ != 255)
    {
        while(j++ != 255)
        {
            _asm
                nop
                nop
                nop
                nop
                nop
                clrwdt
            _endasm;

            if(exitApplication)
            {
                // can't reset into bootloader firmware from here because
                // call stack would be left with data.
                return;
            }
        }
    }
}