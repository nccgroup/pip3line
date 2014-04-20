# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information

QT       += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = lib
CONFIG += plugin debug no_keywords

# Comment this out if you want to compile the 2.7 version
#CONFIG += CONF_PYTHON_3


CONF_PYTHON_3 {
    DEFINES += BUILD_PYTHON_3
    TARGET = python3plugin
    unix {
        LIBS += -lpython3.3
        INCLUDEPATH +="/usr/include/python3.3/"
    }

    win32 {
        INCLUDEPATH +="C:\\Python33\\include\\"
        LIBS += -L"C:\\Python33\\libs\\" -lpython33
    }
} else {
    TARGET = python27plugin
    unix {
        LIBS += -lpython2.7
        INCLUDEPATH ="/usr/include/python2.7/" INCLUDEPATH
    }

    win32 {
        INCLUDEPATH +="C:\\Python27\\include\\"
        LIBS += -L"C:\\Python27\\libs\\" -lpython27
    }
}

unix {
    LIBS += -L"../../bin/" -ltransform
}

win32 {
    LIBS += -L"../../lib/" -ltransform
}

INCLUDEPATH += ../../libtransform/
DESTDIR = ../../bin/plugins

DEFINES += PYTHONPLUGIN_LIBRARY
DEFINES += CONF_PYTHON_3

SOURCES += pythonplugin.cpp \
    pythontransform.cpp \
    pythonmodules.cpp

HEADERS += pythonplugin.h\
        pythonplugin_global.h \
    pythontransform.h \
    pythonmodules.h

OTHER_FILES += \
    pythonplugin.json

