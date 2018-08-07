#define USE_OR_MASKS
#include <p18cxxx.h>
#include <stdio.h>
#include <stdlib.h>
#include "spi.h"
#include "io.h"
#include "interrupt.h"
#include "eeprom.h"
#include "L6474.h"

#define DRVCONF_VALUE   (0x0e0000UL)
#define SGCSCONF_VALUE  (0x0c0000UL)
#define SMARTEN_VALUE   (0x0a0000UL)
#define CHOPCONF_VALUE  (0x088000UL)
#define DRVCTRL_VALUE   (0x000000UL)

#define DRVCONF_MASK    (0x000300UL)
#define SGCSCONF_MASK   (0x017f1fUL)
#define SMARTEN_MASK    (0x00ef6fUL)
#define CHOPCONF_MASK   (0x007fffUL)
#define DRVCTRL_MASK    (0x00020fUL)

/* debug value */
extern unsigned char debug_value;

/* saved register value */
static unsigned long sgcsconf[4];
static unsigned long chopconf[4];
u32 response[4];

/* variables */
unsigned char spi_rx;
unsigned char spi_tx[3];
static unsigned char eeprom_data[3];
static char tx_buf;
static int tx_buf_bit;
static char rx_index;
static int rx_buf_bit;

/* read eeprom byte by byte and return the unsigned long register value */
static u32 u8tou32(u8 byte) {
    u32 tmp32;
    tmp32 = byte;
    return tmp32;
}

static unsigned long eeprom_reg(u8 addr, u32 fixed, u32 mask) {
    u32 value;
    value = (u8tou32(read_eeprom_data(addr)) << 16) & 0x0f0000UL;
    value |= ((u32) (u8tou32(read_eeprom_data((u8) (addr + 1))) << 8));
    value |= ((u32) (u8tou32(read_eeprom_data((u8) (addr + 2)))));
    value &= mask;
    value |= fixed;
    return value;
}

/* must call init_rx_tx before calling in_rx and out_tx */
void init_rx_tx()
{
    /* initial tx rx variables */
	tx_buf = 0;
	tx_buf_bit = 0;
	rx_index = 0;
	rx_buf_bit = 0;
}

void in_rx(unsigned char value)
 {
    int n;
    for (n = 7; n >= 0; n--) {
        response[rx_index] = response[rx_index] << 1;
        if (value & (1 << n))
            response[rx_index] |= 1;
        rx_buf_bit++;
        if (rx_buf_bit == 20) {
            rx_index++;
            rx_buf_bit = 0;
        }
    }
}

void out_tx(u32 value32) {
    int n;

    /* shift 20 bit data into tx_buf */
    for (n = 19; n >= 0; n--) {
        tx_buf = (char) (tx_buf << 1);
        if (value32 & (1UL << n)) {
            tx_buf = (char) (tx_buf | 1);
        }
        tx_buf_bit++;
        if (tx_buf_bit == 8) {
            /* send the 8 bit out and read the response back*/
            while (WriteSPI(tx_buf));
            /* store the response bits */
            in_rx(SSPBUF);
            tx_buf_bit = 0;
            tx_buf = 0;
        }
    }
}

/* Send SPI data and read from the chain 
 * After this call rx[] contains the data from the chain
 * 
 *   bit pos: 2222 1111 1111 11
 *            3210 9876 5432 1098 7654 3210
 *            -----------------------------
 * bit value: 0000 .... .... .... .... ....
 * 
 */
static void write_spi_chain(char unit, unsigned long data) {
    int m;
    u32 value32;

    /* clear response and prepare stuff value for the chain */
    for (m = 0; m < 3; m++) {
        response[(unsigned char)m] = 0UL;
    }
    
    /* initialize variables */
    init_rx_tx();
    
    /* send motor register */
    for (m = 0; m <= 3; m++) {
        /* motor stuff value */
        value32 = sgcsconf[(unsigned char)m];
        if (m == unit){
            value32 = data & 0xfffffUL;
        }
        out_tx(value32);
    }
    
    /* CS_N must be low to select the motor driver chip */
    CS_N = 0;

    /* 10 us delay */
    delay_us(10);

    /* CS_N must be raised and be kept high for a while */
    CS_N = 1;

    /* 10 us delay */
    delay_us(10);
    return;
}

