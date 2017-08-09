/* Modification History:
 *
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
#define FIRMWARE_VERSION    (0x52) /* version 5.2 */

/* debug only, it must be off during normal operation */
#define DEBUG_BLINK         (0) /* blink led test */
#define DEBUG_INFO          (0) /* must be on for output printing debug */
#define DEBUG_SELF_TEST     (0)
#define DEBUG_SHOW_TVAL     (0)
#define DEBUG_SHOW_DOT      (0) /* continue print 0 */
#define DEBUG_DEFAULT_VAL   (0)

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

/* globel variables */
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

#if DEBUG_DEFAULT_VAL
#warning "default motor value is enabled"
unsigned char default_value[] = {
    /* motor current=1, step mode=1 */
    0x00, 0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00, 0x00,
    0x20,
    0x19,
    0x29,
    0x29,
    0x00,
    0x08,
    0x89,
    0xff,
    0x2e, 0x88,
    0x00, 0x00,
    0x2f /* checksum */
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
    for(;;){
        CLEAR_TOTAL_1MS_CLICK();
        while (total_1ms_tick < 1000L/* 1 second */);
        LED_OUT = (unsigned char)~LED_OUT;
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
    
    /* Wait 15 seconds for the 12V to be stable */
    for (n = 0; n < 15; n++) {
        LED_OUT = (unsigned char)~LED_OUT;
        CLEAR_TOTAL_1MS_CLICK();
        while (total_1ms_tick < 1000L/* 1 second */);
    }

    /* initialize SPI */
    CloseSPI();
    OpenSPI(SPI_FOSC_64, MODE_11, SMPMID);

    /* delay 0.2 seconds for SPI to stable */
    CLEAR_TOTAL_1MS_CLICK();
    while (total_1ms_tick < 200L);

#if DEBUG_SELF_TEST
#warning debug self test enabled
    for (;;) {
        
        /* self test1: read ALARM_EN */
        get_alarm_en(M1);
        debug_p2x_crlf(param1);
        get_alarm_en(M2);
        debug_p2x_crlf(param1);
        get_alarm_en(M3);
        debug_p2x_crlf(param1);
        get_alarm_en(M4);
        debug_p2x_crlf(param1);

        /* self test 2: get config */
        get_config(M1);
        debug_p2x(param1);
        debug_p2x_crlf(param2);

        /* self test 3: read write to ABS_POS */
        param1 = 0x01;
        param2 = 0x02;
        param3 = 0x03;
        set_abs_pos(M1);
        get_abs_pos(M1);

        /* read ABS_POS */
        debug_p2x(param1);
        debug_p2x(param2);
        debug_p2x_crlf(param3);

        /* self test 5: get status */
        get_status(M1);
        debug_p2x(param1);
        debug_p2x_crlf(param2);

        /* 1 seconds delay */
        CLEAR_TOTAL_1MS_CLICK();
        while (total_1ms_tick < 1000L/* 1 second */);
    }
#endif

#if DEBUG_SHOW_TVAL
#warning debug show tval enabled
    for(;;){
        /* M1 current */
        debug_p8x_crlf(0x11111111UL);
        get_tval(M1);
        debug_p2x_crlf(param1);

        /* M2 current */
        debug_p8x_crlf(0x22222222UL);
        get_tval(M2);
        debug_p2x_crlf(param1);

        /* M3 current */
        debug_p8x_crlf(0x33333333UL);
        get_tval(M3);
        debug_p2x_crlf(param1);

        /* M4 current */
        debug_p8x_crlf(0x44444444UL);
        get_tval(M4);
        debug_p2x_crlf(param1);
    }
#endif

#if DEBUG_DEFAULT_VAL
#warning "writing default value to eeprom is enabled"
    for (n = 0; n < EEPROM_MAX_BYTE; n++) {
        write_eeprom_data((unsigned char)((M1 * EEPROM_OFFSET) + n),
                          default_value[n]);
        write_eeprom_data((unsigned char)((M2 * EEPROM_OFFSET) + n),
                          default_value[n]);
        write_eeprom_data((unsigned char)((M3 * EEPROM_OFFSET) + n),
                          default_value[n]);
        write_eeprom_data((unsigned char)((M4 * EEPROM_OFFSET) + n),
                          default_value[n]);
    }
#endif

    /* disable motors */
    motor_enabled = 0;
    motor_disable(M1);
    motor_disable(M2);
    motor_disable(M3);
    motor_disable(M4);

    /* copy eeprom value to L6474 motor driver chip */
    for (n = M1; n <= M4; n++) {
        if ((!blank_check(n)) && chksum_check(n)) {
            copy_from_eeprom(n);
        }
    }

    /* initial value */
    CLEAR_DATA_BUF();
    state = STATE_IDLE;

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
            state = STATE_IDLE;
        }

        /* receive character and go into state machine */
        if (DoReadHostByte(&rx) == 0)
            goto state_machine_entry;
                
        switch (state) {
            case STATE_IDLE:
                /* prepare offset */
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
                    case 'v':
                        DoWriteHostByte(FIRMWARE_VERSION);
                        break;

                    case 'x':
                    case 'y':
                    case 'z':
                    case 'a':
                        /* read 6474 registers */
                        /* ABS_POS */
                        get_abs_pos(motor_unit);
                        DoWriteHostByte(param1);
                        DoWriteHostByte(param2);
                        DoWriteHostByte(param3);
                        /* EL_POS */
                        get_el_pos(motor_unit);
                        DoWriteHostByte(param1);
                        DoWriteHostByte(param2);
                        /* MARK */
                        get_mark(motor_unit);
                        DoWriteHostByte(param1);
                        DoWriteHostByte(param2);
                        DoWriteHostByte(param3);
                        /* TVAL */
                        get_tval(motor_unit);
                        DoWriteHostByte(param1);
                        /* T_FAST */
                        get_t_fast(motor_unit);
                        DoWriteHostByte(param1);
                        /* TON_MIN */
                        get_ton_min(motor_unit);
                        DoWriteHostByte(param1);
                        /* TOFF_MIN */
                        get_toff_min(motor_unit);
                        DoWriteHostByte(param1);
                        /* ADC_OUT */
                        get_adc_out(motor_unit);
                        DoWriteHostByte(param1);
                        /* OCD_TH */
                        get_ocd_th(motor_unit);
                        DoWriteHostByte(param1);
                        /* STEP_MODE */
                        get_step_mode(motor_unit);
                        DoWriteHostByte(param1);
                        /* ALARM_EN */
                        get_alarm_en(motor_unit);
                        DoWriteHostByte(param1);
                        /* CONFIG */
                        get_config(motor_unit);
                        DoWriteHostByte(param1);
                        DoWriteHostByte(param2);
                        /* STATUS */
                        get_status(motor_unit);
                        DoWriteHostByte(param1);
                        DoWriteHostByte(param2);
                        /* CHECKSUM = 0 */
                        DoWriteHostByte(0);
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
                        switch (motor_unit) {
                            case M1:
                                eeprom_offset = M1 * EEPROM_OFFSET;
                                break;
                            case M2:
                                eeprom_offset = M2 * EEPROM_OFFSET;
                                break;
                            case M3:
                                eeprom_offset = M3 * EEPROM_OFFSET;
                                break;
                            case M4:
                                eeprom_offset = M4 * EEPROM_OFFSET;
                                break;
                        }
                        /* write all words to EEPROM */
                        for (n = 0; n < EEPROM_MAX_BYTE; n++) {
                            write_eeprom_data((unsigned char)(eeprom_offset + n),
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
            /* copy motor stepping signals before enabling the motors */
            PORTB = PORTA;
            /* only enable the unit if the eeprom is valid */
            for (n = M1; n <= M4; n++) {
                if ((!blank_check(n)) && chksum_check(n)) {
                    /* reset position */
                    reset_position(n);
                    /* enable motor */
                     motor_enable(n);
                    LED_OUT = 1;
                }
            }
            motor_enabled = 1;
        }
        
        /* clear error status */
        get_config(M1);
        get_config(M2);
        get_config(M3);
        get_config(M4);
        
        /* motor enable signal, aka strobe signal is inverted */
        while (MX_ENABLE == 0) {
            /* bit follower */
            PORTB = PORTA;
            
            /* error detected */
            if (FLAG_N == 0){
                ENABLE_INTERRUPT();
                CLEAR_TOTAL_1MS_CLICK();
                /* disable motor */
                motor_disable(M1);
                motor_disable(M2);
                motor_disable(M3);
                motor_disable(M4);
                motor_enabled = 0;
                state = STATE_IDLE;
                /* blink LED */
                for (;;) {
                    LED_OUT = (unsigned char)~LED_OUT;
                    CLEAR_TOTAL_1MS_CLICK();
                    while (total_1ms_tick < 300L/* 0.3 second */);
                    /* motor enable signal, aka strobe signal is inverted */
                    if (MX_ENABLE == 1){
                        LED_OUT = 1;
                        break;
                    }
                }
            }
        }
        state = STATE_IDLE;
    }

    /* forever state machine entry */
    goto state_machine_entry;

}/* end main */
