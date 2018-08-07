/* Modification History:
 *
 * 07/12/2018 change motor driver to TMC2660
 * 02/10/2017 modify to use parallel port for motor setup
 * 10/25/2016 remove motor STBY_RESET pin, FIRMWARE_VERSION=0x51
 * 10/21/2016 set BAUDCONbits.BRG16 = 0
 * 10/17/2016 change to use MPLABX IDE  
 * 03/27/2015 add firmware version
 * 03/06/2015 port to PCIe 6474 card
 * 02/11/2013 reset absolute and electrical position when motor enable
 * 10/23/2013 fix SPI mode to MODE_11 with sample at middle
 * 10/20/2013 fix set_config() and reset motor position
 * 09/15/2013 blank check checksum before write to 6474
 * 08/26/2013 enable int OSC running at 32Mhz
 * 08/25/2013 initial setup
 * 11/14/2010 enable Brown-out Reset at 4.3V
 * 12/15/2012 lower brown-out value
 */

/* for working together with a bootloader, set linker code offset = 0x400 */
#define USE_OR_MASKS
#include <p18cxxx.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "spi.h"
#include "io.h"
#include "L6474.h"
#include "interrupt.h"
#include "eeprom.h"
#include "bitbangport.h"

/* firmware version */
#define FIRMWARE_VERSION    (0x53) /* version 5.3 */

/* debug only, it must be off during normal operation */
#define DEBUG_BLINK         (0) /* blink led test */
#define DEBUG_INFO          (0) /* must be on for output printing debug */
#define DEBUG_SELF_TEST     (0)
#define DEBUG_SHOW_TVAL     (0)
#define DEBUG_SHOW_DOT      (0) /* continue print 0 */
#define DEBUG_DEFAULT_VAL   (1)
#define DEBUG_ALWAYS_ON     (0) /* enable the motor at bootup */
#define DEBUG_RESET_CMD     (1) /* enable reset command */

/* defaults */
#define POWERUP_DELAY_SEC   (5)

#if DEBUG_INFO
#warning debug info enabled
#define debug_putc(c)       DoWriteHostByte(c)
#define debug_p2x(c)        bitbang_p2x(c)
#define debug_p2x_crlf(c)   do{ debug_p2x(c); debug_putc('\r'); debug_putc('\n'); } while (0)
#define debug_p8x(c)        bitbang_p8x(c)
#define debug_p8x_crlf(c)   do{ debug_p8x(c); debug_putc('\r'); debug_putc('\n'); } while (0)
#else
#define debug_putc(c)
#define debug_p2x(c)
#define debug_p2x_crlf(c)
#define debug_p8x(c)
#define debug_p8x_crlf(c)
#endif

/* event loop state machine */
#define STATE_IDLE          (0)
#define STATE_HOST_WRITE    (1)
#define STATE_HOST_READ     (2)

#define ENABLE_INTERRUPT() do{ INTCONbits.GIE = 1; INTCONbits.PEIE = 1; PIR1bits.TMR1IF = 0; PIE1bits.TMR1IE = 1;}while(0)
#define DISABLE_INTERRUPT() do{ INTCONbits.GIE = 0; INTCONbits.PEIE = 0; PIR1bits.TMR1IF = 0; PIE1bits.TMR1IE = 0;}while(0)

/* reset buffer and chksum */
#define CLEAR_DATA_BUF()    do{rx_index=0;chksum=0;}while(0)

/* global variables */
unsigned char eeprom_offset;
unsigned char motor_unit;
unsigned char rx_packet[EEPROM_MAX_BYTE];
unsigned char rx;
unsigned char rx_index;
unsigned char chksum;
unsigned char motor_enabled;
unsigned char state;
unsigned char n;
unsigned char value;
unsigned char last_value;
unsigned char debug_value;