static void do_motor_enable(char unit, char enable) {
    u32 value;

    /* chopconf must be start with binary 1000 1 
     * To enable the motor, the TOFF value is restored to
     * the user specified value
     */
#if (0)
    value = ((chopconf[unit] & ~(0x0f8000UL)) | 0x088000UL);
#endif
    value = chopconf[unit];
    if (!enable) {
        value &= 0x0ffff0UL;
    }
    write_spi_chain(unit, value);
}

unsigned char get_eeprom_offset(char unit) {
    return ((unsigned char) (unit * EEPROM_OFFSET));
}

void motor_enable(char unit) {
#if (0)    
    u32 value;
    /* set minimum current */
    value = (sgcsconf[unit] & 0x0fffe0UL) | 1;
#endif
    do_motor_enable(unit, 1);
}

void motor_disable(char unit) {
    do_motor_enable(unit, 0);
}

unsigned long get_response(char unit) {
    return response[unit];
}

/* read from eeprom and copy into registers */
void copy_from_eeprom(char unit) {
    unsigned char offset;
    u32 value;

    /* blank check and chksum_check */
    if ((blank_check(unit)) || (!chksum_check(unit))) {
        /* set default chopconf */
        chopconf[unit] = 0x098000UL;
        return;
    }

    /* get offset address */
    offset = get_eeprom_offset(unit);

    /* EEPROM_DRVCONF */
    value = eeprom_reg((u8) (offset + EEPROM_DRVCONF), DRVCONF_VALUE, DRVCONF_MASK);
    write_spi_chain(unit, value);

    /* EEPROM_SGCSCONF *//* stuff value for spi data chain */
    value = eeprom_reg((u8) (offset + EEPROM_SGCSCONF), SGCSCONF_VALUE, SGCSCONF_MASK);
    sgcsconf[unit] = value;
    write_spi_chain(unit, value);

    /* EEPROM_SMARTEN */
    value = eeprom_reg((u8) (offset + EEPROM_SMARTEN), SMARTEN_VALUE, SMARTEN_MASK);
    write_spi_chain(unit, value);

    /* EEPROM_CHOPCONF */
    value = eeprom_reg((u8) (offset + EEPROM_CHOPCONF), CHOPCONF_VALUE, CHOPCONF_MASK);
    chopconf[unit] = value;
    /* make sure TOFF is 0 to disable the motor */
    write_spi_chain(unit, value & 0x0ffff0UL);

    /* EEPROM_DRVCTRL */
    value = eeprom_reg((u8) (offset + EEPROM_DRVCTRL), DRVCTRL_VALUE, DRVCTRL_MASK);
    write_spi_chain(unit, value);
}

unsigned char blank_check(char unit) {
    unsigned char n;
    unsigned char offset;

    /* get offset address */
    offset = get_eeprom_offset(unit);
    /* blank check */
    for (n = 0; n < EEPROM_MAX_BYTE; n++) {
        if (read_eeprom_data((unsigned char) (offset + n)) != 0xff) {
            return 0;
        }
    }
    return 1;
}

unsigned char chksum_check(char unit) {
    unsigned char n;
    unsigned char value;
    unsigned char offset;

    /* get offset address */
    offset = get_eeprom_offset(unit);
    /* checksum */
    value = 0;
    for (n = 0; n < EEPROM_MAX_BYTE; n++) {
        value += read_eeprom_data((unsigned char) (offset + n));
    }
    if (value == 0) {
        return 1;
    }
    return 0;
}
