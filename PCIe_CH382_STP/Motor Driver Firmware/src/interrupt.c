#include <p18cxxx.h>
#include "io.h"
#include "interrupt.h"

/* global variables */
unsigned char led_status;
unsigned int led_timer;
unsigned int total_1ms_tick;

void interrupt high_isr(void) {
    /* ------------------------------------------------------
     * Set up Timer1 to count, it will generate an interrupt
     * when the counter roll over, ie the count value change
     * from 0xFFFF to 0x0000.
     *
     * Timer1 Control Value
     * T1CKPS1:T1CKPS0 = prescaler 1:8
     * T1OSCEN = 1
     * T1SYNC = 1
     * TMR1CS = osc/4 = 0
     *
     * OSC = 6Mhz
     * ie 1 ticks is 6Mz/4/8 = 187500Hz = 5.33us
     * 100us = ticks * 5.33us
     * ticks = 19
     * Since the timer always count up,
     * TMR1H = 0xFF
     * TMR1L = (0xFF - 19)=236
     *
     * OSC = 8Mhz
     * ie 1 ticks is 8Mz/4/8 = 187500Hz = 4.00us
     * 100us = ticks * 4.00us
     * ticks = 25
     * Since the timer always count up,
     * TMR1H = 0xFF
     * TMR1L = (0xFF - 25) = 230 = 0xE6
     *
     * OSC = 32Mhz
     * ie 1 ticks is 32Mz/4/8 = 187500Hz = 1.00us
     * 100us = ticks * 1.00us
     * ticks = 100 = 0x64
     * Since the timer always count up,
     * TMR1H = 0xFF
     * TMR1L = (0xFF - 0x64) = 0x9B
     *
     * 1ms = 1000us = ticks * 1.00us
     * ticks = 1000 = 0x3E8
     * TMR1H:TMR1L = 0xFFFF - 0x3E8 = 0xFC17
     * ------------------------------------------------------
     */

    /*
     * stop timer and clear interrupt flag
     */
    T1CONbits.TMR1ON = 0;
    PIR1bits.TMR1IF = 0;

    /*
     * increment all ticks
     */
    total_1ms_tick++;
    led_timer++;

    /*
     * clear timer value, clear interrupt flag
     * and then start the timer again
     */
    TMR1H = 0xFC;
    TMR1L = 0x17;
    T1CONbits.TMR1ON = 1;
}

/* timing loop */
void delay_us(int usec) {
    int n;
    while (usec > 0) {
        for (n = 0; n < 48; n++);
        usec--;
    }
}
