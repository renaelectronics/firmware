/*!
\class Posix_QextBitBangPort
\version 1.0.0
*/

#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
#include "posix_qextbitbangport.h"

/*!
\fn Posix_QextBitBangPort::Posix_QextBitBangPort()
Default constructor.  Note that the name of the device used by a QextSerialPort constructed with
this constructor will be determined by #defined constants, or lack thereof - the default behavior
is the same as _TTY_LINUX_.  Possible naming conventions and their associated constants are:

\verbatim

Constant         Used By         Naming Convention
----------       -------------   ------------------------
_TTY_WIN_        Windows         COM1, COM2
_TTY_IRIX_       SGI/IRIX        /dev/ttyf1, /dev/ttyf2
_TTY_HPUX_       HP-UX           /dev/tty1p0, /dev/tty2p0
_TTY_SUN_        SunOS/Solaris   /dev/ttya, /dev/ttyb
_TTY_DIGITAL_    Digital UNIX    /dev/tty01, /dev/tty02
_TTY_FREEBSD_    FreeBSD         /dev/ttyd0, /dev/ttyd1
_TTY_LINUX_      Linux           /dev/ttyS0, /dev/ttyS1
<none>           Linux           /dev/ttyS0, /dev/ttyS1
\endverbatim

This constructor assigns the device name to the name of the first port on the specified system.
See the other constructors if you need to open a different port.
*/
Posix_QextBitBangPort::Posix_QextBitBangPort()
: QextSerialBase()
{
    Posix_File = new QFile();
    init();
}

/*!
\fn Posix_QextBitBangPort::Posix_QextBitBangPort(const Posix_QextBitBangPort&)
Copy constructor.
*/
Posix_QextBitBangPort::Posix_QextBitBangPort(const Posix_QextBitBangPort& s)
 : QextSerialBase(s.port)
{
    setOpenMode(s.openMode());
    port = s.port;
    Settings.BaudRate=s.Settings.BaudRate;
    Settings.DataBits=s.Settings.DataBits;
    Settings.Parity=s.Settings.Parity;
    Settings.StopBits=s.Settings.StopBits;
    Settings.FlowControl=s.Settings.FlowControl;
    lastErr=s.lastErr;

    Posix_File=new QFile();
    Posix_File=s.Posix_File;
    memcpy(&Posix_Timeout, &s.Posix_Timeout, sizeof(struct timeval));
    memcpy(&Posix_Copy_Timeout, &s.Posix_Copy_Timeout, sizeof(struct timeval));
    memcpy(&Posix_CommConfig, &s.Posix_CommConfig, sizeof(struct termios));
    init();
}

/*!
\fn Posix_QextBitBangPort::Posix_QextBitBangPort(const QString & name)
Constructs a serial port attached to the port specified by name.
name is the name of the device, which is windowsystem-specific,
e.g."COM1" or "/dev/ttyS0".
*/
Posix_QextBitBangPort::Posix_QextBitBangPort(const QString & name)
 : QextSerialBase(name)
{
    Posix_File = new QFile();
    init();
}

/*!
\fn Posix_QextBitBangPort::Posix_QextBitBangPort(const PortSettings& settings)
Constructs a port with default name and specified settings.
*/
Posix_QextBitBangPort::Posix_QextBitBangPort(const PortSettings& settings)
 : QextSerialBase()
{
    setBaudRate(settings.BaudRate);
    setDataBits(settings.DataBits);
    setParity(settings.Parity);
    setStopBits(settings.StopBits);
    setFlowControl(settings.FlowControl);

    Posix_File=new QFile();
    setTimeout(settings.Timeout_Millisec);
    init();
}

/*!
\fn Posix_QextBitBangPort::Posix_QextBitBangPort(const QString & name, const PortSettings& settings)
Constructs a port with specified name and settings.
*/
Posix_QextBitBangPort::Posix_QextBitBangPort(const QString & name, const PortSettings& settings)
 : QextSerialBase(name)
{
    setBaudRate(settings.BaudRate);
    setDataBits(settings.DataBits);
    setParity(settings.Parity);
    setStopBits(settings.StopBits);
    setFlowControl(settings.FlowControl);

    Posix_File=new QFile();
    setTimeout(settings.Timeout_Millisec);
    init();
}

