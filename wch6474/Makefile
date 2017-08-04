#
# 'make depend' uses makedepend to automatically generate dependencies
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = gcc

# define any compile-time flags
CFLAGS = -Wall -O2 -pipe

# define any directories containing header files other than /usr/include
#
# INCLUDES = -I/home/newhall/include  -I../include
INCLUDES = -I./

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
#LFLAGS = -L/home/newhall/lib  -L../lib

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname
#   option, something like (this will link in libmylib.so and libm.so:
#LIBS = -lmylib -lm

# define the C source files
SRCS = wch6474.c options.c parport.c

# define the C object files
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.c=.o)

# define the executable file
MAIN = wch6474

#
# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it

wch6474.o: /usr/include/ctype.h /usr/include/features.h /usr/include/endian.h
wch6474.o: /usr/include/xlocale.h /usr/include/stdio.h /usr/include/libio.h
wch6474.o: /usr/include/_G_config.h /usr/include/wchar.h
wch6474.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/string.h
wch6474.o: /usr/include/unistd.h /usr/include/getopt.h /usr/include/fcntl.h
wch6474.o: /usr/include/time.h /usr/include/linux/ioctl.h 6474.h options.h
wch6474.o: parport.h
options.o: /usr/include/stdio.h /usr/include/features.h /usr/include/libio.h
options.o: /usr/include/_G_config.h /usr/include/wchar.h
options.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/getopt.h
options.o: /usr/include/string.h /usr/include/xlocale.h options.h
parport.o: /usr/include/ctype.h /usr/include/features.h /usr/include/endian.h
parport.o: /usr/include/xlocale.h /usr/include/stdio.h /usr/include/libio.h
parport.o: /usr/include/_G_config.h /usr/include/wchar.h
parport.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/string.h
parport.o: /usr/include/unistd.h /usr/include/getopt.h /usr/include/fcntl.h
parport.o: /usr/include/time.h /usr/include/linux/parport.h
parport.o: /usr/include/linux/ppdev.h 6474.h parport.h
