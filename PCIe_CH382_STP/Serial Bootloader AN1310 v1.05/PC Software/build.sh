#!/bin/bash
make -C ./build-QextSerialPort-Desktop_Qt_5_5_1_GCC_32bit-Debug -f Makefile
make -C ./build-Bootload-Desktop_Qt_5_5_1_GCC_32bit-Debug -f Makefile
make -C ./build-AN1310cl-Desktop_Qt_5_5_1_GCC_32bit-Debug -f Makefile clean
make -C ./build-AN1310cl-Desktop_Qt_5_5_1_GCC_32bit-Debug -f Makefile
sudo ./build-AN1310cl-Desktop_Qt_5_5_1_GCC_32bit-Debug/AN1310cl -b 2400 -e
