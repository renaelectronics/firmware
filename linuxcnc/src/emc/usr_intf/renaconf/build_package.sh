#!/bin/bash
D=Debian/renaconf/files
cp *.glade ${D}/usr/share/linuxcnc/renaconf
cp renaconf.py ${D}/usr/bin/renaconf
chmod +x ${D}/usr/bin/renaconf
cp *.py ${D}/usr/share/pyshared/renaconf/
cd ./Debian/renaconf
dpkg-buildpackage
