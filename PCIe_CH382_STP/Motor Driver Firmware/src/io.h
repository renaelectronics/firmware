/* 
 * File:   io.h
 * Author: Thomas Tai
 *
 * Created on August 26, 2013, 9:08 PM
 */

#ifndef IO_H
#define	IO_H

/* PORT A */
#define PORTA_TRISA         0xFF /* RA [7..0] are input */
#define PORTA_INIT_VALUE    0xFF 
#define IN_0                PORTAbits.RA0 /* 1, input */
#define IN_1                PORTAbits.RA1 /* 1, input  */
#define IN_2                PORTAbits.RA2 /* 1, input  */
#define IN_3                PORTAbits.RA3 /* 1, input  */
#define IN_4                PORTAbits.RA4 /* 1, input */
#define IN_5                PORTAbits.RA5 /* 1, input  */
#define IN_6                PORTAbits.RA6 /* 1, OSC2, input */
#define IN_7                PORTAbits.RA7 /* 1, OSC1, input */

/* PORT B */
#define PORTB_TRISB         0x00 /* RB [7..0] are output */
#define PORTB_INIT_VALUE    0xFF
#define M1_STCK             PORTBbits.RB0 /* 0, output */
#define M1_DIR              PORTBbits.RB1 /* 0, output */
#define M4_STCK             PORTBbits.RB2 /* 0, output */
#define M4_DIR              PORTBbits.RB3 /* 0, output */
#define M3_STCK             PORTBbits.RB4 /* 0, output */
#define M3_DIR              PORTBbits.RB5 /* 0, output */
#define M2_STCK             PORTBbits.RB6 /* 0, output */
#define M2_DIR              PORTBbits.RB7 /* 0, output */

/* PORT C */
#define PORTC_TRISC         0x93 /* RC7=FLAG_N, RC4=SDI, RC0=STROBE, RC1=FIRMWARE SW are input */
#define PORTC_INIT_VALUE    0xFD /* RC1 = WCH382 PERST# = 0  */
#define MX_ENABLE           PORTCbits.RC0 /* 1, input  *//* aka strobe */
#define FIRMWARE_SW         PORTCbits.RC1 /* 1, input  *//* firmware switch */
#define CS_N                PORTCbits.RC2 /* 0, output *//* cs_n */
#define SCK_RC3             PORTCbits.RC3 /* 0, output *//* sck */
#define SDI_RC4             PORTCbits.RC4 /* 1, input  *//* sdi */
#define SDO_RC5             PORTCbits.RC5 /* 0, output *//* sdo */
#define LED_OUT             PORTCbits.RC6 /* 0, output *//* led */
#define FLAG_N              PORTCbits.RC7 /* 1, input  *//* flag_n */

#endif	/* IO_H */

