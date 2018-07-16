#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/ioctl.h>
#include "6474.h"
#include "options.h"
#include "parport.h"

/* ctrl-c signal handler */
int main_running= 1;

void signal_handler(int sig)
{
	main_running = 0;
}

void dump_motor_options(struct motor_options *p)
{
	printf("---------------------------------------\n");
	printf("             Motor Unit : %d\n", p->motor);
	printf("          Motor Current : %f A\n", p->current);
	printf("             Microsteps : %d\n", p->microsteps);
	printf("   Pulse multiplication : %d\n", p->pulse_multi);
	printf("        Slow Decay Time : %d \n", p->slow_decay_time);
	printf("        Fast Decay Time : %d \n", p->chopconf_p3.fast_decay_time);
	printf("       Sine Wave Offset : %d \n", p->chopconf_p2.sine_wave_offset);
	printf("---------------------------------------\n");
}

int outof_range_float(char *message, float value, float min, float max)
{
	if ((value < min) || (value > max)){
		printf("%s out of range, must be between %f to %f, ",
			message, min, max);
		return 1;
	}
	return 0;
}

int outof_range(char *message, int value, int min, int max)
{
	if ((value < min) || (value > max)){
		printf("%s out of range, must be between %d to %d, ",
			message, min, max);
		return 1;
	}
	return 0;
}

int not_powerof_2(char *message, int value, int min, int max)
{
	int n;
	int count = 0;

	if (outof_range(message, value, min, max))
		return 1;
	for (n=0;n<32;n++){
		if (value & (1<<n))
			count++;
		if (count>1){
			printf("%s, must be power of 2, ", message);
			return 1;
		}
	}
	return 0;
}

int options_check(int argc, char **argv, struct motor_options *p)
{
	int rc = 1;

	if (outof_range("Motor unit", p->motor, 0, 3))
		rc = 0;
	else if (outof_range_float("Motor current", p->current, 0.097, 2.034))
		rc = 0;
	/* nominal slow deay time is 20us, ie register value of 9 */
	else if (outof_range("Slow decay time", p->slow_decay_time, -7, 6))
		rc = 0;
	/* nominal fast decay time is 20us, ie register value of 9 */
	else if (outof_range("Fast decay time", p->chopconf_p3.fast_decay_time, -7, 6))
		rc = 0;
	/* nominal sine wave offset is 0 ie register value of 3 */	
	else if (outof_range("Fast decay time", p->chopconf_p2.sine_wave_offset, -3, 9))
		rc = 0;
	else if (outof_range("Microsteps can only be 1,2,4,8,16,32,64,128,256",
			 	p->microsteps, 1, 256))
		rc = 0;

	if (rc == 0){
		printf("use following command for help.\n\n");
		printf("\t%s --help\n\n", argv[0]);
	}

	/* current, resolution 0.097 ie register 0 = 0.097A */
	p->current = (int)(p->current / 0.097)-1;
	if (p->current < 0) 
		p->current = 0;

	return rc;
}

/*
 * convert the options into register value and package
 * it into a datagram. Add checksum at the end
 */
void options_to_buf(struct motor_options *p, char *pbuf)
{
	int n;
	int checksum;

	/* EEPROM_DRVCONF */
	pbuf[EEPROM_DRVCONF + 0] = 0x00;
	pbuf[EEPROM_DRVCONF + 1] = 0x00;
	pbuf[EEPROM_DRVCONF + 2] = 0x00;

	/* EEPROM_SGCSCONF */
	pbuf[EEPROM_SGCSCONF + 0] = 0x00;
	pbuf[EEPROM_SGCSCONF + 1] = 0x00;
	pbuf[EEPROM_SGCSCONF + 2] = 0x00;

	/* EEPROM_SMARTEN */
	pbuf[EEPROM_SMARTEN + 0] = 0x00;
	pbuf[EEPROM_SMARTEN + 1] = 0x00;
	pbuf[EEPROM_SMARTEN + 2] = 0x00;

	/* EEPROM_CHOPCONF */
	pbuf[EEPROM_CHOPCONF + 0] = 0x00;
	pbuf[EEPROM_CHOPCONF + 1] = 0x00;
	pbuf[EEPROM_CHOPCONF + 2] = 0x00;

	/* EEPROM_DRVCTRL */
	pbuf[EEPROM_DRVCTRL + 0] = 0x00;
	pbuf[EEPROM_DRVCTRL + 1] = 0x00;
	pbuf[EEPROM_DRVCTRL + 2] = 0x00;

	/* EEPROM_CHECK_SUM */
	pbuf[EEPROM_CHECK_SUM] = 0x00;
	for (n=0, checksum=0; n<EEPROM_CHECK_SUM; n++){
		checksum += pbuf[n];
	}
	checksum = ~checksum;
	checksum += 1;
	pbuf[EEPROM_CHECK_SUM] = checksum & 0xff;
}


