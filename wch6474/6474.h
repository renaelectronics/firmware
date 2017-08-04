#ifndef _6474_H
#define _6474_H

#define msleep(a)	do { /*printf("."); fflush(stdout); */usleep(a*1000); } while (0)

/* EEPROM location */
#define EEPROM_ABS_POS      (0)
#define EEPROM_EL_POS       (EEPROM_ABS_POS + 3)
#define EEPROM_MARK         (EEPROM_EL_POS + 2)
#define EEPROM_TVAL         (EEPROM_MARK + 3)
#define EEPROM_T_FAST       (EEPROM_TVAL + 1)
#define EEPROM_TON_MIN      (EEPROM_T_FAST + 1)
#define EEPROM_TOFF_MIN     (EEPROM_TON_MIN + 1)
#define EEPROM_ADC_OUT      (EEPROM_TOFF_MIN + 1)
#define EEPROM_OCD_TH       (EEPROM_ADC_OUT + 1)
#define EEPROM_STEP_MODE    (EEPROM_OCD_TH + 1)
#define EEPROM_ALARM_EN     (EEPROM_STEP_MODE + 1)
#define EEPROM_CONFIG       (EEPROM_ALARM_EN + 1)
#define EEPROM_STATUS       (EEPROM_CONFIG + 2)
#define EEPROM_CHECK_SUM    (EEPROM_STATUS + 2)
#define EEPROM_MAX_BYTE     (EEPROM_CHECK_SUM + 1)
#define EEPROM_OFFSET       (0x20)

#if EEPROM_MAX_BYTE > EEPROM_OFFSET
	#error EEPROM_MAX_BYTE >= EEPROM_OFFSET
#endif

#endif
