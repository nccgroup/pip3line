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
#include <QDateTime>
#include <QMutex>

FILE * _logFile = NULL; // logFile
QMutex logFileMutex;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QString currentTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    switch (type) {
    case QtDebugMsg:
        logFileMutex.lock();
        fprintf(stderr, "%s Debug: %s (%s:%u, %s)\n",currentTime.toUtf8().constData(), localMsg.constData(), context.file, context.line, context.function);
        if (_logFile != NULL)
            fprintf(_logFile, "%s Debug: %s (%s:%u, %s)\n",currentTime.toUtf8().constData(), localMsg.constData(), context.file, context.line, context.function);
        logFileMutex.unlock();
        break;
    case QtWarningMsg:
        logFileMutex.lock();
        fprintf(stderr, "%s Warning: %s (%s:%u, %s)\n",currentTime.toUtf8().constData(), localMsg.constData(), context.file, context.line, context.function);
        if (_logFile != NULL)
            fprintf(_logFile, "%s Warning: %s (%s:%u, %s)\n",currentTime.toUtf8().constData(), localMsg.constData(), context.file, context.line, context.function);
        logFileMutex.unlock();
        break;
    case QtCriticalMsg:
        logFileMutex.lock();
        fprintf(stderr, "%s Critical: %s (%s:%u, %s)\n",currentTime.toUtf8().constData(), localMsg.constData(), context.file, context.line, context.function);
        if (_logFile != NULL)
            fprintf(_logFile, "%s Critical: %s (%s:%u, %s)\n",currentTime.toUtf8().constData(), localMsg.constData(), context.file, context.line, context.function);
        logFileMutex.unlock();
        break;
    case QtFatalMsg:
        logFileMutex.lock();
        fprintf(stderr, "%s Fatal: %s (%s:%u, %s)\n",currentTime.toUtf8().constData(), localMsg.constData(), context.file, context.line, context.function);
        if (_logFile != NULL) {
            fprintf(_logFile, "%s Fatal: %s (%s:%u, %s)\n",currentTime.toUtf8().constData(), localMsg.constData(), context.file, context.line, context.function);
            fclose (_logFile);
            _logFile = NULL;
        }
        logFileMutex.unlock();
        abort();
    }
}
#endif

int main(int argc, char *argv[])
{

#if QT_VERSION >= 0x050000
    QStringList pathlist = QCoreApplication::libraryPaths();
    qDebug() << pathlist;
    // Fixing the qt5 plugin mess
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
#define BUFFERSIZE 200

        char buffer[BUFFERSIZE];
        memset((void *)buffer, 0, BUFFERSIZE);

        errno_t err = fopen_s (&_logFile, logdir.toUtf8().data(),"w");
        if ( err != 0) {
            strerror_s(buffer, BUFFERSIZE, err);
            qWarning( buffer );
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

    // forcing style on windows. can be overwritten at runtime
    // with the option --style
    QStringList stylelist = QStyleFactory::keys();

    if (stylelist.contains("WindowsVista"))
        QApplication::setStyle("WindowsVista");
    else if (stylelist.contains("WindowsXP"))
        QApplication::setStyle("WindowsXP");

    // Qt initialisation
    QApplication a(argc, argv);

    // Cleaning the PATH to avoid library loading corruption
    qputenv("PATH", QByteArray());

#ifndef _MSC_VER
    qputenv("LD_PRELOAD", QByteArray());
#endif

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
    logFileMutex.lock();
    if (_logFile != NULL) {
        fclose (_logFile);
        _logFile = NULL;
    }
    logFileMutex.unlock();
#endif

    return ret;
}
