# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information


QT       += core gui xml network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = pip3line
TEMPLATE = app

INCLUDEPATH +="../libtransform"

unix {
    LIBS += -L"../bin" -ltransform
}

win32 {
    LIBS += -L"../lib" -ltransform
}

DESTDIR = ../bin

SOURCES += main.cpp\
        mainwindow.cpp \
    transformwidget.cpp \
    messagedialog.cpp \
    aboutdialog.cpp \
    analysedialog.cpp \
    regexphelpdialog.cpp \
    loggerwidget.cpp \
    customdialogdockwidget.cpp \
    ../tools/textprocessor.cpp \
    ../tools/processor.cpp \
    ../tools/binaryprocessor.cpp \
    views/bytetableview.cpp \
    views/byteitemmodel.cpp \
    infodialog.cpp \
    massprocessingdialog.cpp \
    ../tools/tcpserver.cpp \
    ../tools/streamprocessor.cpp \
    screeniodevice.cpp \
    ../tools/pipeserver.cpp \
    settingsdialog.cpp \
    downloadmanager.cpp \
    pluginconfwidget.cpp \
    transformsgui.cpp \
    guihelper.cpp \
    maintabs.cpp \
    ../tools/serverabstract.cpp \
    floatingdialog.cpp \
    quickviewdialog.cpp \
    quickviewitem.cpp \
    quickviewitemconfig.cpp \
    hexwidget.cpp \
    newbytedialog.cpp \
    debugdialog.cpp \
    ../tools/centralprocessor.cpp \
    ../tools/transformrequest.cpp \
    sources/bytesourceabstract.cpp \
    sources/basicsource.cpp \
    views/hexview.cpp \
    textinputdialog.cpp \
    views/textview.cpp \
    sources/filesource.cpp \
    views/doubleviewwidget.cpp

HEADERS  += mainwindow.h \
    transformwidget.h \
    aboutdialog.h \
    messagedialog.h \
    analysedialog.h \
    regexphelpdialog.h \
    loggerwidget.h \
    customdialogdockwidget.h \
    ../tools/textprocessor.h \
    ../tools/processor.h \
    ../tools/binaryprocessor.h \
    views/bytetableview.h \
    views/byteitemmodel.h \
    infodialog.h \
    massprocessingdialog.h \
    ../tools/tcpserver.h \
    ../tools/streamprocessor.h \
    screeniodevice.h \
    ../tools/pipeserver.h \
    ../version.h \
    settingsdialog.h \
    downloadmanager.h \
    pluginconfwidget.h \
    transformsgui.h \
    guihelper.h \
    maintabs.h \
    ../tools/serverabstract.h \
    floatingdialog.h \
    quickviewdialog.h \
    quickviewitem.h \
    quickviewitemconfig.h \
    hexwidget.h \
    newbytedialog.h \
    debugdialog.h \
    ../tools/centralprocessor.h \
    ../tools/transformrequest.h \
    sources/bytesourceabstract.h \
    sources/basicsource.h \
    views/hexview.h \
    textinputdialog.h \
    views/textview.h \
    sources/filesource.h \
    crossplatform.h \
    views/doubleviewwidget.h

FORMS    += mainwindow.ui \
    transformwidget.ui \
    aboutdialog.ui \
    messagedialog.ui \
    regexphelpdialog.ui \
    analysedialog.ui \
    loggerwidget.ui \
    customdialogdockwidget.ui \
    infodialog.ui \
    massprocessingdialog.ui \
    settingsdialog.ui \
    pluginconfwidget.ui \
    transformsgui.ui \
    floatingdialog.ui \
    quickviewdialog.ui \
    quickviewitem.ui \
    quickviewitemconfig.ui \
    hexwidget.ui \
    newbytedialog.ui \
    debugdialog.ui \
    views/hexview.ui \
    textinputdialog.ui \
    views/textview.ui \
    views/doubleviewwidget.ui

OTHER_FILES += icons/pip3line.png \
    win.rc

RESOURCES += \
    gui_res.qrc

unix {
    ROOT_PATH = /usr/local
    target.path = $$ROOT_PATH/bin/
    desktop.path = /usr/share/applications
    desktop.files += pip3line.desktop
    icon.path = /usr/share/icons/hicolor/128x128/apps
    icon.files += icons/pip3line.png
    INSTALLS += target desktop icon
}

win32 {
    RC_FILE = win.rc
}

# DEFINES +=
