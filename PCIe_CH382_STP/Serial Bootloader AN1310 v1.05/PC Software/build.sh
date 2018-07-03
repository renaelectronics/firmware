#!/bin/bash
QMAKE=/home/thomastai/Qt/5.5/gcc/bin/qmake
make clean
rm -f ./AN1310cl/AN1310cl
rm -f ./AN1310ui/AN1310ui
rm -f ./AN1310ui/an1310ui_plugin_import.cpp
rm -f `find -name moc*`
rm -f `find -name *.a`

# clean only
if [ "$1" = "clean" ]; then
	echo "clean only"
	exit
fi

# qmake to generate Makefile then make
for i in Bootload QextSerialPort AN1310ui AN1310cl
do
	cd $i
	${QMAKE}
	cd ../
done
make
find . -name AN1310cl
