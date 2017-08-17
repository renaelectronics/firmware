#!/usr/bin/python
import sys
import argparse

# options which take string as argument
string_options = [ \
    ['-m', '--motor', 0, 'motor unit', 0, 3], \
    ['-s', '--step_mode', 0, '0=full, 1=half, 2=1/4 step, 3=1/8 step, 4=1/16 step', 0, 4], \
    ['-p', '--port', '/dev/partport0', 'parport port name, default is /dev/parport0', None, None], \
    ['-c', '--current', 0.03125, 'drive current', 0.03125, 4.0], \
    ['-w', '--pwm_off', 8.0, 'PWM off time, 4.0 to 124.0 (usec)', 4.0, 124.0], \
    ['-t', '--t_fast', 4.0, 'fast decay time, 2.0 to 6.0 (usec)', 2.0, 32.0], \
    ['-e', '--t_step', 4.0, 'fall step time, 2.0 to 32.0 (usec)', 2.0, 32.0], \
    ['-o', '--ton_min',0.5, 'minimum on time, 0.5 to 64.0 (usec)', 0.5, 64.0], \
    ['-f', '--toff_min',0.5, 'minimum off time, 0.5 to 64.0 (usec)', 0.5, 64.0], \
    ['-d', '--ocd_th', 3.0, 'over current detection threshold', 0.375, 6.0], \
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
            if type(min) == None:
                return string 

            # integer value
            if type(min) == type(0):
                rc = int(string)

            # float value
            if type(min) == type(0.0):
                rc = float(string)

    except ValueError:
        print "failed to convert string"
        rc = None

    return rc
        
            


parser = argparse.ArgumentParser(description='Rena Motor Driver Configuration Program')

# string agrument
for option in string_options:
    parser.add_argument(option[0], option[1], default=option[2], help=option[3])

# boolean agrument
for option in bool_options:
    parser.add_argument(option[0], option[1], default=False, help=option[3], action='store_true')

# get the args
args = parser.parse_args()

# convert args to value
for option in string_options:
    # attr name, min and max value
    attr_name = option[1].lstrip('-')
    min = option[4]
    max = option[5]
    attr = agrument_to_value(getattr(args, attr_name), min, max)
    setattr(args, attr_name, attr)
    print attr_name, type(attr), attr    


#    print string
#    setattr(args, attr_name, 


#print getattr(args, 'motor')
#print args.motor
#print args.example