/*!
\fn Posix_QextBitBangPort& Posix_QextBitBangPort::operator=(const Posix_QextBitBangPort& s)
Override the = operator.
*/
Posix_QextBitBangPort& Posix_QextBitBangPort::operator=(const Posix_QextBitBangPort& s)
{
    setOpenMode(s.openMode());
    port = s.port;
    Settings.BaudRate=s.Settings.BaudRate;
    Settings.DataBits=s.Settings.DataBits;
    Settings.Parity=s.Settings.Parity;
    Settings.StopBits=s.Settings.StopBits;
    Settings.FlowControl=s.Settings.FlowControl;
    lastErr=s.lastErr;

    Posix_File=s.Posix_File;
    memcpy(&Posix_Timeout, &(s.Posix_Timeout), sizeof(struct timeval));
    memcpy(&Posix_Copy_Timeout, &(s.Posix_Copy_Timeout), sizeof(struct timeval));
    memcpy(&Posix_CommConfig, &(s.Posix_CommConfig), sizeof(struct termios));
    return *this;
}

void Posix_QextBitBangPort::init()
{
    return;
}

/*!
\fn Posix_QextBitBangPort::~Posix_QextBitBangPort()
Standard destructor.
*/
Posix_QextBitBangPort::~Posix_QextBitBangPort()
{
    if (isOpen()) {
        close();
    }
    Posix_File->close();
    delete Posix_File;
}

/*!
\fn void Posix_QextBitBangPort::setBaudRate(unsigned int baudRate)
Sets the baud rate of the serial port.  Note that not all rates are applicable on
all platforms.  The following table shows translations of the various baud rate
constants on Windows(including NT/2000) and POSIX platforms.  Speeds marked with an *
are speeds that are usable on both Windows and POSIX.

\note
BAUD76800 may not be supported on all POSIX systems.  SGI/IRIX systems do not support
BAUD1800.

\verbatim

  RATE          Windows Speed   POSIX Speed
  -----------   -------------   -----------
   BAUD50                 110          50
   BAUD75                 110          75
  *BAUD110                110         110
   BAUD134                110         134.5
   BAUD150                110         150
   BAUD200                110         200
  *BAUD300                300         300
  *BAUD600                600         600
  *BAUD1200              1200        1200
   BAUD1800              1200        1800
  *BAUD2400              2400        2400
  *BAUD4800              4800        4800
  *BAUD9600              9600        9600
   BAUD14400            14400        9600
  *BAUD19200            19200       19200
  *BAUD38400            38400       38400
   BAUD56000            56000       38400
  *BAUD57600            57600       57600
   BAUD76800            57600       76800
  *BAUD115200          115200      115200
   BAUD128000          128000      115200
   BAUD256000          256000      115200
\endverbatim
*/
void Posix_QextBitBangPort::setBaudRate(unsigned int baudRate)
{
    double width_us;
    LOCK_MUTEX();
    if (Settings.BaudRate != baudRate) {
        switch (baudRate) {
            case BAUD14400:
                Settings.BaudRate=BAUD9600;
                break;

            case BAUD56000:
                Settings.BaudRate=BAUD38400;
                break;

            case BAUD76800:

#ifndef B76800
                Settings.BaudRate=BAUD57600;
#else
                Settings.BaudRate=baudRate;
#endif
                break;

            case BAUD128000:
                Settings.BaudRate=BAUD115200;
                break;

            default:
                Settings.BaudRate = baudRate;
                break;
        }
    }
    width_us = (1.0 / Settings.BaudRate) * 1000000;
    pulse_width_us = (int)width_us;
    UNLOCK_MUTEX();
}

