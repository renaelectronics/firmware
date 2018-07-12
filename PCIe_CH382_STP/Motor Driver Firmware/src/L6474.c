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
#define CHOPCONF_VALUE  (0x098000UL)
#define DRVCTRL_VALUE   (0x000000UL)

#define DRVCONF_MASK    (0x000300UL)
#define SGCSCONF_MASK   (0x017f1fUL)
#define SMARTEN_MASK    (0x00ef6fUL)
#define CHOPCONF_MASK   (0x007fffUL)
#define DRVCTRL_MASK    (0x00020fUL)

/* debug value */
extern unsigned char debug_value;

/* saved motor drvconf */
static unsigned long chopconf[4];

/* variables */
unsigned char spi_rx[3];
static unsigned char spi_tx[3];
static unsigned char n, i, offset;
static unsigned char eeprom_data[3];

/* read eeprom byte by byte and return the unsigned long register value */
u32 u8tou32(u8 byte) {
    u32 tmp32;
    tmp32 = byte;
    return tmp32;
}

unsigned long eeprom_reg(u8 addr, u32 fixed, u32 mask) {
    u32 value;
    value = (u8tou32(read_eeprom_data(addr)) << 16) & 0x0f0000UL;
    value |= ((u32) (u8tou32(read_eeprom_data((u8) (addr + 1))) << 8));
    value |= ((u32) (u8tou32(read_eeprom_data((u8) (addr + 2)))));
    value &= mask;
    value |= fixed;
    return value;
}

unsigned char get_eeprom_offset(char unit) {
    return ((unsigned char) (unit * EEPROM_OFFSET));
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

    /* CS_N must be low to select the motor driver chip */
    CS_N = 0;

    /* 10 us delay */
    delay_us(10);

    /* write to and read from spi */
    while (WriteSPI(((data & 0x0f0000UL) >> 16)));
    spi_rx[0] = SSPBUF;

    while (WriteSPI(((data & 0xff00UL) >> 8)));
    spi_rx[1] = SSPBUF;

    while (WriteSPI((data & 0xffUL)));
    spi_rx[2] = SSPBUF;

    /* CS_N must be raised and be kept high for a while */
    CS_N = 1;

    /* 10 us delay */
    delay_us(10);
    return;
}

void motor_enable_disable(char unit, char enable) {
    u32 value;

    /* chopconf must be start with binary 10011... */
    value = ((chopconf[unit] & ~(0x0f8000)) | 0x098000);
    if (!enable) {
        value &= 0x0ffff0;
    }
    write_spi_chain(unit, value);
}

void motor_enable(char unit) {

    /* debug only, set current scale */
    write_spi_chain(unit, 0x0c0000UL | debug_value);

    /* enable unit */
    motor_enable_disable(unit, 1);
}

void motor_disable(char unit) {
    motor_enable_disable(unit, 0);
}

/* read from eeprom and copy into registers */
void copy_from_eeprom(char unit) {

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
    value = eeprom_reg((u8)(offset + EEPROM_DRVCONF), DRVCONF_VALUE, DRVCONF_MASK);
    write_spi_chain(unit, value);

    /* EEPROM_SGCSCONF */
    value = eeprom_reg((u8)(offset + EEPROM_SGCSCONF), SGCSCONF_VALUE, SGCSCONF_MASK);
    write_spi_chain(unit, value);

    /* EEPROM_SMARTEN */
    value = eeprom_reg((u8)(offset + EEPROM_SMARTEN), SMARTEN_VALUE, SMARTEN_MASK);
    write_spi_chain(unit, value);

    /* EEPROM_CHOPCONF */
    value = eeprom_reg((u8)(offset + EEPROM_CHOPCONF), CHOPCONF_VALUE, CHOPCONF_MASK);
    chopconf[unit] = value;
    /* make sure TOFF is 0 to disable the motor */
    write_spi_chain(unit, value & 0x0ffff0);

    /* EEPROM_DRVCTRL */
    value = eeprom_reg((u8)(offset + EEPROM_DRVCTRL), DRVCTRL_VALUE, DRVCTRL_MASK);
    write_spi_chain(unit, value);
}

unsigned char blank_check(char unit) {
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
    unsigned char value;

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