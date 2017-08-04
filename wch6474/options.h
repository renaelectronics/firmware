#ifndef OPTIONS_H
#define OPTIONS_H

struct motor_options{
	char parport[1024];
	int motor;
	int version;
	float current;
	float pwm_off;
	float t_fast;
	float t_step;
	float ton_min;
	float toff_min;
	float ocd_th;
	int step_mode;
	int config;
	int readinfo;
	int console;
	int strobe;
};

int get_motor_options(int argc, char **argv, struct motor_options *p);

#endif
