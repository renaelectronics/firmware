#ifndef OPTIONS_H
#define OPTIONS_H

struct motor_options{
	char parport[1024];
	/* motor number */
	int motor;
	int version;
	/* Driver Control Register, DRVCTRL */
	int pulse_multi;	/* step interpolation */
	int double_edge;	/* double edge */
	int microsteps;		/* microstep resolution */
	/* Chopper Control Register, CHOPCONF */
	int blanking_time;
	int chopper_mode; 	/* auto fast decay */
	int random_toff;	/* random TOFF */
	union {
		int hysteresis_dec; 	/* HDEC */
		union {
			int hdec1;
			int hdec0;
		} fast_decay_mode;
	} chopconf_p1;
	union {
		int hyteresis_end;	/* HEND */
		int sine_wave_offset;	
	} chopconf_p2;
	union {
		int hysteresis_start;	/* HSTRT */
		int fast_decay_time;
	} chopconf_p3;
	/* ... spreadCycle and constant toff mode */
	int slow_decay_time;		/* TOFF */
	/* coolStep Control Register, SMARTEN */
	int coolstep_min_cur;		/* SEIMIN */
	int coolstep_dec_speed;		/* SEND */
	int coolstep_upper_thres;	/* SEMAX */
	int coolstep_inc_size;		/* SEUP */
	int coolstep_lower_thres;	/* SEMIN */
	/* stallGuard2 Control Reigster, SGCSCONF */
	int stallGuard_filter;		/* SFILT */
	int stallGuard_thres;		/* SGT */
	float current;			/* CS */
	/* Driver Configure Register, DRVCONF */
	int mosfet_hi_slope;		/* SLPH */
	int mosfet_lo_slope;		/* SLPL */
	int short_detect_enable;	/* DISS2G */
	int short_dectect_timer;	/* TS2G */
	int step_dir_disable;		/* SDOFF */
	int sense_voltage;		/* VSENSE */
	int read_selection;		/* RSEL */
	/* Others */
	int config;
	int readinfo;
	int console;
	int strobe;
	int dump_only;
};

int get_motor_options(int argc, char **argv, struct motor_options *p);

#endif
