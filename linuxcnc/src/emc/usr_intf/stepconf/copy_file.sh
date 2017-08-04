#!/bin/bash
sudo cp *.glade /usr/share/linuxcnc/stepconf
sudo cp stepconf.py /usr/bin/mystepconf.py
#sudo cp pages.py /usr/lib/pymodules/python2.7/pncconf/pages.py
sudo cp pages.py /usr/lib/pymodules/python2.7/stepconf/pages.py
#sudo cp pages.py /usr/share/pyshared/pncconf/pages.py
sudo cp pages.py /usr/share/pyshared/stepconf/pages.py


