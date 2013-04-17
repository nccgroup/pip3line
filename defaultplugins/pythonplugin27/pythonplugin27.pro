# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information

QT       += svg  gui

TARGET = python27plugin
TEMPLATE = lib
CONFIG += plugin release

LIBS += -L"../../bin/" -ltransform
INCLUDEPATH += ../../libtransform/
DESTDIR = ../../bin/plugins

DEFINES += PYTHONPLUGIN27_LIBRARY

SOURCES += pythonplugin27.cpp \
    python27.cpp \

HEADERS += pythonplugin27.h\
        pythonplugin27_global.h \
    python27.h \
    ../../version.h \

FORMS +=

unix {
    LIBS += -lpython2.7
}

win32 {
    INCLUDEPATH +="C:\\Python27\\include\\"
    LIBS += -L"C:\\Python27\\libs\\" -lpython27
}

OTHER_FILES += \
    python27plugin.json
