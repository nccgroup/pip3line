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
#include <QObject>
#include <QTimer>


#define DEBUG_CMD "--debug"
#define FILE_CMD "--file"

#if QT_VERSION >= 0x050000

#include <stdio.h>
#include <stdlib.h>
#include <QByteArray>
#include <commonstrings.h>
#include <QDir>
#include <QDateTime>
#include <QMutex>

FILE * _logFile = NULL; // logFile

void myMessageOutput(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    QByteArray localMsg = msg.toUtf8();
    QByteArray currentTime = QDateTime::currentDateTime().toString(Qt::ISODate).toUtf8();

    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "%s Debug: %s\n",currentTime.constData(), localMsg.constData());
        if (_logFile != NULL)
            fprintf(_logFile, "%s Debug: %s\n",currentTime.constData(), localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "%s Warning: %s\n",currentTime.constData(), localMsg.constData());
        if (_logFile != NULL)
            fprintf(_logFile, "%s Warning: %s\n",currentTime.constData(), localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "%s Critical: %s\n",currentTime.constData(), localMsg.constData());
        if (_logFile != NULL)
            fprintf(_logFile, "%s Critical: %s\n",currentTime.constData(), localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s Fatal: %s\n",currentTime.constData(), localMsg.constData());
        if (_logFile != NULL) {
            fprintf(_logFile, "%s Fatal: %s\n",currentTime.constData(), localMsg.constData());
            fclose (_logFile);
            _logFile = NULL;
        }
        abort();
    }
	if (_logFile != NULL) {
		fflush(_logFile);
	}
}
#endif



#ifdef Q_OS_LINUX
#include <signal.h>

void termSignalHandler(int signal) {
    qWarning() <<"Received signal " << signal;
    QTimer::singleShot(0,QCoreApplication::instance(),SLOT(quit()));
}

static void setup_unix_signal_handlers()
{
    struct sigaction term;

    term.sa_handler = termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags |= SA_RESTART;

    if (sigaction(SIGTERM, &term, 0) == -1)
        qWarning("Could not set the SIGTERM signal handler");
    if (sigaction(SIGINT, &term, 0) == -1)
        qWarning("Could not set the SIGINT signal handler");
    if (sigaction(SIGQUIT, &term, 0) == -1)
        qWarning("Could not set the SIGQUIT signal handler");
    if (sigaction(SIGABRT, &term, 0) == -1)
        qWarning("Could not set the SIGABRT signal handler");

}
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
    setup_unix_signal_handlers();
#endif

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
        QByteArray logdir = homeDir.absolutePath().append(QDir::separator()).append("pip3line_gui.log").toLocal8Bit();
#ifdef Q_CC_MSVC
#define BUFFERSIZE 200

        char buffer[BUFFERSIZE];
        memset((void *)buffer, 0, BUFFERSIZE);

        errno_t err = fopen_s (&_logFile, logdir.constData(),"w");
        if ( err != 0) {
            strerror_s(buffer, BUFFERSIZE, err);
            qWarning( buffer );
#else
        _logFile = fopen (logdir.constData(),"w");

        if (_logFile == NULL) {
            qWarning("Cannot open log file for writing");
#endif
        } else {
            qInstallMessageHandler(myMessageOutput);
        }
    }


#endif

#ifdef Q_OS_WIN
    // forcing style on windows. can be overwritten at runtime
    // with the option -style [style name]
    // An indicative list of available themes is given in the Help->info dialog
    QStringList stylelist = QStyleFactory::keys();
    if (stylelist.contains("Fusion"))
        QApplication::setStyle("Fusion");
    else if (stylelist.contains("WindowsVista"))
        QApplication::setStyle("WindowsVista");
    else if (stylelist.contains("WindowsXP"))
        QApplication::setStyle("WindowsXP");


#endif

    // Qt initialisation
    QApplication a(argc, argv);
    a.setStyleSheet("QWidget{ selection-background-color: blue}");
	qWarning() << "App started";

    // Cleaning the PATH to avoid library corruption whenever loading plugins
    qputenv("PATH", QByteArray());

#ifdef Q_OS_UNIX
    qputenv("LD_PRELOAD", QByteArray());
#endif

    QStringList list = QApplication::arguments();

    bool debugging =  false;
    if (list.contains(DEBUG_CMD)) {
        debugging = true;
        list.removeAll(DEBUG_CMD);
    }

    QString fileName;
    if (list.contains(FILE_CMD)) {
        int index = list.indexOf(FILE_CMD);
        if (list.size() > index + 1) {
            fileName = list.at(index + 1);
            list.removeAt(index);
            list.removeAt(index);
        } else {
            qCritical() << QObject::tr("Missing file name for --file, ignoring.");
        }
    }

    QApplication::setQuitOnLastWindowClosed(false);

    int ret = 0;
    MainWindow * w = new(std::nothrow) MainWindow(debugging);
    if (w != NULL) {
        if (!fileName.isEmpty())
            w->loadFile(fileName);

        w->show();

        ret = a.exec();
        //qWarning() << "App ending";

        delete w;
    }
#if QT_VERSION >= 0x050000
    if (_logFile != NULL) {
        fclose (_logFile);
        _logFile = NULL;
    }
#endif

    return ret;
}
