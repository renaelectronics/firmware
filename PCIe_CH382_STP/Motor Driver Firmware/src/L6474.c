#define USE_OR_MASKS
#include <p18cxxx.h>
#include <stdio.h>
#include <stdlib.h>
#include "spi.h"
#include "io.h"
#include "interrupt.h"
#include "serial.h"
#include "eeprom.h"
#include "L6474.h"

/* global variables for spi response value */
unsigned char param1;
unsigned char param2;
unsigned char param3;

/* variables */
static unsigned char spi_tx[4];
static unsigned char spi_rx[4];
static unsigned char n, i, value, offset;

static unsigned char read_spi_chain_single(char unit) {

    /*
     * select motor driver chip
     */
    CS_N = 0;
    delay_us(10);

    /* read the chain */
    while (WriteSPI(CMD_NOP));
    spi_rx[0] = SSPBUF;
    while (WriteSPI(CMD_NOP));
    spi_rx[1] = SSPBUF;
    while (WriteSPI(CMD_NOP));
    spi_rx[2] = SSPBUF;
    while (WriteSPI(CMD_NOP));
    spi_rx[3] = SSPBUF;

    /*
     * CS_N must be raised and be kept high for tdisCS (800ns)
     * in order to allow the device to decode the received command.
     */
    CS_N = 1;
    delay_us(10);

    /* return the response byte */
    if (unit == M1) return spi_rx[3];
    if (unit == M2) return spi_rx[2];
    if (unit == M3) return spi_rx[1];
    if (unit == M4) return spi_rx[0];
    return 0;
}

static void read_spi_chain(char num_response, char unit) {

    switch (num_response) {
        case 1:
            param1 = read_spi_chain_single(unit);
            break;
        case 2:
            param1 = read_spi_chain_single(unit);
            param2 = read_spi_chain_single(unit);
            break;
        case 3:
            param1 = read_spi_chain_single(unit);
            param2 = read_spi_chain_single(unit);
            param3 = read_spi_chain_single(unit);
            break;
    }
}

static void write_spi_chain(unsigned char value, char unit) {

    /* CS_N must be low to select the motor driver chip */
    CS_N = 0;

    /* 10 us delay */
    delay_us(10);

    /* prepare temporary command chain */
    spi_tx[0] = spi_tx[1] = spi_tx[2] = spi_tx[3] = CMD_NOP;
    if (unit == M1) spi_tx[3] = value;
    if (unit == M2) spi_tx[2] = value;
    if (unit == M3) spi_tx[1] = value;
    if (unit == M4) spi_tx[0] = value;

    /* write to SPI */
    while (WriteSPI(spi_tx[0]));
    while (WriteSPI(spi_tx[1]));
    while (WriteSPI(spi_tx[2]));
    while (WriteSPI(spi_tx[3]));

    /* CS_N must be raised and be kept high for tdisCS (800ns)
     * in order to allow the device to decode the received command.
     */
    CS_N = 1;

    /* 10 us delay */
    delay_us(10);
    return;
}

/* set, get ABS_POS */
void set_abs_pos(char unit) {
    /* set ABS_POS */
    write_spi_chain(CMD_SETPARAM | ABS_POS, unit);
    write_spi_chain(param1, unit);
    write_spi_chain(param2, unit);
    write_spi_chain(param3, unit);
}

void get_abs_pos(char unit) {
    /* read ABS_POS and return 3 byte response value */
    write_spi_chain(CMD_GETPARAM | ABS_POS, unit);
    read_spi_chain(3, unit);
}

/* set, get EL_POS */
void set_el_pos(char unit) {
    /* set ABS_POS */
    write_spi_chain(CMD_SETPARAM | EL_POS, unit);
    write_spi_chain(param1, unit);
    write_spi_chain(param2, unit);
}

void get_el_pos(char unit) {
    /* read ABS_POS and return 2 byte response value */
    write_spi_chain(CMD_GETPARAM | EL_POS, unit);
    read_spi_chain(2, unit);
}

