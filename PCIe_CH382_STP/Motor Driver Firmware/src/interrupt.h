#ifndef INTERRUPT_H
#define INTERRUPT_H

#define BIT0	(1)
#define BIT1	(1<<1)
#define BIT2	(1<<2)
#define BIT3	(1<<3)
#define BIT4	(1<<4)
#define BIT5	(1<<5)
#define BIT6	(1<<6)
#define BIT7	(1<<7)

#define DISABLE_TIMER1_IE()         PIE1bits.TMR1IE=0
#define ENABLE_TIMER1_IE()          PIE1bits.TMR1IE=1

#define CLEAR_TOTAL_1MS_CLICK()     do{PIE1bits.TMR1IE=0;total_1ms_tick=0;PIE1bits.TMR1IE=1;}while(0)
#define GET_TOTAL_1MX_CLICK()       (total_1ms_tick)

extern unsigned char led_status;
extern unsigned int led_timer;
extern unsigned int total_1ms_tick;
void delay_us(int usec);

#endif
