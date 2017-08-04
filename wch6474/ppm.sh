#!/bin/sh
echo "### Setting driver for pick and place machine ###"
sudo ./mtutil -m 1 -c 2 -s 3 -w 30
sudo ./mtutil -m 3 -c 2 -s 3 -w 30
sudo ./mtutil -m 2 -c 1.5 -s 3
sudo ./mtutil -m 4 -c 2.8 -s 0