/* set, get MARK */
void set_mark(char unit) {
    write_spi_chain(CMD_SETPARAM | MARK, unit);
    write_spi_chain(param1, unit);
    write_spi_chain(param2, unit);
    write_spi_chain(param3, unit);
}

void get_mark(char unit) {
    write_spi_chain(CMD_GETPARAM | MARK, unit);
    read_spi_chain(3, unit);
}

/* set, get TVAL */
void set_tval(char unit) {
    write_spi_chain(CMD_SETPARAM | TVAL, unit);
    write_spi_chain(param1, unit);
}

void get_tval(char unit) {
    write_spi_chain(CMD_GETPARAM | TVAL, unit);
    read_spi_chain(1, unit);
}

/* set, get T_FAST */
void set_t_fast(char unit) {
    write_spi_chain(CMD_SETPARAM | T_FAST, unit);
    write_spi_chain(param1, unit);
}

void get_t_fast(char unit) {
    write_spi_chain(CMD_GETPARAM | T_FAST, unit);
    read_spi_chain(1, unit);
}

/* set, get TON_MIN */
void set_ton_min(char unit) {
    write_spi_chain(CMD_SETPARAM | TON_MIN, unit);
    write_spi_chain(param1, unit);
}

void get_ton_min(char unit) {
    write_spi_chain(CMD_GETPARAM | TON_MIN, unit);
    read_spi_chain(1, unit);
}

/* set, get TOFF_MIN */
void set_toff_min(char unit) {
    write_spi_chain(CMD_SETPARAM | TOFF_MIN, unit);
    write_spi_chain(param1, unit);
}

void get_toff_min(char unit) {
    write_spi_chain(CMD_GETPARAM | TOFF_MIN, unit);
    read_spi_chain(1, unit);
}

/* set, get ADC_OUT */
void set_adc_out(char unit) {
    write_spi_chain(CMD_SETPARAM | ADC_OUT, unit);
    write_spi_chain(param1, unit);
}

void get_adc_out(char unit) {
    write_spi_chain(CMD_GETPARAM | ADC_OUT, unit);
    read_spi_chain(1, unit);
}

/* set, get_OCD_TH */
void set_ocd_th(char unit) {
    write_spi_chain(CMD_SETPARAM | OCD_TH, unit);
    write_spi_chain(param1, unit);
}

void get_ocd_th(char unit) {
    write_spi_chain(CMD_GETPARAM | OCD_TH, unit);
    read_spi_chain(1, unit);
}

/* set, get_STEP_MODE */
void set_step_mode(char unit) {
    write_spi_chain(CMD_SETPARAM | STEP_MODE, unit);
    write_spi_chain(param1, unit);
}

void get_step_mode(char unit) {
    write_spi_chain(CMD_GETPARAM | STEP_MODE, unit);
    read_spi_chain(1, unit);
}

/* set, get ALARM_EN */
void set_alarm_en(char unit) {
    write_spi_chain(CMD_SETPARAM | ALARM_EN, unit);
    write_spi_chain(param1, unit);
}

void get_alarm_en(char unit) {
    write_spi_chain(CMD_GETPARAM | ALARM_EN, unit);
    read_spi_chain(1, unit);
}

/* set, get CONFIG */
void set_config(char unit) {
    write_spi_chain(CMD_SETPARAM | CONFIG, unit);
    write_spi_chain(param1, unit);
    write_spi_chain(param2, unit);
}

void get_config(char unit) {
    write_spi_chain(CMD_GETPARAM | CONFIG, unit);
    read_spi_chain(2, unit);
}

void reset_position(char unit){
    param1 = 0;
    param2 = 0;
    param3 = 0;
    set_abs_pos(unit);
    set_el_pos(unit);
}

void motor_enable(char unit) {
    write_spi_chain(CMD_ENABLE, unit);
}

void motor_disable(char unit) {
    write_spi_chain(CMD_DISABLE, unit);
}

