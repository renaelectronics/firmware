#!/bin/bash
DIR=`pwd`
USRBIN=$DIR/linuxcnc/src/emc/usr_intf/renaconf/Debian/renaconf/files/usr/bin
DEB=renaconf_1.0.0_i386.deb

# AN1310cl
echo "build AN1310cl ... "
cd $DIR/PCIe_CH382_STP/Serial\ Bootloader\ AN1310\ v1.05/PC\ Software/
./build.sh $1
if [ -e ./AN1310cl/AN1310cl ]; then
	cp ./AN1310cl/AN1310cl $USRBIN
fi

# wch6474
echo "build wch6474 ... "
cd $DIR/wch6474
./build.sh $1
if [ -e wch6474 ]; then
	cp wch6474 $USRBIN
fi

# renaconf and create package
cd $DIR/linuxcnc/src/emc/usr_intf/renaconf
./build.sh $1

cd $DIR
rm -f $DEB
A=`find -name $DEB` 
if [ ! -z $A ]; then
	ln -s $A $DEB	
fi


	