/*!
\fn void Posix_QextBitBangPort::setDataBits(DataBitsType dataBits)
Sets the number of data bits used by the serial port.  Possible values of dataBits are:
\verbatim
    DATA_5      5 data bits
    DATA_6      6 data bits
    DATA_7      7 data bits
    DATA_8      8 data bits
\endverbatim

\note
This function is subject to the following restrictions:
\par
    5 data bits cannot be used with 2 stop bits.
\par
    8 data bits cannot be used with space parity on POSIX systems.

*/
void Posix_QextBitBangPort::setDataBits(DataBitsType dataBits)
{
    LOCK_MUTEX();
    if (Settings.DataBits!=dataBits) {
        if ((Settings.StopBits==STOP_2 && dataBits==DATA_5) ||
            (Settings.StopBits==STOP_1_5 && dataBits!=DATA_5) ||
            (Settings.Parity==PAR_SPACE && dataBits==DATA_8)) {
        }
        else {
            Settings.DataBits=dataBits;
        }
    }
    if (isOpen()) {
        switch(dataBits) {

            /*5 data bits*/
            case DATA_5:
                if (Settings.StopBits==STOP_2) {
                    TTY_WARNING("Posix_QextBitBangPort: 5 Data bits cannot be used with 2 stop bits.");
                }
                else {
                    Settings.DataBits=dataBits;
                    Posix_CommConfig.c_cflag&=(~CSIZE);
                    Posix_CommConfig.c_cflag|=CS5;
                }
                break;

            /*6 data bits*/
            case DATA_6:
                if (Settings.StopBits==STOP_1_5) {
                    TTY_WARNING("Posix_QextBitBangPort: 6 Data bits cannot be used with 1.5 stop bits.");
                }
                else {
                    Settings.DataBits=dataBits;
                    Posix_CommConfig.c_cflag&=(~CSIZE);
                    Posix_CommConfig.c_cflag|=CS6;
                }
                break;

            /*7 data bits*/
            case DATA_7:
                if (Settings.StopBits==STOP_1_5) {
                    TTY_WARNING("Posix_QextBitBangPort: 7 Data bits cannot be used with 1.5 stop bits.");
                }
                else {
                    Settings.DataBits=dataBits;
                    Posix_CommConfig.c_cflag&=(~CSIZE);
                    Posix_CommConfig.c_cflag|=CS7;
                }
                break;

            /*8 data bits*/
            case DATA_8:
                if (Settings.StopBits==STOP_1_5) {
                    TTY_WARNING("Posix_QextBitBangPort: 8 Data bits cannot be used with 1.5 stop bits.");
                }
                else {
                    Settings.DataBits=dataBits;
                    Posix_CommConfig.c_cflag&=(~CSIZE);
                    Posix_CommConfig.c_cflag|=CS8;
                }
                break;
        }
    }
    UNLOCK_MUTEX();
}

/*!
\fn void Posix_QextBitBangPort::setParity(ParityType parity)
Sets the parity associated with the serial port.  The possible values of parity are:
\verbatim
    PAR_SPACE       Space Parity
    PAR_MARK        Mark Parity
    PAR_NONE        No Parity
    PAR_EVEN        Even Parity
    PAR_ODD         Odd Parity
\endverbatim

\note
This function is subject to the following limitations:
\par
POSIX systems do not support mark parity.
\par
POSIX systems support space parity only if tricked into doing so, and only with
   fewer than 8 data bits.  Use space parity very carefully with POSIX systems.

*/
void Posix_QextBitBangPort::setParity(ParityType parity)
{
    LOCK_MUTEX();
    if (Settings.Parity!=parity) {
        if (parity==PAR_MARK || (parity==PAR_SPACE && Settings.DataBits==DATA_8)) {
        }
        else {
            Settings.Parity=parity;
        }
    }
    if (isOpen()) {
        switch (parity) {

            /*space parity*/
            case PAR_SPACE:
                if (Settings.DataBits==DATA_8) {
                    TTY_PORTABILITY_WARNING("Posix_QextBitBangPort:  Space parity is only supported in POSIX with 7 or fewer data bits");
                }
                else {

                    /*space parity not directly supported - add an extra data bit to simulate it*/
                    Posix_CommConfig.c_cflag&=~(PARENB|CSIZE);
                    switch(Settings.DataBits) {
                        case DATA_5:
                            Settings.DataBits=DATA_6;
                            Posix_CommConfig.c_cflag|=CS6;
                            break;

                        case DATA_6:
                            Settings.DataBits=DATA_7;
                            Posix_CommConfig.c_cflag|=CS7;
                            break;

                        case DATA_7:
                            Settings.DataBits=DATA_8;
                            Posix_CommConfig.c_cflag|=CS8;
                            break;

                        case DATA_8:
                            break;
                    }
                }
                break;

            /*mark parity - WINDOWS ONLY*/
            case PAR_MARK:
                TTY_WARNING("Posix_QextBitBangPort: Mark parity is not supported by POSIX.");
                break;

            /*no parity*/
            case PAR_NONE:
                Posix_CommConfig.c_cflag&=(~PARENB);
                break;

            /*even parity*/
            case PAR_EVEN:
                Posix_CommConfig.c_cflag&=(~PARODD);
                Posix_CommConfig.c_cflag|=PARENB;
                break;

            /*odd parity*/
            case PAR_ODD:
                Posix_CommConfig.c_cflag|=(PARENB|PARODD);
                break;
        }
    }
    UNLOCK_MUTEX();
}

