# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information


QT       += svg

TARGET = transform
TEMPLATE = lib
CONFIG += release plugin
DEFINES += LIBTRANSFORM_LIBRARY

unix {
DESTDIR = ../bin
}

win32 {
DESTDIR = ../lib
}

SOURCES += transformmgmt.cpp \
    transformabstract.cpp \
    pip3linecallback.cpp \
    transformchain.cpp \
    composedtransform.cpp \
    modulesmanagementwidget.cpp \
    modulesmanagement.cpp \
    moduletransformwidget.cpp \
    scripttransformabstract.cpp \
    deleteablelistitem.cpp

HEADERS += \
    transformmgmt.h \
    transformabstract.h \
    transformfactoryplugininterface.h \
    commonstrings.h \
    libtransform_global.h \
    pip3linecallback.h \
    ../version.h \
    transformchain.h \
    composedtransform.h \
    modulesmanagementwidget.h \
    modulesmanagement.h \
    moduletransformwidget.h \
    scripttransformabstract.h \
    deleteablelistitem.h

FORMS += \
    modulesmanagementwidget.ui \
    moduletransformwidget.ui \
    deleteablelistitem.ui

