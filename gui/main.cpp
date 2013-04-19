/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QApplication>
#include "mainwindow.h"
#include <QtPlugin>
#include <QDebug>
#include <QStyleFactory>

#define DEBUG_CMD "--debug"

#if QT_VERSION >= 0x050000

#include <stdio.h>
#include <stdlib.h>
#include <QByteArray>
#include <commonstrings.h>
#include <QDir>

FILE * _logFile = NULL; // logFile

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        if (_logFile != NULL)
            fprintf(_logFile, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        if (_logFile != NULL)
            fprintf(_logFile, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        if (_logFile != NULL)
            fprintf(_logFile, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        if (_logFile != NULL)
            fprintf(_logFile, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}
#endif

int main(int argc, char *argv[])
{
#if QT_VERSION >= 0x050000
    // Fixing the qt5 plugin mess
    QStringList pathlist;
    pathlist << ".";
    QCoreApplication::setLibraryPaths(pathlist);

    // Creating the QT log file

    QDir homeDir = QDir::home();
    if (!homeDir.cd(Pip3lineConst::USER_DIRECTORY)) {
        if (!homeDir.mkpath(Pip3lineConst::USER_DIRECTORY)) {
            qWarning("Cannot create user directory for log file");
        }
    }

    homeDir = QDir::home();
    if (homeDir.cd(Pip3lineConst::USER_DIRECTORY)) {
        QString logdir = homeDir.absolutePath().append(QDir::separator()).append("pip3line_gui.log");
#ifdef _MSC_VER
        char buffer[80];
        if (fopen_s (_logFile, logdir.toUtf8().data(),"w") != 0) {
            _strerror_s(buffer, 80);
            fprintf( stderr, buffer );
#else
        _logFile = fopen (logdir.toUtf8().data(),"w");

        if (_logFile == NULL) {
            qWarning("Cannot open log file for writing");
#endif
        } else {
            qInstallMessageHandler(myMessageOutput);
        }
    }


#endif

    // forcing style on windows
    QStringList stylelist = QStyleFactory::keys();

    if (stylelist.contains("WindowsVista"))
        QApplication::setStyle("WindowsVista");
    else if (stylelist.contains("WindowsXP"))
        QApplication::setStyle("WindowsXP");

    // Qt initialisation
    QApplication a(argc, argv);

    // Cleaning the PATH to avoid library loading corruption
    {
#ifdef _MSC_VER
        _putenv_s(const_cast<char *>("PATH",""));
#else
        putenv(const_cast<char *>("PATH=''"));
        putenv(const_cast<char *>("LD_PRELOAD=''"));
#endif



    }

    QStringList list = QApplication::arguments();

    bool debugging =  false;
    if (list.contains(DEBUG_CMD)) {
        debugging = true;
    }

    QApplication::setQuitOnLastWindowClosed(false);
    MainWindow w(debugging);
    w.show();

    int ret = a.exec();

#if QT_VERSION >= 0x050000
    if (_logFile != NULL) {
        fclose (_logFile);
        _logFile = NULL;
    }
#endif

    return ret;
}
