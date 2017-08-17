#!/usr/bin/python
import time
import signal
import sys
import argparse
import fcntl
import os

# glabol
main_running=True

# parallel port status register, base + 1
STATUS_ERROR=(0x08)         # pin 15 
STATUS_SELECT=(0x10)        # pin 13 
STATUS_PAPER_OUT= (0x20)    # pin 12
STATUS_ACK=(0x40)           # pin 10
STATUS_BUSY=(0x80)          # pin 11 

# parallel port control register, base + 2
CTRL_STROBE=(0x01)          # pin 1 
CTRL_AUTOFEED=(0x02)        # pin 14 
CTRL_INIT=(0x04)            # pin 16 
CTRL_SELECT_IN=(0x08)       # pin 17

# host bit banding 
HOST_STROBE=(CTRL_STROBE)   # crtl addr, bit 0
HOST_CLK=(0x01)             # data addr, bit 0
HOST_SDO=(0x02)             # data addr, bit 1
HOST_CS=(0x04)              # data addr, bit 2

# options index
OPT_SHORT=0
OPT_LONG=1
OPT_DEFAULT=2
OPT_DESC=3
OPT_MIN=4
OPT_MAX=5
OPT_UNIT=6

# EEPROM location 
EEPROM_ABS_POS=(0)
EEPROM_EL_POS=(EEPROM_ABS_POS + 3)
EEPROM_MARK=(EEPROM_EL_POS + 2)
EEPROM_TVAL=(EEPROM_MARK + 3)
EEPROM_T_FAST=(EEPROM_TVAL + 1)
EEPROM_TON_MIN=(EEPROM_T_FAST + 1)
EEPROM_TOFF_MIN=(EEPROM_TON_MIN + 1)
EEPROM_ADC_OUT=(EEPROM_TOFF_MIN + 1)
EEPROM_OCD_TH=(EEPROM_ADC_OUT + 1)
EEPROM_STEP_MODE=(EEPROM_OCD_TH + 1)
EEPROM_ALARM_EN=(EEPROM_STEP_MODE + 1)
EEPROM_CONFIG=(EEPROM_ALARM_EN + 1)
EEPROM_STATUS=(EEPROM_CONFIG + 2)
EEPROM_CHECK_SUM=(EEPROM_STATUS + 2)
EEPROM_MAX_BYTE=(EEPROM_CHECK_SUM + 1)
EEPROM_OFFSET=(0x20)

if EEPROM_MAX_BYTE > EEPROM_OFFSET:
    print "PROGRAM ERROR: EEPROM_MAX_BYTE > EEPROM_OFFSET"
    sys.exit()

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

# function: handle control-C
def signal_handler(signal, frame):
    global main_running
    print "xxxx"
    main_running=False
    return

# function: convert string to string/int/float value
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

# function:
def wait_width:
    # 1/195200 = 52us
    time.sleep(52/1000000.0)
    return 

# function:
def pulse_HOST_CS(int fd):
    # data = 0
    data=array.array('b',[0])

    # HOST_CS = 0 
    fcntl.ioctl(fd, PPWDATA, data)
    time.sleep(0.01)

    # HOST_CS = 1 
    data[0] = data[0] | HOST_CS;
    fcntl.ioctl(fd, PPWDATA, data)
    time.sleep(0.01)

    # HOST_CS = 0 
    data[0] = 0;
    fcntl.ioctl(fd, PPWDATA, data)
    time.sleep(0.01)
    return

def parport_init(devname):
    # open parport
    try:
        fd = os.open(devname, os.O_RDWR)
    except OSError:
        print "Could not open parallel port, remember to modprobe ppdev"

    # claim parport
    try:
        fnctl.ioctl(fd, PPCLAIM)
    except IOError:
        print "Could not claim parallel port"
 

    return

# main
signal.signal(signal.SIGINT, signal_handler)
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

# create a byte array from the options
data = bytearray(EEPROM_MAX_BYTE)





#    print string
#    setattr(args, attr_name, 

#print getattr(args, 'motor')
#print args.motor
#print args.example
