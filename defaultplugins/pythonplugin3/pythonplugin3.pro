# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information

QT       += svg gui

TARGET = pythonplugin3
TEMPLATE = lib
CONFIG += plugin release no_keywords

LIBS += -L"../../bin/" -ltransform
INCLUDEPATH += ../../libtransform/
DESTDIR = ../../bin/plugins

DEFINES += PYTHONPLUGIN3_LIBRARY

SOURCES += pythonplugin3.cpp \
    python3.cpp

HEADERS += pythonplugin3.h\
        pythonplugin3_global.h \
    python3.h


unix {
    LIBS += -lpython3.2
}

win32 {
    INCLUDEPATH +="C:\\Python33\\include\\"
    LIBS += -L"C:\\Python33\\libs\\" -lpython33
}

OTHER_FILES += \
    pythonplugin3.json

