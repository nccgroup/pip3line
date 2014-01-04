# Released as open source by NCC Group Plc - http://www.nccgroup.com/
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com
#
# https://github.com/nccgroup/pip3line
#
# Released under AGPL see LICENSE for more information


QT       += core gui xml network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += debug WITH_THREAD
TARGET = pip3line
TEMPLATE = app

INCLUDEPATH +="../libtransform"

unix {
    LIBS += -L"../bin" -ltransform -lpcap
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
    infodialog.cpp \
    massprocessingdialog.cpp \
    screeniodevice.cpp \
    settingsdialog.cpp \
    downloadmanager.cpp \
    pluginconfwidget.cpp \
    guihelper.cpp \
    maintabs.cpp \
    debugdialog.cpp \
    floatingdialog.cpp \
    quickviewdialog.cpp \
    quickviewitem.cpp \
    quickviewitemconfig.cpp \
    textinputdialog.cpp \
    comparisondialog.cpp \
    newbytedialog.cpp \
    ../tools/tcpserver.cpp \
    ../tools/streamprocessor.cpp \
    ../tools/textprocessor.cpp \
    ../tools/processor.cpp \
    ../tools/binaryprocessor.cpp \
    ../tools/serverabstract.cpp \
    ../tools/pipeserver.cpp \
    ../tools/centralprocessor.cpp \
    ../tools/transformrequest.cpp \
    tabs/tababstract.cpp \
    tabs/generictab.cpp \
    tabs/transformsgui.cpp \
    tabs/randomaccesstab.cpp \
    sources/bytesourceabstract.cpp \
    sources/basicsource.cpp \
    sources/filewidget.cpp \
    sources/sourcemanager.cpp \
    sources/pcapsource.cpp \
    sources/currentmemorysource.cpp \
    sources/largerandomaccesssource.cpp \
    views/hexview.cpp \
    views/textview.cpp \
    views/singleviewabstract.cpp \
    views/bytetableview.cpp \
    views/byteitemmodel.cpp \
    shared/hexwidget.cpp \
    shared/offsetgotowidget.cpp \
    shared/searchwidget.cpp \
    shared/readonlybutton.cpp \
    shared/bytesourceguibutton.cpp \
    shared/clearallmarkingsbutton.cpp \
    appdialog.cpp \
    sources/memorywidget.cpp \
    ../tools/processingstats.cpp \
    shared/detachtabbutton.cpp \
    sources/largefile.cpp \
    sources/blockssource.cpp \
    sources/udplistener.cpp \
    sources/tcpserverlistener.cpp \
    sources/searchabstract.cpp \
    sources/tcplistener.cpp

HEADERS  += mainwindow.h \
    transformwidget.h \
    aboutdialog.h \
    messagedialog.h \
    analysedialog.h \
    regexphelpdialog.h \
    loggerwidget.h \
    infodialog.h \
    massprocessingdialog.h \
    screeniodevice.h \
    crossplatform.h \
    comparisondialog.h \
    textinputdialog.h \
    settingsdialog.h \
    downloadmanager.h \
    pluginconfwidget.h \
    guihelper.h \
    maintabs.h \
    floatingdialog.h \
    quickviewdialog.h \
    quickviewitem.h \
    quickviewitemconfig.h \
    newbytedialog.h \
    debugdialog.h \
    ../version.h \
    ../tools/textprocessor.h \
    ../tools/processor.h \
    ../tools/binaryprocessor.h \
    ../tools/tcpserver.h \
    ../tools/streamprocessor.h \
    ../tools/pipeserver.h \
    ../tools/serverabstract.h \
    ../tools/centralprocessor.h \
    ../tools/transformrequest.h \
    tabs/tababstract.h \
    tabs/generictab.h \
    tabs/randomaccesstab.h \
    tabs/transformsgui.h \
    sources/sourcemanager.h \
    sources/pcapsource.h \
    sources/currentmemorysource.h \
    sources/largerandomaccesssource.h \
    sources/filewidget.h \
    sources/bytesourceabstract.h \
    sources/basicsource.h \
    views/hexview.h \
    views/textview.h \
    views/singleviewabstract.h \
    views/bytetableview.h \
    views/byteitemmodel.h \
    shared/offsetgotowidget.h \
    shared/searchwidget.h \
    shared/readonlybutton.h \
    shared/hexwidget.h \
    shared/bytesourceguibutton.h \
    shared/clearallmarkingsbutton.h \
    appdialog.h \
    sources/memorywidget.h \
    ../tools/processingstats.h \
    shared/detachtabbutton.h \
    sources/largefile.h \
    sources/blockssource.h \
    sources/udplistener.h \
    sources/tcpserverlistener.h \
    sources/searchabstract.h \
    sources/tcplistener.h

FORMS    += mainwindow.ui \
    transformwidget.ui \
    comparisondialog.ui \
    aboutdialog.ui \
    messagedialog.ui \
    regexphelpdialog.ui \
    analysedialog.ui \
    loggerwidget.ui \
    infodialog.ui \
    massprocessingdialog.ui \
    settingsdialog.ui \
    pluginconfwidget.ui \
    textinputdialog.ui \
    floatingdialog.ui \
    quickviewdialog.ui \
    quickviewitem.ui \
    quickviewitemconfig.ui \
    newbytedialog.ui \
    debugdialog.ui \
    views/hexview.ui \
    views/textview.ui \
    tabs/generictab.ui \
    tabs/randomaccesstab.ui \
    tabs/transformsgui.ui \
    shared/hexwidget.ui \
    sources/filewidget.ui \
    sources/memorywidget.ui

OTHER_FILES += icons/pip3line.png \
    win.rc \
    release.txt

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
