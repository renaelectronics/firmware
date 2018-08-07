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

/* minimum and maximum current setting */
#define MIN_CURRENT	(0.097)
#define MAX_CURRENT	(2.034)


/* ctrl-c signal handler */
int main_running = 1;

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
		printf("%s must be between %f to %f.\n",
			message, min, max);
		return 1;
	}
	return 0;
}

int outof_range(char *message, int value, int min, int max)
{
	if ((value < min) || (value > max)){
		printf("%s must be between %d to %d.\n",
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
			printf("%s must be power of 2.\n", message);
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
	else if (outof_range_float("Motor current", p->current, MIN_CURRENT, MAX_CURRENT))
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
	else if (not_powerof_2("Microsteps can only be 1,2,4,8,16,32,64,128,256",
			 	p->microsteps, 1, 256))
		rc = 0;

	if (rc == 0){
		printf("Use following command for help.\n\n");
		printf("\t%s --help\n\n", argv[0]);
	}

	/* current, resolution MIN_CURRENT ie register 0 = MIN_CURRENT A */
	p->current = ((int)(p->current / MIN_CURRENT)) * MIN_CURRENT;

	return rc;
}

/*
 * convert the options into register value and package
 * it into a datagram. Add checksum at the end
 */
void options_to_buf(struct motor_options *p, char *pbuf)
{
	unsigned int n;
	int checksum;
	unsigned int value32;

	/* EEPROM_DRVCONF, mask the bit size and shift */
	value32 = 0x0e0000 |
		((p->mosfet_hi_slope & 3) << 14) | 
		((p->mosfet_lo_slope & 3) << 12) |
		((p->short_dectect_timer & 3) << 8) |
		((p->step_dir_disable & 1) << 7) |
		((p->sense_voltage & 3) << 6)    |
		((p->read_selection & 3) << 4);
	pbuf[EEPROM_DRVCONF + 0] = (value32 & 0x0f0000) >> 16;
	pbuf[EEPROM_DRVCONF + 1] = (value32 & 0x00ff00) >> 8;
	pbuf[EEPROM_DRVCONF + 2] = (value32 & 0x0000ff);

	/* EEPROM_SGCSCONF */
	n = abs((unsigned int)(p->current / MIN_CURRENT) - 1);
	value32 = 0x0c0000 |
		((p->stallGuard_filter & 1) << 16) |
		((p->stallGuard_thres & 1) << 8) |
		(n & 0x1f);
	pbuf[EEPROM_SGCSCONF + 0] = (value32 & 0x0f0000) >> 16;
	pbuf[EEPROM_SGCSCONF + 1] = (value32 & 0x00ff00) >> 8;
	pbuf[EEPROM_SGCSCONF + 2] = (value32 & 0x0000ff);

	/* EEPROM_SMARTEN */
	value32 = 0x0a0000 |
		((p->coolstep_min_cur & 1) << 15) |
		((p->coolstep_dec_speed & 3) << 13) |
		((p->coolstep_upper_thres & 0xf) << 8) |
		((p->coolstep_inc_size & 3) << 5) |
		(p->coolstep_lower_thres & 0xf);
	pbuf[EEPROM_SMARTEN + 0] = (value32 & 0x0f0000) >> 16;
	pbuf[EEPROM_SMARTEN + 1] = (value32 & 0x00ff00) >> 8;
	pbuf[EEPROM_SMARTEN + 2] = (value32 & 0x0000ff);

	/* EEPROM_CHOPCONF */
	value32	= 0x080000 |
		((p->blanking_time & 3) << 15) |
		((p->chopper_mode & 1) << 14) |
		((p->random_toff & 1) <<13);
	if (p->chopper_mode == 0) {
		/* spreadCycle mode */
		value32 = value32 |
			((p->chopconf_p1.hysteresis_dec & 3) << 11) |
			((p->chopconf_p2.hyteresis_end & 0xf) << 7) |
			((p->chopconf_p3.hysteresis_start & 0xf) << 4);
	}
	else {
		/* constant toff mode */
		/* nominal fast decay time is 20 us, ie register value of 9 */
		n = (p->chopconf_p3.fast_decay_time + 9);
		/* constant toff mode */
		value32 = value32 |
			/* hdec1 current comparator can terminate fast decay phase */
			((p->chopconf_p1.fast_decay_mode.hdec1 & 1) << 12) |
			/* hdec0: MSB of fast decay timer */
			(((n & 0x8) >> 3) << 11) |
			/* nominal sine wave offset is 0, ie register value of 3 */
			(((p->chopconf_p2.sine_wave_offset & 0xf) + 3) << 7) |
			/* last 3 bits of fast decay timer */
			((n & 7) << 4);
	}
	/* nominal slow decay time is 20us, ie register value of 9 */
	value32 |= p->slow_decay_time + 9;
	pbuf[EEPROM_CHOPCONF + 0] = (value32 & 0x0f0000) >> 16;
	pbuf[EEPROM_CHOPCONF + 1] = (value32 & 0x00ff00) >> 8;
	pbuf[EEPROM_CHOPCONF + 2] = (value32 & 0x0000ff);

	/* EEPROM_DRVCTRL */
	/* convert microsteps to register value */
	for (n=0;n<=8;n++){
		if (p->microsteps & (1<<n))
			break;
	}
	value32 = 0x000000 |
		((p->pulse_multi & 1) << 9) |
		((8-n) & 0xf);
	pbuf[EEPROM_DRVCTRL + 0] = (value32 & 0x0f0000) >> 16;
	pbuf[EEPROM_DRVCTRL + 1] = (value32 & 0x00ff00) >> 8;
	pbuf[EEPROM_DRVCTRL + 2] = (value32 & 0x0000ff);

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
	p.current = MIN_CURRENT;
	p.pulse_multi = 0;
	p.microsteps = 1;
	p.chopper_mode = 1; /* constant TOFF mode */
	p.random_toff = 0;
	p.chopconf_p1.fast_decay_mode.hdec1 = 0; 
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
		printf("Force strobe line to active for 5 seconds ");		
		fflush(stdout);
		for(n=0; n<5; n++){
			printf(".");
			fflush(stdout);
			sleep(1);
		}
		printf("\n");
		parport_strobe(1, fd);
		goto out;
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
	

	/* prepare data */
	memset(data, 0, EEPROM_MAX_BYTE);
	options_to_buf(&p, data);

	/* dump the packet */
	dump_data(data, EEPROM_MAX_BYTE);

	/* dump only */
	if (p.dump_only){
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
