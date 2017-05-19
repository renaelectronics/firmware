#include <p18cxxx.h>
#include "serial.h"

/* Put a byte to serial port */
void uart_putc(char byte) {
    while (!PIR1bits.TXIF); /* set when register is empty */
    TXREG = byte;
    while (!PIR1bits.TXIF); /* wait until the character is sent */
}

void uart_puts(char *str) {
    int n;
    for (n = 0; str[n] != 0; n++) {
        uart_putc(str[n]);
    }
}

char uart_getc(void) {
    char c;
    while (!PIR1bits.RCIF);
    c = RCREG;
    return c;
}

/* Put an 8 bit HEX number to the serial port */
void uart_p2x(char input_byte) {
    char ascii_value[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f'
    };
    uart_putc(ascii_value[(input_byte >> 4) & 0xf]);
    uart_putc(ascii_value[input_byte & 0xf]);
}

void uart_p8x(unsigned long value) {
    uart_p2x((value >> 24) & 0xff);
    uart_p2x((value >> 16) & 0xff);
    uart_p2x((value >> 8) & 0xff);
    uart_p2x(value & 0xff);
}