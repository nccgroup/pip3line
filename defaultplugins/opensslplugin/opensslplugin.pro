# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information

QT       += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = opensslplugin
TEMPLATE = lib
CONFIG += plugin release
DEFINES += OPENSSLPLUGIN_LIBRARY

SOURCES += opensslplugin.cpp \
    opensslhashes.cpp

HEADERS += opensslplugin.h\
        opensslplugin_global.h \
    opensslhashes.h

unix {
    LIBS += -L"../../bin/" -ltransform
}

win32 {
    QMAKE_LFLAGS_WINDOWS += /FS
    LIBS += -L"../../lib/" -ltransform
}

INCLUDEPATH += ../../libtransform/

DESTDIR = ../../bin/plugins

unix {
    LIBS += -lcrypto
}

win32 {
    INCLUDEPATH +="C:\\OpenSSL-Win32\\include\\"
    LIBS += -L"C:\\OpenSSL-Win32\\lib\\" -llibeay32
}

OTHER_FILES += \
    opensslplugin.json
