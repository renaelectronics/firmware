#ifndef SERIAL_H
#define SERIAL_H

int serial_open(char * device_name);
void serial_close(int fd);
int serial_read_data(int fd, char *pchar, int size);
int serial_write_read_data(int fd, char *pchar, int size);
void dump_data(char *pdata, int size);

#endif
