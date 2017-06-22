/************************************************************************
* Copyright (c) 2009-2010,  Microchip Technology Inc.
*
* Microchip licenses this software to you solely for use with Microchip
* products.  The software is owned by Microchip and its licensors, and
* is protected under applicable copyright laws.  All rights reserved.
*
* SOFTWARE IS PROVIDED "AS IS."  MICROCHIP EXPRESSLY DISCLAIMS ANY
* WARRANTY OF ANY KIND, WHETHER EXPRESS OR IMPLIED, INCLUDING BUT
* NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.  IN NO EVENT SHALL
* MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
* CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, HARM TO YOUR
* EQUIPMENT, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY
* OR SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED
* TO ANY DEFENSE THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION,
* OR OTHER SIMILAR COSTS.
*
* To the fullest extent allowed by law, Microchip and its licensors
* liability shall not exceed the amount of fees, if any, that you
* have paid directly to Microchip to use this software.
*
* MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE
* OF THESE TERMS.
*/

#ifndef QSERIALTERMINAL_H
#define QSERIALTERMINAL_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QTime>
#include <QBasicTimer>
#ifdef USE_SERIAL
#include "QextSerialPort/qextserialport.h"
#else
#include "QextSerialPort/qextbitbangport.h"
#endif

/*!
 * This thread provides non-blocking serial transmit capability for the serial terminal GUI.
 */
class TransmitThread : public QThread
{
    Q_OBJECT

public:
    bool shutdown;
#ifdef USE_SERIAL
    QextSerialPort* serial;
#else
    QextBitBangPort* serial;
#endif

    TransmitThread();

    void enqueue(char data);
    void enqueue(char* data, int length);
    void run();

protected:
    QByteArray transmit[2];
    int threadBuffer;
    QMutex mutex;
};

/*!
 * Provides a simple serial terminal GUI by extending the QPlainTextEdit widget.
 */
class QSerialTerminal : public QPlainTextEdit
{
    Q_OBJECT

public:
    QSerialTerminal(QWidget *parent = 0);
    ~QSerialTerminal();

#ifdef USE_SERIAL
    void open(QextSerialPort* newSerialPort);
#else
    void open(QextBitBangPort* newSerialPort);
#endif
    void close(void);
    void transmitFile(QString fileName);

#ifdef USE_SERIAL
    QextSerialPort* serialPort(void);
#else
    QextBitBangPort* serialPort(void);
#endif
    bool isConnected(void);

    unsigned int TotalRx, TotalTx;
    double RxPerSecond;

signals:
    void RefreshStatus();

public slots:
    void serialReceive();
    void clear();

protected:
#ifdef USE_SERIAL
    QextSerialPort* serial;
#else
    QextBitBangPort* serial;
#endif
    QTime lastReceive;
    TransmitThread* transmitThread;
    int originalTimeout;

    QTime rxTime;
    int runningAverageDivisor;

    QTextCursor cursor;
    unsigned int editorCharacterCount;

    QBasicTimer timer;
    bool dirty;

    void flushText(QString& text, QTextCursor& cursor);

    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void timerEvent(QTimerEvent* e);
};

#endif // QSERIALTERMINAL_H