void get_status(char unit) {
    write_spi_chain(CMD_GETSTATUS, unit);
    read_spi_chain(2, unit);
}

/* helper function to return eeprom offset */
unsigned char get_eeprom_offset(char unit) {
    if (unit == M1)
        return M1 * EEPROM_OFFSET;

    if (unit == M2)
        return M2 * EEPROM_OFFSET;

    if (unit == M3)
        return M3 * EEPROM_OFFSET;

    if (unit == M4)
        return M4 * EEPROM_OFFSET;
    
    /* default out of range offset */
    return M4 * EEPROM_OFFSET;
}

/* read from eeprom and copy into registers */
void copy_from_eeprom(char unit) {
    
    /* get offset address */
    offset = get_eeprom_offset(unit);

    /* EEPROM_ABS_POS */
    param1 = read_eeprom_data(offset + EEPROM_ABS_POS);
    param2 = read_eeprom_data(offset + EEPROM_ABS_POS + 1);
    param3 = read_eeprom_data(offset + EEPROM_ABS_POS + 2);
    set_abs_pos(unit);

    /* EEPROM_EL_POS */
    param1 = read_eeprom_data(offset + EEPROM_EL_POS);
    param2 = read_eeprom_data(offset + EEPROM_EL_POS + 1);
    set_el_pos(unit);

    /* EEPROM_MARK */
    param1 = read_eeprom_data(offset + EEPROM_MARK);
    param2 = read_eeprom_data(offset + EEPROM_MARK + 1);
    param3 = read_eeprom_data(offset + EEPROM_MARK + 2);
    set_mark(unit);

    /* EEPROM_TVAL */
    param1 = read_eeprom_data(offset + EEPROM_TVAL);
    set_tval(unit);

    /* EEPROM_T_FAST */
    param1 = read_eeprom_data(offset + EEPROM_T_FAST);
    set_t_fast(unit);

    /* EEPROM_TON_MIN */
    param1 = read_eeprom_data(offset + EEPROM_TON_MIN);
    set_ton_min(unit);

    /* EEPROM_TOFF_MIN */
    param1 = read_eeprom_data(offset + EEPROM_TOFF_MIN);
    set_toff_min(unit);

    /* EEPROM_ADC_OUT */
    param1 = read_eeprom_data(offset + EEPROM_ADC_OUT);
    set_adc_out(unit);

    /* EEPROM_OCD_TH */
    param1 = read_eeprom_data(offset + EEPROM_OCD_TH);
    set_ocd_th(unit);

    /* EEPROM_STEP_MODE */
    param1 = read_eeprom_data(offset + EEPROM_STEP_MODE);
    set_step_mode(unit);

    /* EEPROM_ALARM_EN */
    param1 = read_eeprom_data(offset + EEPROM_ALARM_EN);
    set_alarm_en(unit);

    /* reset motor position to 0 */
    param1 = 0;
    param2 = 0;
    param3 = 0;
    set_abs_pos(unit);
    set_el_pos(unit);
    set_mark(unit);

    /* EEPROM_CONFIG */
    param1 = read_eeprom_data(offset + EEPROM_CONFIG);
    param2 = read_eeprom_data(offset + EEPROM_CONFIG + 1);
    set_config(unit);

}

unsigned char blank_check(char unit) {
    /* get offset address */
    offset = get_eeprom_offset(unit);
    /* blank check */
    for (n = 0; n < EEPROM_MAX_BYTE; n++) {
        if (read_eeprom_data(offset + n) != 0xff) {
            return 0;
        }
    }
    return 1;
}

unsigned char chksum_check(char unit) {
    /* get offset address */
    offset = get_eeprom_offset(unit);
    /* checksum */
    value = 0;
    for (n = 0; n < EEPROM_MAX_BYTE; n++) {
        value += read_eeprom_data(offset + n);
    }
    if (value == 0) {
        return 1;
    }
    return 0;
}