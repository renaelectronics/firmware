# -------------------------------------------------
# Project created by QtCreator 2010-08-02T17:44:13
# -------------------------------------------------
QT -= gui
TARGET = AN1310cl
CONFIG += console
CONFIG += static
QMAKE_LFLAGS += -static
CONFIG -= app_bundle
TEMPLATE = app
QMAKE_CXXFLAGS_RELEASE = -Os
INCLUDEPATH += ../
SOURCES += main.cpp \
    Bootload.cpp
unix {
    DEFINES += _TTY_POSIX_
    #LIBS += -L../build-QextSerialPort-Desktop_Qt_5_9_0_GCC_64bit-Debug
    #LIBS += -L../build-Bootload-Desktop_Qt_5_9_0_GCC_64bit-Debug
    #LIBS += -L../build-QextSerialPort-Desktop_Qt_5_5_1_GCC_32bit-Debug
    #LIBS += -L../build-Bootload-Desktop_Qt_5_5_1_GCC_32bit-Debug
    LIBS += -L../QextSerialPort
    LIBS += -L../Bootload
    LIBS += -L/home/thomastai/Qt/5.5/Src/qtbase/lib
}

win32 { 
    DEFINES += _TTY_WIN_
    LIBS += -lsetupapi
    CONFIG(debug) {
        LIBS += -L../QextSerialPort/debug
        LIBS += -L../Bootload/debug
    }
    CONFIG(release) {
        LIBS += -L../QextSerialPort/release
        LIBS += -L../Bootload/release
    }
}
LIBS += -lBootload \
    -lQextSerialPort 
HEADERS += Bootload.h