/*!
\fn void Posix_QextBitBangPort::setStopBits(StopBitsType stopBits)
Sets the number of stop bits used by the serial port.  Possible values of stopBits are:
\verbatim
    STOP_1      1 stop bit
    STOP_1_5    1.5 stop bits
    STOP_2      2 stop bits
\endverbatim
\note
This function is subject to the following restrictions:
\par
    2 stop bits cannot be used with 5 data bits.
\par
    POSIX does not support 1.5 stop bits.

*/
void Posix_QextBitBangPort::setStopBits(StopBitsType stopBits)
{
    LOCK_MUTEX();
    if (Settings.StopBits!=stopBits) {
        if ((Settings.DataBits==DATA_5 && stopBits==STOP_2) || stopBits==STOP_1_5) {}
        else {
            Settings.StopBits=stopBits;
        }
    }
    if (isOpen()) {
        switch (stopBits) {

            /*one stop bit*/
            case STOP_1:
                Settings.StopBits=stopBits;
                Posix_CommConfig.c_cflag&=(~CSTOPB);
                break;

            /*1.5 stop bits*/
            case STOP_1_5:
                TTY_WARNING("Posix_QextBitBangPort: 1.5 stop bit operation is not supported by POSIX.");
                break;

            /*two stop bits*/
            case STOP_2:
                if (Settings.DataBits==DATA_5) {
                    TTY_WARNING("Posix_QextBitBangPort: 2 stop bits cannot be used with 5 data bits");
                }
                else {
                    Settings.StopBits=stopBits;
                    Posix_CommConfig.c_cflag|=CSTOPB;
                }
                break;
        }
    }
    UNLOCK_MUTEX();
}

/*!
\fn void Posix_QextBitBangPort::setFlowControl(FlowType flow)
Sets the flow control used by the port.  Possible values of flow are:
\verbatim
    FLOW_OFF            No flow control
    FLOW_HARDWARE       Hardware (RTS/CTS) flow control
    FLOW_XONXOFF        Software (XON/XOFF) flow control
\endverbatim
\note
FLOW_HARDWARE may not be supported on all versions of UNIX.  In cases where it is
unsupported, FLOW_HARDWARE is the same as FLOW_OFF.

*/
void Posix_QextBitBangPort::setFlowControl(FlowType flow)
{
    LOCK_MUTEX();
    if (Settings.FlowControl!=flow) {
        Settings.FlowControl=flow;
    }
    if (isOpen()) {
        switch(flow) {

            /*no flow control*/
            case FLOW_OFF:
                Posix_CommConfig.c_cflag&=(~CRTSCTS);
                Posix_CommConfig.c_iflag&=(~(IXON|IXOFF|IXANY));
                break;

            /*software (XON/XOFF) flow control*/
            case FLOW_XONXOFF:
                Posix_CommConfig.c_cflag&=(~CRTSCTS);
                Posix_CommConfig.c_iflag|=(IXON|IXOFF|IXANY);
                break;

            case FLOW_HARDWARE:
                Posix_CommConfig.c_cflag|=CRTSCTS;
                Posix_CommConfig.c_iflag&=(~(IXON|IXOFF|IXANY));
                break;
        }
    }
    UNLOCK_MUTEX();
}

/*!
\fn void Posix_QextBitBangPort::setTimeout(ulong sec);
Sets the read and write timeouts for the port to millisec milliseconds.
Note that this is a per-character timeout, i.e. the port will wait this long for each
individual character, not for the whole read operation.  This timeout also applies to the
bytesWaiting() function.

\note
POSIX does not support millisecond-level control for I/O timeout values.  Any
timeout set using this function will be set to the next lowest tenth of a second for
the purposes of detecting read or write timeouts.  For example a timeout of 550 milliseconds
will be seen by the class as a timeout of 500 milliseconds for the purposes of reading and
writing the port.  However millisecond-level control is allowed by the select() system call,
so for example a 550-millisecond timeout will be seen as 550 milliseconds on POSIX systems for
the purpose of detecting available bytes in the read buffer.

*/
void Posix_QextBitBangPort::setTimeout(long millisec)
{
    LOCK_MUTEX();
    Settings.Timeout_Millisec = millisec;
    Posix_Copy_Timeout.tv_sec = millisec / 1000;
    Posix_Copy_Timeout.tv_usec = millisec % 1000;
    if (isOpen()) {
         Posix_CommConfig.c_cc[VTIME] = millisec/100;
    }
    UNLOCK_MUTEX();
}

