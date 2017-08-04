#!/bin/bash
D=Debian/renaconf/files
# glade file
echo "copying glade files"
cp *.glade ${D}/usr/share/linuxcnc/renaconf

echo "copying py scripts"
cp renaconf.py ${D}/usr/bin/renaconf
chmod +x ${D}/usr/bin/renaconf
cp *.py ${D}/usr/share/pyshared/renaconf/
cd ./Debian/renaconf

echo "build debian package: use sudo dpkg -i <packagename>"
dpkg-buildpackage
