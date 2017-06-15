#ifndef POSIX_QEXTBITBANGPORT_H
#define POSIX_QEXTBITBANGPORT_H

#include <QThread>
#include <QWaitCondition>

#include <stdio.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#include "qextserialbase.h"

/*
 * set and clr pin
 */
#define CLK     (0)
#define OUTPIN  (1)
#define CS      (2)

/*!
 * This class encapsulates the Posix (Linux) portion of QextBitBangPort.
 */
class Posix_QextBitBangPort : public QextSerialBase
{
    private:
    /*!
     * This method is a part of constructor.
     */
    void init();

protected:
    QFile* Posix_File;
    int handle;

    struct termios Posix_CommConfig;
    struct timeval Posix_Timeout;
    struct timeval Posix_Copy_Timeout;

    virtual qint64 readData(char * data, qint64 maxSize);
    virtual qint64 writeData(const char * data, qint64 maxSize);

public:
    Posix_QextBitBangPort();
    Posix_QextBitBangPort(const Posix_QextBitBangPort& s);
    Posix_QextBitBangPort(const QString & name);
    Posix_QextBitBangPort(const PortSettings& settings);
    Posix_QextBitBangPort(const QString & name, const PortSettings& settings);
    Posix_QextBitBangPort& operator=(const Posix_QextBitBangPort& s);
    virtual ~Posix_QextBitBangPort();

    virtual void setBaudRate(unsigned int);
    virtual void setDataBits(DataBitsType);
    virtual void setParity(ParityType);
    virtual void setStopBits(StopBitsType);
    virtual void setFlowControl(FlowType);
    virtual void setTimeout(long);

    virtual bool open(OpenMode mode);
    virtual void close();
    virtual void flush();
    virtual void clearReceiveBuffer();

    virtual qint64 size() const;
    virtual qint64 bytesAvailable() const;

    virtual void ungetChar(char c);

    virtual void translateError(ulong error);

    virtual void setBreak(bool set=true);
    virtual void setDtr(bool set=true);
    virtual void setRts(bool set=true);
    virtual ulong lineStatus();

    /* bitbanging operations */
    void dprint(char data);
    int set_pin(int data, int pin);
    int clr_pin(int data, int pin);
    void pulse_cs(void);

};

#endif // POSIX_QEXTBITBANGPORT_H
