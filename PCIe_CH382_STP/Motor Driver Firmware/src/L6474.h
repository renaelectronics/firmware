/* 
 * File:   L6474.h
 * Author: Thomas Tai
 *
 * Created on August 26, 2013, 9:04 PM
 */

#ifndef L6474_H
#define	L6474_H

/* typedef */
typedef unsigned char u8;
typedef unsigned long u32;

/* Motor number must start at 0 */
#define M1                  (0)
#define M2                  (1)
#define M3                  (2)
#define M4                  (3)

/* EEPROM location */
#define EEPROM_DRVCONF      (0)
#define EEPROM_SGCSCONF     (1*3)
#define EEPROM_SMARTEN      (2*3)
#define EEPROM_CHOPCONF     (3*3)
#define EEPROM_DRVCTRL      (4*3)
#define EEPROM_CHECK_SUM    (5*3)
#define EEPROM_MAX_BYTE     (EEPROM_CHECK_SUM + 1)
#define EEPROM_OFFSET       (0x20)
#if (EEPROM_MAX_BYTE >= EEPROM_OFFSET)
#error "EEPROM_MABYTE >  EEPROM_OFFSET"
#endif

/* functions prototype */
void flush_spi(void);
void write_cmd(unsigned char cmd, char unit);

/* init, enable, disable motor */
void motor_init(void);
void motor_enable(char unit);
void motor_disable(char unit);

/* helper function to return eeprom offset */
unsigned char get_eeprom_offset(char unit);

/* read from eeprom and copy into registers */
void copy_from_eeprom(char unit);

/* eeprom blank check and chksum check */
unsigned char blank_check(char unit);
unsigned char chksum_check(char unit);

extern unsigned char spi_rx[3];

#endif	/* L6474_H */