/*!
\fn bool Posix_QextBitBangPort::open(OpenMode mode)
Opens the serial port associated to this class.
This function has no effect if the port associated with the class is already open.
The port is also configured to the current settings, as stored in the Settings structure.
*/
bool Posix_QextBitBangPort::open(OpenMode mode)
{
    int ppmode;
    int ppdir;

    LOCK_MUTEX();
    if (mode == QIODevice::NotOpen)
    {
        UNLOCK_MUTEX();
        return isOpen();
    }

    if (!isOpen())
    {
        /*open the port*/
        Posix_File->setFileName(port);
        QueueReceiveSignals = 10;
        if (Posix_File->open(QIODevice::ReadWrite | QIODevice::Unbuffered))
        {
            /*set open mode*/
            QIODevice::open(mode);

            /* configure port settings, claim the port, set mode, and pin direction */
            if (ioctl(Posix_File->handle(), PPCLAIM, NULL))
            {
                 qDebug("Could not claim parallel port " + port.toLatin1());
                 close();
                 Posix_File->close();
                 goto error_exit;
            }

            /* Set the Mode */
            ppmode = IEEE1284_MODE_BYTE;
            if (ioctl(Posix_File->handle(), PPSETMODE, &ppmode))
            {
                qDebug("Could not set mode");
                ioctl(Posix_File->handle(), PPRELEASE);
                close();
                Posix_File->close();
                goto error_exit;
             }

            /* Set data pins to output */
            ppdir = 0x00;
            if (ioctl(Posix_File->handle(), PPDATADIR, &ppdir))
            {
                qDebug("Could not set parallel port direction");
                ioctl(Posix_File->handle(), PPRELEASE);
                close();
                Posix_File->close();
                goto error_exit;
 
            }

            /* reset target device */
            pulse_cs();

#if (0)/* loop back test */
            do{
                /* loop back test */ 
                char testdata[64];
                testdata[0] = 0x0f;
                writeData(testdata, 1);
                readData(testdata, 1);

                ioctl(Posix_File->handle(), PPRELEASE);
                close();
                Posix_File->close();
            } while (0);
            goto error_exit;
#endif

            /*set up other port settings*/
            Posix_CommConfig.c_cflag|=CREAD|CLOCAL;
            Posix_CommConfig.c_lflag&=(~(ICANON|ECHO|ECHOE|ECHOK|ECHONL|ISIG));
            Posix_CommConfig.c_iflag&=(~(INPCK|IGNPAR|IGNBRK|PARMRK|ISTRIP|ICRNL|IXANY));
            Posix_CommConfig.c_oflag&=(~OPOST);
            Posix_CommConfig.c_cc[VMIN]=0;
            Posix_CommConfig.c_cc[VINTR] = _POSIX_VDISABLE;
            Posix_CommConfig.c_cc[VQUIT] = _POSIX_VDISABLE;
            Posix_CommConfig.c_cc[VSTART] = _POSIX_VDISABLE;
            Posix_CommConfig.c_cc[VSTOP] = _POSIX_VDISABLE;
            Posix_CommConfig.c_cc[VSUSP] = _POSIX_VDISABLE;
            setBaudRate(Settings.BaudRate);
            setDataBits(Settings.DataBits);
            setParity(Settings.Parity);
            setStopBits(Settings.StopBits);
            setFlowControl(Settings.FlowControl);
            //setTimeout(Settings.Timeout_Sec, Settings.Timeout_Millisec);
            setTimeout(Settings.Timeout_Millisec);

            handle = Posix_File->handle();
        }
        else
        {
            qDebug("Could not open File! Error code : %d", Posix_File->error());
        }
    }

error_exit:
    UNLOCK_MUTEX();
    return isOpen();
}

/*!
\fn void Posix_QextBitBangPort::close()
Closes a serial port.  This function has no effect if the serial port associated with the class
is not currently open.
*/
void Posix_QextBitBangPort::close()
{
    LOCK_MUTEX();

    if (isOpen())
    {
        ioctl(Posix_File->handle(), PPRELEASE);
        Posix_File->close();
        QIODevice::close();
    }
    UNLOCK_MUTEX();
}