/*
 * main entry
 */
int main(int argc, char **argv)
{
	char data[EEPROM_MAX_BYTE];
	struct motor_options p;
	int fd;
	int n;

	memset(&p, 0, sizeof(struct motor_options));
	strcpy(p.parport, "/dev/parport0");
	p.motor = 0;
	p.version = 0;
	p.current = 0.097;
	p.pulse_multi = 0;
	p.microsteps = 1;
	p.chopper_mode = 1; /* constant TOFF mode */
	p.random_toff = 0;
	p.chopconf_p1.fast_decay_mode = 0;
	p.chopconf_p2.sine_wave_offset = 0;
	p.chopconf_p3.fast_decay_time = 0;
	p.slow_decay_time = 0;

	if (!get_motor_options(argc, argv, &p)){
		exit(0);
	}
	
	if (!p.console){
		/* options check */
		if (!options_check(argc, argv, &p)){
			exit(0);
		}
	}

	/* initialize parallel port */
	fd = parport_init(p.parport);
	if (fd <0){
		printf("failed to initialize parallel port\n");
		exit (0);
	}

	if (p.strobe){
		/* force strobe to enable for few seconds */
		parport_strobe(0, fd);
		printf("Force strobe line to active for 10 seconds ");		
		fflush(stdout);
		for(n=0; n<10; n++){
			printf(".");
			fflush(stdout);
			sleep(1);
		}
		printf("\n");
	}

	/* parport strobe disable */
	parport_strobe(1, fd);

	/* debug console */
	if (p.console){
		pulse_HOST_CS(fd);
		for(;;){
			while (get_HOST_SDI(fd) == 0){
				if (!main_running)
					goto out;
			}
			receive_packet_raw(data, 1, fd);
			printf("%c", data[0]);
			fflush(stdout);
			if (!main_running)
				goto out;
		}
	}

	/* read firmware version */
	if (p.version){
		memset(data, 0, sizeof(data));

		/* reset target */
		pulse_HOST_CS(fd);

		/* send a character 'v' to device */
		data[0]='v';
		send_packet_raw(data, 1, fd);

		/* wait for target data ready */
		while (get_HOST_SDI(fd) == 0){
			if (!main_running)
				goto out;
		}
		receive_packet_raw(data, 1, fd);
		printf("Device Firmware Version: %x\n", data[0]);
		fflush(stdout);
		goto out;
	}
		
	/* write and read device info */
	if (!p.readinfo){

		/* dump_motor_options */
		dump_motor_options(&p);

		/* reset target */
		pulse_HOST_CS(fd);

		/* prepare data */
		memset(data, 0, EEPROM_MAX_BYTE);
		options_to_buf(&p, data);

		/* write motor data */
		printf("Programming ...");
		fflush(stdout);
		send_packet(p.motor, data, EEPROM_MAX_BYTE, fd);
		printf("\n");
	
	}

	/* always read the device info */
	memset(data, 0, sizeof(data));

	/* reset target */
	pulse_HOST_CS(fd);

	/* get device data */
	receive_packet(p.motor, data, EEPROM_MAX_BYTE, fd);
	dump_data(data, EEPROM_MAX_BYTE);

out:
	/* close and exit */
	parport_exit(fd);
	exit(0);

}
