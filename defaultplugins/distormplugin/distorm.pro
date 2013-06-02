# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information

QT       += svg
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = distormplugin
TEMPLATE = lib
CONFIG += plugin

DEFINES += DISTORM_LIBRARY

SOURCES += distormplugin.cpp \
    distormwidget.cpp \
    distormtransf.cpp

HEADERS += distormplugin.h\
        distorm_global.h \
    distormwidget.h \
    distormtransf.h

unix {
    LIBS += -L"../../bin/" -ltransform -L"../../../distorm/trunk/build/lib/distorm3/" -ldistorm3
}

win32 {
    LIBS += -L"../../lib/" -ltransform -L"../../../distorm/trunk/" -ldistorm
}

INCLUDEPATH += ../../libtransform/ ../../../distorm/trunk/include/
DESTDIR = ../../bin/plugins

OTHER_FILES += \
    distormplugin.json

FORMS += \
    distormwidget.ui
