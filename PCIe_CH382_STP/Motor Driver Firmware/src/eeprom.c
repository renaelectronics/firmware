#include <p18cxxx.h>
#include "interrupt.h"
#include "serial.h"
#include "eeprom.h"

char read_eeprom_data(char addr) {
    /*
     * MOVLW DATA_EE_ADDR ;
     * MOVWF EEADR ; Data Memory Address to read
     * BCF EECON1, EEPGD ; Point to DATA memory
     * BCF EECON1, CFGS ; Access EEPROM
     * BSF EECON1, RD ; EEPROM Read
     * MOVF EEDATA, W ; W = EEDATA
     */
    EEADR = addr;
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.RD = 1;
    return EEDATA;
}

void write_eeprom_data(char addr, char data) {
    /*
     * MOVLW DATA_EE_ADDR ;
     * MOVWF EEADR ; Data Memory Address to write
     * MOVLW DATA_EE_DATA ;
     * MOVWF EEDATA ; Data Memory Value to write
     * BCF EECON1, EPGD ; Point to DATA memory
     * BCF EECON1, CFGS ; Access EEPROM
     * BSF EECON1, WREN ; Enable writes
     * BCF INTCON, GIE ; Disable Interrupts
     * MOVLW 55h ;
     * Required MOVWF EECON2 ; Write 55h
     * Sequence MOVLW 0AAh ;
     * MOVWF EECON2 ; Write 0AAh
     * BSF EECON1, WR ; Set WR bit to begin write
     * BSF INTCON, GIE ; Enable Interrupts
     * ; User code execution
     * BCF EECON1, WREN ; Disable writes on write complete (EEIF set)
     */
    PIR2bits.EEIF = 0;
    EEADR = addr;
    EEDATA = data;
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1;
    INTCONbits.GIE = 0;
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    while (!PIR2bits.EEIF);
    INTCONbits.GIE = 1;
    EECON1bits.WREN = 0;
}
