#ifndef QEXTBITBANGPORT_H
#define QEXTBITBANGPORT_H

/*POSIX CODE*/
#ifdef _TTY_POSIX_
#include "posix_qextbitbangport.h"
#define QextBaseType Posix_QextBitBangPort
/*MS WINDOWS CODE*/
#else
#error MS Windows Not Supported
#endif

/*!
 * Provides low level COM port serial communications.
 */
class QextBitBangPort: public QextBaseType
{
    Q_OBJECT

    public:
        QextBitBangPort();
        QextBitBangPort(const QString & name);
        QextBitBangPort(PortSettings const& s);
        QextBitBangPort(const QString & name, PortSettings const& s);
        QextBitBangPort(const QextBitBangPort& s);
        QextBitBangPort& operator=(const QextBitBangPort&);
        virtual ~QextBitBangPort();

};

#endif // QEXTBITBANGPORT_H