/*!
\fn void Posix_QextBitBangPort::flush()
Flushes all pending I/O to the serial port.  This function has no effect if the serial port
associated with the class is not currently open.
*/
void Posix_QextBitBangPort::flush()
{
    LOCK_MUTEX();
    if (isOpen()) {
        Posix_File->flush();
    }
    UNLOCK_MUTEX();
}

/*!
\fn qint64 Posix_QextBitBangPort::size() const
This function will return the number of bytes waiting in the receive queue of the serial port.
It is included primarily to provide a complete QIODevice interface, and will not record errors
in the lastErr member (because it is const).  This function is also not thread-safe - in
multithreading situations, use Posix_QextBitBangPort::bytesWaiting() instead.
*/
qint64 Posix_QextBitBangPort::size() const
{
    char status;
    ioctl(Posix_File->handle(), PPRSTATUS, &status);
    if (status & PARPORT_STATUS_ACK)
        return 1;
    return 0;
}

/*!
\fn qint64 Posix_QextBitBangPort::bytesAvailable() const
Returns the number of bytes waiting in the port's receive queue.  This function will return 0 if
the port is not currently open, or -1 on error.  Error information can be retrieved by calling
Posix_QextBitBangPort::getLastError().
*/
qint64 Posix_QextBitBangPort::bytesAvailable() const
{
    qint64 result = 0;
    LOCK_MUTEX();
    result = size();
    UNLOCK_MUTEX();
    return result;
}

/*!
\fn void Posix_QextBitBangPort::ungetChar(char)
This function is included to implement the full QIODevice interface, and currently has no
purpose within this class.  This function is meaningless on an unbuffered device and currently
only prints a warning message to that effect.
*/
void Posix_QextBitBangPort::ungetChar(char)
{
    /*meaningless on unbuffered sequential device - return error and print a warning*/
    TTY_WARNING("Posix_QextBitBangPort: ungetChar() called on an unbuffered sequential device - operation is meaningless");
}

/*!
\fn void Posix_QextBitBangPort::translateError(ulong error)
Translates a system-specific error code to a QextSerialPort error code.  Used internally.
*/
void Posix_QextBitBangPort::translateError(ulong error)
{
    switch (error) {
        case EBADF:
        case ENOTTY:
            lastErr=E_INVALID_FD;
            break;

        case EINTR:
            lastErr=E_CAUGHT_NON_BLOCKED_SIGNAL;
            break;

        case ENOMEM:
            lastErr=E_NO_MEMORY;
            break;
    }
}

/*!
\fn void Posix_QextBitBangPort::setBreak(bool set)
Sets TXD line to the requested state (break/constant logic low if parameter set is true).
This function will have no effect if the port associated with the class is not currently open.
*/
void Posix_QextBitBangPort::setBreak(bool set __attribute__((unused)))
{
    return;
}

/*!
\fn void Posix_QextBitBangPort::setDtr(bool set)
Sets DTR line to the requested state (high by default).  This function will have no effect if
the port associated with the class is not currently open.
*/
void Posix_QextBitBangPort::setDtr(bool set __attribute__((unused)))
{
    return;
}

/*!
\fn void Posix_QextBitBangPort::setRts(bool set)
Sets RTS line to the requested state (high by default).  This function will have no effect if
the port associated with the class is not currently open.
*/
void Posix_QextBitBangPort::setRts(bool set __attribute__((unused)))
{
    return;
}

/*!
\fn unsigned long Posix_QextBitBangPort::lineStatus()
returns the line status as stored by the port function.  This function will retrieve the states
of the following lines: DCD, CTS, DSR, and RI.  On POSIX systems, the following additional lines
can be monitored: DTR, RTS, Secondary TXD, and Secondary RXD.  The value returned is an unsigned
long with specific bits indicating which lines are high.  The following constants should be used
to examine the states of individual lines:

\verbatim
Mask        Line
------      ----
LS_CTS      CTS
LS_DSR      DSR
LS_DCD      DCD
LS_RI       RI
LS_RTS      RTS (POSIX only)
LS_DTR      DTR (POSIX only)
LS_ST       Secondary TXD (POSIX only)
LS_SR       Secondary RXD (POSIX only)
\endverbatim

This function will return 0 if the port associated with the class is not currently open.
*/
unsigned long Posix_QextBitBangPort::lineStatus()
{
    return 0;
}

/**
 * Purges all buffered receive data from memory without requiring explicit read of said data.
 */
