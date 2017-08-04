#!/bin/bash
D=Debian/renaconf/files
echo "remove old package files"
rm -f ./Debian/*.deb
rm -f ./Debian/renaconf/debian/renaconf/usr/bin/AN1310cl
rm -f ./Debian/renaconf/files/usr/bin/AN1310cl
rm -f ./Debian/renaconf/debian/renaconf/DEBIAN/control
rm -f ./Debian/renaconf/debian/renaconf/DEBIAN/md5sums
rm -f ./Debian/renaconf/debian/renaconf/DEBIAN/postinst
rm -f ./Debian/renaconf_1.0.0.dsc
rm -f ./Debian/renaconf_1.0.0_i386.changes

# clean only
if [ "$1" = "clean" ]; then
	echo "clean only"
	exit
fi

echo "copying glade files"
cp *.glade ${D}/usr/share/linuxcnc/renaconf

echo "copying py scripts"
cp renaconf.py ${D}/usr/bin/renaconf
chmod +x ${D}/usr/bin/renaconf
cp *.py ${D}/usr/share/pyshared/renaconf/
cd ./Debian/renaconf

echo "build debian package: use sudo dpkg -i <packagename>"
dpkg-buildpackage
