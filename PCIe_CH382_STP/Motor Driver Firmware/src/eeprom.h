#ifndef EEPROM_H
#define EEPROM_H

char read_eeprom_data(char addr);
void write_eeprom_data(char addr, char data);

#endif
