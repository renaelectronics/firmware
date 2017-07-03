/* 
 * File:   L6474.h
 * Author: Thomas Tai
 *
 * Created on August 26, 2013, 9:04 PM
 */

#ifndef L6474_H
#define	L6474_H

/* L6474 unit number, must start at 0 */
#define M1                  (0)
#define M2                  (1)
#define M3                  (2)
#define M4                  (3)

/* L6474 commands */
#define CMD_NOP           (0x00)
#define CMD_SETPARAM      (0x00)
#define CMD_GETPARAM      (0x20)
#define CMD_ENABLE        (0xB8)
#define CMD_DISABLE       (0xA8)
#define CMD_GETSTATUS     (0xD0)

/* CMD registers address */
#define ABS_POS             (0x01)
#define EL_POS              (0x02)
#define MARK                (0x03)
#define TVAL                (0x09)
#define T_FAST              (0x0E)
#define TON_MIN             (0x0F)
#define TOFF_MIN            (0x10)
#define ADC_OUT             (0x12)
#define OCD_TH              (0x13)
#define STEP_MODE           (0x16)
#define ALARM_EN            (0x17)
#define CONFIG              (0x18)
#define CMD_STATUS          (0x19)

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
#if (EEPROM_MAX_BYTE >= EEPROM_OFFSET)
#error "EEPROM_MABYTE >  EEPROM_OFFSET"
#endif

/* functions prototype */
void flush_spi(void);
void write_cmd(unsigned char cmd, char unit);

/* get and set functions for all registers */

/* set, get ABS_POS */
void set_abs_pos(char unit);
void get_abs_pos(char unit);

void set_el_pos(char unit);
void get_el_pos(char unit);

void set_mark(char unit);
void get_mark(char unit);

void set_tval(char unit);
void get_tval(char unit);

void set_t_fast(char unit);
void get_t_fast(char unit);

void set_ton_min(char unit);
void get_ton_min(char unit);

void set_toff_min(char unit);
void get_toff_min(char unit);

void set_adc_out(char unit);
void get_adc_out(char unit);

void set_ocd_th(char unit);
void get_ocd_th(char unit);

void set_step_mode(char unit);
void get_step_mode(char unit);

void set_alarm_en(char unit);
void get_alarm_en(char unit);

void set_config(char unit);
void get_config(char unit);

void get_status(char unit);
void reset_position(char unit);

void motor_enable(char unit);
void motor_disable(char unit);

/* helper function to return eeprom offset */
unsigned char get_eeprom_offset(char unit);

/* read from eeprom and copy into registers */
void copy_from_eeprom(char unit);

/* eeprom blank check and chksum check */
unsigned char blank_check(char unit);
unsigned char chksum_check(char unit);

extern unsigned char param1;
extern unsigned char param2;
extern unsigned char param3;

#endif	/* L6474_H */