#if DEBUG_DEFAULT_VAL
#warning "default motor value is enabled"
#if (0) /* working eeprom */
unsigned char default_value[EEPROM_MAX_BYTE] = {
    0x0e, 0x00, 0x00, /* drvconf */
    0x0c, 0x00, 0x03, /* sgcsconf */
    0x0a, 0x00, 0x00, /* smarten */
    0x09, 0xc1, 0x87, /* chopconf */
    0x00, 0x00, 0x08, /* drvctrl */
    0x00, 0x00, 0x00, /* status */
    0x00 /* checksum, to be filled in */
};
#endif
unsigned char default_value[EEPROM_MAX_BYTE] = {
/*
 *  ./wch6474 -m 0 -c 1.0 -s 8 -t -3 -w -3 -o 3
 *
 * 0e f0 00 
 * 0c 00 09 
 * 0a 00 00 
 * 08 43 66 
 * 00 02 05 
 * 00 00 00
 * 2b
 * ---------------------------------------
 *              Motor Unit : 0
 *           Motor Current : 0.970000 A
 *              Microsteps : 8
 *    Pulse multiplication : 1
 *         Slow Decay Time : -3
 *         Fast Decay Time : -3
 *        Sine Wave Offset : 3
 * ---------------------------------------
 */
    0x0e, 0xf0, 0x00, /* drvconf */
    0x0c, 0x00, 0x09, /* sgcsconf */
    0x0a, 0x00, 0x00, /* smarten */
    0x08, 0x43, 0x66, /* chopconf */
    0x00, 0x02, 0x05, /* drvctrl */
    0x00, 0x00, 0x00, /* status */
    0x00 /* checksum, to be filled in */
};
#endif

