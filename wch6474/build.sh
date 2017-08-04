#!/bin/bash
make clean
if [ "$1" = "clean" ]; then
	echo "clean only"
	exit
fi
make
