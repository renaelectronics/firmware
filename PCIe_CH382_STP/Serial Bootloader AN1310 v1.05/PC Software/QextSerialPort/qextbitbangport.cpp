#include <stdio.h>
#include "qextbitbangport.h"

QextBitBangPort::QextBitBangPort() : QextBaseType()
{
}

/*!
Constructs a serial port attached to the port specified by name.
name is the name of the device, which is windowsystem-specific,
e.g."COM1" or "/dev/ttyS0".
*/
QextBitBangPort::QextBitBangPort(const QString & name) : QextBaseType(name)
{
}

/*!
Constructs a port with default name and settings specified by the settings parameter.
*/
QextBitBangPort::QextBitBangPort(PortSettings const& settings) : QextBaseType(settings)
{
}

/*!
Constructs a port with the name and settings specified.
*/
QextBitBangPort::QextBitBangPort(const QString & name, PortSettings const& settings) : QextBaseType(name, settings)
{
}

/*!
\fn QextBitBangPort::~QextBitBangPort()
Standard destructor.
*/
QextBitBangPort::~QextBitBangPort()
{}
