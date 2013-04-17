# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information

QT      += core gui

VERSION  = 1.2

TARGET   = pip3linecmd
CONFIG  += console release
CONFIG  -= app_bundle
DESTDIR  = ../bin
TEMPLATE = app

INCLUDEPATH += "../libtransform"
LIBS += -L"../bin" -ltransform

SOURCES += main.cpp \
    ../tools/processor.cpp \
    masterthread.cpp \
    ../tools/binaryprocessor.cpp \
    ../tools/textprocessor.cpp

HEADERS += \
    ../tools/processor.h \
    masterthread.h \
    ../tools/binaryprocessor.h \
    ../tools/textprocessor.h


unix {
    ROOT_PATH = /usr/local
    target.path = $$ROOT_PATH/bin/
    INSTALLS += target
}

win32 {

}
