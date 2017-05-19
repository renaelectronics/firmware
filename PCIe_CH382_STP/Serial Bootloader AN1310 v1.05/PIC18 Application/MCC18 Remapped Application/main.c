/************************************************************************
*
* Serial Bootloader 
*
* This code example shows how to create MCC18 C application code 
* compatible for use with the Serial Bootloader when the bootloader
* firmware is located at address 0.
*
*************************************************************************
* FileName:     main.c
* Dependencies: 
* Compiler:     MPLAB C18, v3.30 or higher
* Company:      Microchip Technology, Inc.
*
* Software License Agreement
*
* Copyright © 2009 Microchip Technology Inc. All rights reserved.
*
* Microchip licenses to you the right to use, modify, copy and distribute
* Software only when embedded on a Microchip microcontroller or digital
* signal controller, which is integrated into your product or third party
* product (pursuant to the sublicense terms in the accompanying license
* agreement).
*
* You should refer to the license agreement accompanying this Software for
* additional information regarding your rights and obligations.
*
* SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY
* WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A
* PARTICULAR PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE
* LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY,
* CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY
* DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY
* INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST
* PROFITS OR LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY,
* SERVICES, OR ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO
* ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*
* Author        Date        Comment
*************************************************************************
* E. Schlunder  2009/04/23  Updating from previous AN851 example code.
************************************************************************/

#include <p18cxxx.h>
#include <stdio.h>
#include <usart.h>
#include <math.h>

void high_isr(void);
void low_isr (void);

// Prevent application code from being written into FLASH memory space 
// needed for the Bootloader firmware at addresses 0 through 3FFh.
#pragma romdata BootloaderProgramMemorySpace = 0x6
const rom char bootloaderProgramMemorySpace[0x400 - 0x6];

// The routine _startup() is defined in the C18 startup code (usually c018i.c) and
// is usually the first code to be called by a GOTO at the normal reset vector of 0.
extern void _startup(void);

// Since the bootloader isn't going to write the normal reset vector at 0, we have
// to generate our own remapped reset vector at the address specified in the 
// bootloader firmware.
#pragma code AppVector = 0x400          // must match bootloader source code definition
void AppVector(void)
{
	_asm GOTO _startup _endasm
}

// For PIC18 devices the high priority interrupt vector is normally positioned at
// address 0008h, but the bootloader resides there. Therefore, the 
// bootloader's interrupt vector code is set up to branch to our code at 0408h.
#pragma code AppHighIntVector = 0x408   // must match bootloader source code definition
void AppHighIntVector(void)
{
    _asm GOTO high_isr _endasm          // branch to the high_isr() function to handle priority interrupts.
}

#pragma code AppLowIntVector = 0x418    // must match bootloader source code definition
void low_vector(void)
{
  _asm GOTO low_isr _endasm             // branch to the low_isr() function to handle priority interrupts.
}

#pragma code                            // return to the default code section

#pragma interrupt high_isr
void high_isr(void)
{
    LATDbits.LATD1 ^= 1;    // blink LED
    PIR1bits.TMR1IF = 0;    // clear TMR1IF interrupt flag
}

#pragma interruptlow low_isr
void low_isr(void)
{
	LATDbits.LATD0 ^= 1;    // blink an LED
	INTCONbits.TMR0IF = 0;
}

// BAUDRG is calculated as  = Fosc / (4 * Desired Baudrate) - 1
// So, 4MHz / (4 * 19200) - 1 = 52 (approx.)
// 32MHz / (4 * 115200) - 1 = 69
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

unsigned int brg;
void main()
{
    OSCTUNEbits.PLLEN = 1;      // enable PLL for higher speed execution
    LATCbits.LATC6 = 1;         // drive TXD pin high for RS-232 IDLE state
    TRISCbits.TRISC6 = 0;

    if(RCONbits.POR == 0)
    {
        // Power On Reset occurred, bootloader did not capture an autobaud BRG value.
        brg = 0;
        RCONbits.POR = 1;       // flag that future MCLR/soft resets are not POR resets
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

/*
    OSCCONbits.IRCF2 = 1;       // enable full 8MHz INTOSC
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF0 = 1;
*/

	TMR0H = 0;
    TMR0L = 0;
	T0CON = 0b00010110;
	INTCONbits.TMR0IF = 0;
	INTCONbits.TMR0IE = 1;
    INTCON2bits.TMR0IP = 0;

	TMR1H = 0;
    TMR1L = 0;
	T1CON = 0b00110111;
	T1CONbits.TMR1CS = 0;	// internal clock source (Fosc/4)
    PIR1bits.TMR1IF = 0;
	PIE1bits.TMR1IE = 1;
    IPR1bits.TMR1IP = 1;        // Timer 1 as High Priority interrupt

    RCONbits.IPEN = 1;          // enable interrupt priority
	INTCONbits.GIEH = 1;
	INTCONbits.GIEL = 1;

  	Open1USART(USART_TX_INT_OFF & USART_RX_INT_OFF & USART_EIGHT_BIT & USART_ASYNCH_MODE & USART_ADDEN_OFF, BAUDRG);
	baud1USART(BAUD_IDLE_TX_PIN_STATE_HIGH & BAUD_IDLE_RX_PIN_STATE_HIGH & BAUD_AUTO_OFF & BAUD_WAKEUP_OFF & BAUD_16_BIT_RATE & USART_RX_INT_ON);

	LATD = 0;                   // turn off LEDs
	TRISD = 0;                  // enable PORTD LED outputs

    // wait for stable clock and host connection by waiting for first Timer 0 interrupt
    T0CONbits.TMR0ON = 1;       
    while(LATDbits.LATD0 == 0); 

    if(brg != 0)
    {
        printf( (far rom char*) "Bootloader BRG: %u\r\n", brg);
    }
    printf( (far rom char*) "\r\nHello world from PIC microcontroller!\r\n");

    // infinite loop, interrupt handler will blink PORTD pins.
	while(1)
    {
        if(PIR1bits.RCIF)
        {
            if(RCSTAbits.FERR && PORTCbits.RC7 == 0)
            {
                // receiving BREAK state, soft reboot back into Bootloader mode.
                Reset();
            }

            TXREG = RCREG;      // echo received serial data back out
            LATD ^= 0xF0;       // blink LEDs
        }
    }
}
