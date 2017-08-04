#!/bin/bash
DIR=`pwd`
echo "build AN1310cl ... "
cd ~/rena/firmware/PCIe_CH382_STP/Serial\ Bootloader\ AN1310\ v1.05/PC\ Software/
./build.sh
cp ./AN1310cl/AN1310cl $DIR/linuxcnc/src/emc/usr_intf/renaconf/Debian/renaconf/files/usr/bin
cd $DIR/linuxcnc/src/emc/usr_intf/renaconf
./build.sh
