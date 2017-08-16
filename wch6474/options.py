#!/usr/bin/python
import argparse

# options which take string as argument
string_options = [ \
    ['-m', '--motor', 0, 'motor unit', 0, 3], \
    ['-s', '--step_mode', 0, '0=full, 1=half, 2=1/4 step, 3=1/8 step, 4=1/16 step', 0, 4], \
    ['-p', '--port', '/dev/partport0', 'parport port name, default is /dev/parport0', None, None], \
    ['-c', '--current', 0.03125, 'drive current', 0.03125, 4.0], \

]

# boolean argument
bool_options = [ \
    ['-x', '--example', None, 'print details example'], \
    ['-z', '--console', None, 'read from device'], \
    ['-a', '--force', None, 'force strobe to on for 5 seconds'], \
    ['-v', '--version', None, 'read firmware version from device'], \
    ['-r', '--read', None, 'read motor setting information'], \
]

parser = argparse.ArgumentParser(description='Rena Motor Driver Configuration Program')

# create argument parser
for option in string_options:
    parser.add_argument(option[0], option[1], default=option[2], help=option[3])

for option in bool_options:
    parser.add_argument(option[0], option[1], default=False, help=option[3], action='store_true')

args = parser.parse_args()
print args.motor
print args.example
