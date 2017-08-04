#!/bin/bash
make clean
rm ./AN1310cl/AN1310cl
rm ./AN1310ui/AN1310ui
for i in Bootload QextSerialPort AN1310ui AN1310cl
do
	cd $i
	qmake
	cd ../
done
make
find . -name AN1310cl