/* program entry */
void main(void) {

    /*
     * run internal OSC at 8Mhz
     *  OSCCON = 0x72
     * run internal OSC at 8Mhz with PLL enabled
     *  OSCCON = 0x70;
     *  OSTUNE = 0xC0;
     */
    OSCCON = 0x70;

    /* enable PLL */
    OSCTUNE = 0xC0;

    /*
     * interrupt priority, PIC16CXXX Compatibility mode
     */
    RCONbits.IPEN = 0;

    /*
     * Timer 0:
     * set timer 0 to 16 bit mode and run at clock speed
     */
    T0CONbits.TMR0ON = 0;
    TMR0H = 0;
    TMR0L = 0;
    T0CONbits.T08BIT = 0;
    T0CONbits.T0CS = 0;
    T0CONbits.T0SE = 0;
    T0CONbits.PSA = 1;
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 0;

    /*
     * Timer 1:
     * set to counter mode and generate an interrupt,
     * the timer ISR is then setup the correct interrupt interval
     */
    T1CONbits.TMR1ON = 0;
    T1CON = 0x30;
    TMR1H = 0xFF;
    TMR1L = 0xFF;
    T1CONbits.TMR1ON = 1;

    /*
     * Clear Interrupt Flag and enable Timer1 interrupt
     */
    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;
    CLEAR_TOTAL_1MS_CLICK();

    /* set PORTA to all digital io */
    ADCON1bits.PCFG0 = 1;
    ADCON1bits.PCFG1 = 1;
    ADCON1bits.PCFG2 = 1;
    ADCON1bits.PCFG3 = 1;

    /* set PORT output direction */
    PORTA = PORTA_INIT_VALUE;
    TRISA = PORTA_TRISA;

    PORTB = PORTB_INIT_VALUE;
    TRISB = PORTB_TRISB;

    PORTC = PORTC_INIT_VALUE;
    TRISC = PORTC_TRISC;

    /*
     * Global interrupt enable and peripheral interrupt
     */
    ENABLE_INTERRUPT();

#if DEBUG_BLINK
#warning debug led blink enabled
    for (;;) {
        CLEAR_TOTAL_1MS_CLICK();
        while (total_1ms_tick < 1000L/* 1 second */);
        LED_OUT = (unsigned char) ~LED_OUT;
    }
#endif        

#if DEBUG_SHOW_DOT
#warning debug show dot enabled
    for (;;) {
        CLEAR_TOTAL_1MS_CLICK();
        while (total_1ms_tick < 1000L/* 1 second */);
        DoWriteHostByte('0');
    }
#endif

    /*
     * pause for hardware to become stable
     */
    CLEAR_TOTAL_1MS_CLICK();
    while (total_1ms_tick < 1/* 1 ms */);

    /*
     * Set up completed
     */
    /* set motor CS to hi */
    CS_N = 1;

    /* Powerup delay for the 12V to be stable */
    for (n = 0; n < POWERUP_DELAY_SEC; n++) {
        LED_OUT = (unsigned char) ~LED_OUT;
        CLEAR_TOTAL_1MS_CLICK();
        while (total_1ms_tick < 1000L/* 1 second */);
    }

    /* initialize SPI */
    debug_value = 0;
    CloseSPI();
    OpenSPI(SPI_FOSC_64, MODE_11, SMPMID);

    /* delay 0.2 seconds for SPI to stable */
    CLEAR_TOTAL_1MS_CLICK();
    while (total_1ms_tick < 200L);

#if DEBUG_SELF_TEST
#warning debug self test enabled
    for (;;) {
    }
#endif

#if DEBUG_SHOW_TVAL
#warning debug show tval enabled
    for (;;) {
    }
#endif

#if DEBUG_DEFAULT_VAL
#warning "writing default value to eeprom is enabled"
    /* write the default value to eeprom */
    chksum = 0;
    for (n = 0; n < EEPROM_MAX_BYTE; n++) {
        /* find check sum */
        if (n == (EEPROM_MAX_BYTE - 1)) {
            chksum = (unsigned char) ~chksum;
            chksum += 1;
            value = chksum;
        } else {
            value = default_value[n];
            chksum += value;
        }
        /* write to eeprom */
        write_eeprom_data((unsigned char) ((M1 * EEPROM_OFFSET) + n), value);
        write_eeprom_data((unsigned char) ((M2 * EEPROM_OFFSET) + n), value);
        write_eeprom_data((unsigned char) ((M3 * EEPROM_OFFSET) + n), value);
        write_eeprom_data((unsigned char) ((M4 * EEPROM_OFFSET) + n), value);
    }
#endif

    /* copy eeprom value to motor driver chip */
    for (n = M1; n <= M4; n++) {
        copy_from_eeprom(n);
    }

    /* disable motors */
    motor_enabled = 0;
    motor_disable(M1);
    motor_disable(M2);
    motor_disable(M3);
    motor_disable(M4);

    /* initial value */
    LED_OUT = 0;
    CLEAR_DATA_BUF();
    state = STATE_IDLE;

#if DEBUG_ALWAYS_ON
#warning enable motor at boot 
    DISABLE_INTERRUPT();
    CLEAR_TOTAL_1MS_CLICK();
    /* only enable the unit if the eeprom is valid */
    for (n = M1; n <= M4; n++) {
        if ((!blank_check(n)) && chksum_check(n)) {
            /* enable motor */
            motor_enable(n);
        }
    }
    motor_enabled = 1;
    /* copy motor stepping signals before enabling the motors */
    PORTB = PORTA;
    LED_OUT = 1;
    for (;;)
        PORTB = PORTA;
#endif
    
state_machine_entry:

    /* motor enable signal, aka strobe signal is inverted */
    if (MX_ENABLE == 1) {

        /* change motor state to disable */
        if (motor_enabled) {
            ENABLE_INTERRUPT();
            CLEAR_TOTAL_1MS_CLICK();
            /* disable motor */
            motor_disable(M1);
            motor_disable(M2);
            motor_disable(M3);
            motor_disable(M4);
            motor_enabled = 0;
            LED_OUT = 0; /* DoReadHostByte will also clear this bit */
            state = STATE_IDLE;
        }

        /* receive character and go into state machine */
        if (DoReadHostByte(&rx) == 0)
            goto state_machine_entry;

        switch (state) {
            case STATE_IDLE:
                /* prepare motor unit offset */
                switch (rx) {
                    case 'x':
                    case 'X':
                        motor_unit = M1;
                        break;
                    case 'y':
                    case 'Y':
                        motor_unit = M2;
                        break;
                    case 'z':
                    case 'Z':
                        motor_unit = M3;
                        break;
                    case 'a':
                    case 'A':
                        motor_unit = M4;
                        break;
                }
                /* read or write action */
                switch (rx) {
#if DEBUG_RESET_CMD
#warning debug reset command enabled
                    case 'R':
                        RESET();
                        break;
#endif
                    case 'v':
                        DoWriteHostByte(FIRMWARE_VERSION);
                        break;

                    case 'x':
                    case 'y':
                    case 'z':
                    case 'a':
                        /* read eeprom registers value */
                        eeprom_offset = get_eeprom_offset(motor_unit);
                        for (n = 0; n < EEPROM_MAX_BYTE; n++) {
                            value = read_eeprom_data((unsigned char) (eeprom_offset + n));
                            /* send the response back at EEPROM_STATUS */
                            if (n == EEPROM_STATUS) {
                                value = (get_response(motor_unit) & 0x0f0000UL) >> 16;
                            }
                            if (n == EEPROM_STATUS + 1) {
                                value = (get_response(motor_unit) & 0xff00UL) >> 8;
                            }
                            if (n == EEPROM_STATUS + 2) {
                                value = (get_response(motor_unit) & 0xffUL);
                            }
                            DoWriteHostByte(value);
                        }
                        break;

                    case 'X':
                    case 'Y':
                    case 'Z':
                    case 'A':
                        /* write to eeprom and copy to 6474 */
                        CLEAR_DATA_BUF();
                        state = STATE_HOST_WRITE;
                        /* echo the character back */
                        DoWriteHostByte(rx);
                        break;
                }
                break;

            case STATE_HOST_WRITE:
                rx_packet[rx_index] = rx;
                chksum += rx;
                rx_index++;
                if (rx_index < EEPROM_MAX_BYTE) {
                    /* echo the rx */
                    DoWriteHostByte(rx);
                } else {
                    /* check sum */
                    if (chksum == 0) {
                        /* find eeprom offset */
                        eeprom_offset = get_eeprom_offset(motor_unit);
                        /* write all words to EEPROM */
                        for (n = 0; n < EEPROM_MAX_BYTE; n++) {
                            write_eeprom_data((unsigned char) (eeprom_offset + n),
                                    rx_packet[n]);
                        }
                        /* copy eeprom data to 6474 */
                        copy_from_eeprom(motor_unit);
                    } else {
                        /* wrong check sum */
                        rx = chksum;
                    }
                    DoWriteHostByte(rx);
                    state = STATE_IDLE;
                }
                break;
        }/* switch state */

    } else {
        /* enable motors once */
        if (!motor_enabled) {
            DISABLE_INTERRUPT();
            CLEAR_TOTAL_1MS_CLICK();
            /* only enable the unit if the eeprom is valid */
            for (n = M1; n <= M4; n++) {
                if ((!blank_check(n)) && chksum_check(n)) {
                    /* enable motor */
                    motor_enable(n);
                }
            }
            motor_enabled = 1;
            /* copy motor stepping signals before enabling the motors */
            PORTB = PORTA;
            LED_OUT = 1;
        }
        
        /* motor enable signal, aka strobe signal is inverted */
        while (MX_ENABLE == 0) {
            /* bit follower */
            PORTB = PORTA;
        }
        state = STATE_IDLE;
    }

    /* forever state machine entry */
    goto state_machine_entry;

}/* end main */
