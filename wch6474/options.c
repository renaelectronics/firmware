#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "options.h"

/* Flag set by --verbose */
static int verbose_flag;

static void print_example(int argc, char **argv){
	printf("\n");
	printf("EXAMPLE: \n");
	printf("\n");
	printf("    %s --port /dev/parport0  --motor 1 --current 2.0 --step_mode 16 --ocd_th 3\n", argv[0]);
	printf("\n");
	printf("    Use parallel port /dev/parport0 to configure motor\n");
	printf("    with drive current to 2.0A, step mode to 16 steps and\n");
	printf("    over detection current detection to 3A\n");
	printf("\n");
	printf("SHORT HAND EXAMPLE:\n");
	printf("\n");
	printf("    %s -p /dev/parport0 -m 1 -c 2.0 -s 16 -d 3\n", argv[0]);
	printf("\n");
	printf("    Use serial port /dev/parport0 to configure motor\n");
	printf("    with drive current to 2.0A, step mode to 16 steps and\n");
	printf("    over detection current detection to 3A\n");
	printf("\n");
}

static void print_usage(int argc, char **argv){
	printf("\n");
	printf("EXAMPLE: %s --motor 0 --current 2.0\n", argv[0]);
	printf("    Set motor 0 with max drive current of 2A\n");
	printf("\n");
 	printf("SHORT HAND EXAMPLE: %s -m 0 -c 2.0\n", argv[0]);
	printf("    Set motor 0 with max drive current of 2A\n");
	printf("\n");
	printf("DETAIL USAGE: %s [OPTION...] \n", argv[0]);
	printf("\n");
	printf("OPTION:\n");
	printf("     ?                 print usage and example message\n");
	printf("    -h, --help         print usage and example message\n");
	printf("    -x, --example      print details example\n");
	printf("    -z, --console      read from device\n");
	printf("    -a, --force        force strobe to on for 5 seconds\n");
	printf("    -v, --version      read firmware version from device\n");
	printf("    -d, --device       parport port device name, default is /dev/parport0\n");
	printf("    -m, --motor        motor unit\n");
	printf("    -r, --read         read motor setting information\n");
	printf("    -c, --curent       drive current, 0.03125 to 4.00 (A)\n");
	printf("    -w, --pwm_off      PWM off time, 4.0 to 124.0 (usec)\n");
	printf("    -t, --t_fast       fast decay time, 2.0 to 32.0 (usec)\n");
	printf("    -e, --t_step       fall step time, 2.0 to 32.0 (usec)\n");
	printf("    -o, --ton_min      minimum on time, 0.5 to 64.0 (usec)\n");
	printf("    -f, --toff_min     minimum off time, 0.5 to 64.0 (usec)\n");
	printf("    -t, --ocd_th       over current detection threshold\n");
	printf("    -s, --step_mode    0=full, 1=half, 2=1/4 step, 3=1/8 step, 4=1/16 step\n");
	printf("\n");
}

int get_motor_options(int argc, char **argv, struct motor_options *p)
{
	int c;
	char *endptr;

	if (argc == 1){
		print_usage(argc, argv);
		return 0;
	}

	while (1){
		static struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"example", no_argument, 0, 'x'},
			{"console", no_argument, 0, 'z'},
			{"force", no_argument, 0, 'a'},
			{"version", no_argument, 0, 'v'},
			{"port", required_argument, 0, 'p'},
			{"motor", required_argument, 0, 'm'},
			{"read", no_argument, 0, 'r'},
			{"current", required_argument, 0, 'c'},
			{"pwm_off", required_argument, 0, 'w'},
			{"t_fast", required_argument, 0, 't'},
			{"t_step", required_argument, 0, 'e'},
			{"ton_min", required_argument, 0, 'o'},
			{"toff_min", required_argument, 0, 'f'},
			{"ocd_th", required_argument, 0, 'd'},
			{"step_mode", required_argument, 0, 's'},
			{0, 0, 0, 0}
		};

		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "avzrhxp:m:c:w:t:e:o:f:d:s:", long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c){
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (long_options[option_index].flag != 0)
					break;
				printf ("option %s", long_options[option_index].name);
				if (optarg)
					printf (" with arg %s", optarg);
				printf ("\n");
				return 0;
				break;

			case 'a':
				p->strobe = 1;
				break;

			case 'z':
				p->console = 1;
				break;

			case 'v':
				p->version = 1;
				break;

			case 'h':
				print_usage(argc, argv);
				return 0;
				break;

			case 'x':
				print_example(argc, argv);
				return 0;
				break;

			case 'p':
				strcpy(p->parport, optarg);
				break;

			case 'm':
				/* strtol will return 0 if the optarg is invalid */
				p->motor = strtol(optarg, &endptr, 10);
				break;

			case 'r':
				p->readinfo = 1;
				break;

			case 'c':
				p->current = atof(optarg);
				break;
			case 'w':
				p->pwm_off = atof(optarg);
				break;

			case 't':
				p->t_fast = atof(optarg);
				break;

			case 'e':
				p->t_step = atof(optarg);
				break;

			case 'o':
				p->ton_min = atof(optarg);
				break;

			case 'f':
				p->toff_min = atof(optarg);
				break;

			case 'd':
				p->ocd_th = atof(optarg);
				break;

			case 's':
				p->step_mode = strtol(optarg, &endptr, 10);
				break;

			case '?':
				/* getopt_long already printed an error message. */
				break;

			default:
				print_usage(argc, argv);
				return 0;
				break;
		}
	}

	/* Instead of reporting ‘--verbose’
	   and ‘--brief’ as they are encountered,
	   we report the final status resulting from them. */

	if (verbose_flag)
		puts ("verbose flag is set");

	/* Print any remaining command line arguments (not options). */
	if (optind < argc){
		while (optind < argc){
			if (strcmp(argv[optind], "?")){
				printf ("\nUnknown option: %s\n", argv[optind]);
			}
			optind++;
		}
		putchar ('\n');

		/* print a usage message */
		print_usage(argc, argv);
		return 0;
	}

	return 1;
}
