#ifndef SERIAL_H
#define SERIAL_H

void uart_putc(char byte);
void uart_puts(char *str);
void uart_p2x(char input_byte);
char uart_getc(void);
void uart_p8x(unsigned long value);

#endif
