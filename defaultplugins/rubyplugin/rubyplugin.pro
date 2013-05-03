# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information

QT       += svg

TARGET = rubyplugin
TEMPLATE = lib
CONFIG += plugin release

LIBS += -L"../../bin/" -ltransform
INCLUDEPATH += ../../libtransform/
INCLUDEPATH += /usr/include/ruby-1.9.1/x86_64-linux/
INCLUDEPATH += /usr/include/ruby-1.9.1/

DESTDIR = ../../bin/plugins

DEFINES += RUBYPLUGIN_LIBRARY

SOURCES += rubyplugin.cpp

HEADERS += rubyplugin.h\
        rubyplugin_global.h

unix {
    LIBS += -lruby19
}

OTHER_FILES += \
    rubyplugin.json
