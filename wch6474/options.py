#!/usr/bin/python
import sys
import argparse

# constant
OPT_SHORT=0
OPT_LONG=1
OPT_DEFAULT=2
OPT_DESC=3
OPT_MIN=4
OPT_MAX=5
OPT_UNIT=6

# options which take string as argument
string_options = [ \
    ['-m', '--motor', 0, 'motor unit', 0, 3,''], \
    ['-s', '--step_mode', 0, '0=full, 1=half, 2=1/4 step, 3=1/8 step, 4=1/16 step', 0, 4, ''], \
    ['-p', '--port', '/dev/partport0', 'parport port name, default is /dev/parport0', None, None, ''], \
    ['-c', '--current', 0.03125, 'drive current', 0.03125, 4.0, 'A'], \
    ['-w', '--pwm_off', 8.0, 'PWM off time, 4.0 to 124.0', 4.0, 124.0, 'us'], \
    ['-t', '--t_fast', 4.0, 'fast decay time, 2.0 to 6.0', 2.0, 32.0, 'us'], \
    ['-e', '--t_step', 20.0, 'fall step time, 2.0 to 32.0', 2.0, 32.0, 'us'], \
    ['-o', '--ton_min',0.5, 'minimum on time, 0.5 to 64.0', 0.5, 64.0, 'us'], \
    ['-f', '--toff_min',0.5, 'minimum off time, 0.5 to 64.0', 0.5, 64.0, 'us'], \
    ['-d', '--ocd_th', 3.0, 'over current detection threshold', 0.375, 6.0, 'A'], \
]

# boolean argument
bool_options = [ \
    ['-x', '--example', None, 'print details example'], \
    ['-z', '--console', None, 'read from device'], \
    ['-a', '--force', None, 'force strobe to on for 5 seconds'], \
    ['-v', '--version', None, 'read firmware version from device'], \
    ['-r', '--read', None, 'read motor setting information'], \
]

# function
def agrument_to_value(string, min, max):
    rc = string
    try:    
        if type(string) == type(""):
            # string value
            if type(min) == type(None):
                return string 

            # integer value
            if type(min) == type(0):
                rc = int(string)

            # float value
            if type(min) == type(0.0):
                rc = float(string)

            # check range
            if (rc < min) or (rc > max):
                return None

    except ValueError:
        rc = None

    return rc

# main        
parser = argparse.ArgumentParser(description='Rena Motor Driver Configuration Program')

# add string agrument
for option in string_options:
    parser.add_argument(option[OPT_SHORT], option[OPT_LONG], \
                        default=option[OPT_DEFAULT], help=option[OPT_DESC])

# add boolean agrument
for option in bool_options:
    parser.add_argument(option[OPT_SHORT], option[OPT_LONG], default=False, \
                        help=option[OPT_DESC], action='store_true')

# parser agrument
args = parser.parse_args()

# convert args to value
for option in string_options:
    # attr name, min and max value
    attr_name = option[OPT_LONG].lstrip('-')
    min = option[OPT_MIN]
    max = option[OPT_MAX]
    unit = option[OPT_UNIT]
    # value = args.attr_name
    string_value = getattr(args, attr_name)
    # convert string_value to string/int/float base on the type of min
    value = agrument_to_value(string_value, min, max)
    if (value == None):
        # show min and max value
        if (min != None):
            print 'Invalid agrument \'{} {}\', {} must be between {} and {}'.format( \
                    option[OPT_LONG], string_value, option[OPT_LONG], min, max)
            break
    setattr(args, attr_name, value)
    print '{:10} {:16} {}{}'.format(attr_name, type(value), value, unit)




#    print string
#    setattr(args, attr_name, 


#print getattr(args, 'motor')
#print args.motor
#print args.example
