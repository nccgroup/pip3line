# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information

QT       += svg gui

TARGET = exampleplugin
TEMPLATE = lib
CONFIG += plugin release

unix {
    LIBS += -L"../../bin/" -ltransform
}

win32 {
    QMAKE_LFLAGS_WINDOWS += /FS
    LIBS += -L"../../lib/" -ltransform
}

INCLUDEPATH += ../../libtransform/
DESTDIR = ../../bin/plugins

DEFINES += EXAMPLEPLUGIN_LIBRARY

SOURCES += exampleplugin.cpp \
        identity.cpp

HEADERS += exampleplugin.h\
        exampleplugin_global.h \
        identity.h

OTHER_FILES += \
    exampleplugin.json