void Posix_QextBitBangPort::clearReceiveBuffer()
{
    return;
}

void Posix_QextBitBangPort::dprint(char data)
{
        int n;
        for (n=7; n>=0; n--){
                if (data & (1<<n))
                        printf("%c", '1');
                else
                        printf("%c", '0');
        }
        printf("\n");
}


int Posix_QextBitBangPort::set_pin(int data, int pin)
{

    data = data | (1<<pin);
    ioctl(Posix_File->handle(), PPWDATA, &data);
    return data;

}

int Posix_QextBitBangPort::clr_pin(int data, int pin)
{
    data = data & ~(1<<pin);
    ioctl(Posix_File->handle(), PPWDATA, &data);
    return data;
}

void Posix_QextBitBangPort::wait_pulse_width(void)
{
    usleep(pulse_width_us);
}

void Posix_QextBitBangPort::pulse_cs()
{
    int data = 0;
    int _10ms = 10000;

    data = clr_pin(data, CS);
    usleep(_10ms);
    
    data = set_pin(data, CS);
    usleep(_10ms);

    data = clr_pin(data, CS);
    usleep(_10ms);
}

/*!
\fn qint64 Posix_QextBitBangPort::readData(char * data, qint64 maxSize)
Reads a block of data from the serial port.  This function will read at most maxSize bytes from
the serial port and place them in the buffer pointed to by data.  Return value is the number of
bytes actually read, or -1 on error.

\warning before calling this function ensure that serial port associated with this class
is currently open (use isOpen() function to check if port is open).
*/
qint64 Posix_QextBitBangPort::readData(char * data, qint64 maxSize)
{
    if (!read_queue.isEmpty()){
	*data = read_queue.dequeue();
        return 1;
    }
    return readRawData(data, maxSize);
}

qint64 Posix_QextBitBangPort::readRawData(char * data, qint64 maxSize)
{
    int n;
    char tmp = 0;

    /* sanity check */
    if (maxSize <= 0)
	return -1;    

    /* clear bitbang value */
    tmp = 0;

    /* check for input ready */
    if (bytesAvailable() == 0){
        qDebug("WARNING: target not ready to send data.\n");
	return -1;
    }

    /* CLK = 0 */
    tmp = clr_pin(tmp, CLK);
    wait_pulse_width();

    for (n=0; n<8; n++){

            /* CLK = 1 */
            tmp = set_pin(tmp, CLK);
            wait_pulse_width();

            /* INPIN = 1,0 */
            if (size())
                    *data = *data | (1<<n);
            else
                    *data = *data & ~(1<<n);
             wait_pulse_width();

             /* CLK = 0 */
             tmp = clr_pin(tmp, CLK);
             wait_pulse_width();

    }
    tmp = clr_pin(tmp, CLK);
    wait_pulse_width();

    return 1;
}

/*!
\fn qint64 Posix_QextBitBangPort::writeData(const char * data, qint64 maxSize)
Writes a block of data to the serial port.  This function will write maxSize bytes
from the buffer pointed to by data to the serial port.  Return value is the number
of bytes actually written, or -1 on error.

\warning before calling this function ensure that serial port associated with this class
is currently open (use isOpen() function to check if port is open).
*/
qint64 Posix_QextBitBangPort::writeData(const char * data, qint64 maxSize)
{
    int n;
    int ctr;
    char tmp;
    char read_data;

    while (size() != 0){
	readRawData(&read_data, 1);
	read_queue.enqueue(read_data);
    }

    /* clear bitbang value */
    tmp = 0;
    for (ctr=0; ctr<maxSize; ctr++){

        if (size() != 0){
            lastErr = E_WRITE_FAILED;
            qDebug("### WARNING : Target not ready to receive data.\n");
            return -1;
        }

        /* write a byte */
        for (n=0; n<8; n++){

            /* CLK = 0 */
            tmp = clr_pin(tmp, CLK);
            wait_pulse_width();

            /* OUTPIN = 1,0 */
            if (*data & (1<<n))
                tmp = set_pin(tmp, OUTPIN);
            else
                tmp = clr_pin(tmp, OUTPIN);
            wait_pulse_width();

            /* CLK = 1 */
            tmp = set_pin(tmp, CLK);
            wait_pulse_width();

        }
        tmp = clr_pin(tmp, CLK);
        tmp = clr_pin(tmp, OUTPIN);
        wait_pulse_width();
        data++;
    }

    return ctr;
}
