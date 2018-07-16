#ifndef _6474_H
#define _6474_H

#define msleep(a)	do { /*printf("."); fflush(stdout); */usleep(a*1000); } while (0)

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

#if EEPROM_MAX_BYTE > EEPROM_OFFSET
	#error EEPROM_MAX_BYTE >= EEPROM_OFFSET
#endif

#endif
