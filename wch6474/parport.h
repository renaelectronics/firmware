#ifndef PARPORT_H
#define PARPORT_H

void parport_strobe(int enable, int fd);
void pulse_HOST_CS(int fd);
int get_HOST_SDI(int fd);
void send_packet(int motor, char *pdata, int num_byte, int fd);
void receive_packet(int motor, char *pdata, int num_byte, int fd);
void send_packet_raw(char *pdata, int num_byte, int fd);
void receive_packet_raw(char *pdata, int num_byte, int fd);
void dump_data(char *pdata, int size);
int parport_init(char *devname);
void parport_exit(int fd);

#endif
