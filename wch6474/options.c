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
	printf("    %s --port /dev/parport0  --motor 1 --current 1.0 --microsteps 16\n", argv[0]);
	printf("\n");
	printf("    Use parallel port /dev/parport0 to configure motor\n");
	printf("    with drive current to 1.0A, 16 microsteps and\n");
	printf("\n");
	printf("SHORT HAND EXAMPLE:\n");
	printf("\n");
	printf("    %s -p /dev/parport0 -m 1 -c 1.0 -s 16\n", argv[0]);
	printf("\n");
	printf("    Use serial port /dev/parport0 to configure motor\n");
	printf("    with drive current to 1.0A, 16 microsteps\n");
	printf("\n");
}

static void print_usage(int argc, char **argv){
	printf("\n");
	printf("EXAMPLE: %s --motor 0 --current 1.0\n", argv[0]);
	printf("    Set motor 0 with max drive current of 1A\n");
	printf("\n");
 	printf("SHORT HAND EXAMPLE: %s -m 0 -c 1.0\n", argv[0]);
	printf("    Set motor 0 with max drive current of 1A\n");
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
	printf("    -p, --device       parport port device name, default is /dev/parport0\n");
	printf("    -m, --motor        motor unit\n");
	printf("    -r, --read         read motor setting information\n");
	printf("    -c, --curent       drive current, 0.097 to 2.034 (A)\n");
	printf("    -w, --t_off        PWM off time setting\n");
	printf("    -t, --t_fast       fast decay time\n");
	printf("    -o, --t_offset     sinewave offset\n");
	printf("    -s, --microsteps   1,2,4,8,16,32,64,128,256 microsteps\n");
	printf("    -e, --pulse_multi  enable step pulse multiplication by 16\n");
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
			{"t_off", required_argument, 0, 'w'},
			{"t_fast", required_argument, 0, 't'},
			{"t_offset", required_argument, 0, 'o'},
			{"microsteps", required_argument, 0, 's'},
			{"pulse_multi", no_argument, 0, 'e'},
			{0, 0, 0, 0}
		};

		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "avezrhxp:m:c:w:t:o:s:", long_options, &option_index);

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

			case 'h':
				print_usage(argc, argv);
				return 0;
				break;
			case 'x':
				print_example(argc, argv);
				return 0;
				break;
			case 'z':
				p->console = 1;
				break;
			case 'a':
				p->strobe = 1;
				break;
			case 'v':
				p->version = 1;
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
				/* strtol will return 0 if the optarg is invalid */
				p->slow_decay_time = strtol(optarg, &endptr, 10);
				break;
			case 't':
				/* strtol will return 0 if the optarg is invalid */
				p->chopconf_p3.fast_decay_time = strtol(optarg, &endptr, 10);
				break;
			case 'o':
				/* strtol will return 0 if the optarg is invalid */
				p->chopconf_p2.sine_wave_offset = strtol(optarg, &endptr, 10);
				break;
			case 's':
				/* strtol will return 0 if the optarg is invalid */
				p->microsteps = strtol(optarg, &endptr, 10);
				break;
			case 'e':
				p->pulse_multi = 1;
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
