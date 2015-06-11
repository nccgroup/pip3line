/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef STATEORCHESTRATOR_H
#define STATEORCHESTRATOR_H

#include <QObject>
#include <QStack>
#include <QTime>
#include <commonstrings.h>

class QThread;
class BaseStateAbstract;
class QFile;
class QXmlStreamWriter;
class QXmlStreamReader;
class StateDialog;
class QWidget;

class StateOrchestrator : public QObject
{
        Q_OBJECT
    public:
        explicit StateOrchestrator(QString fileName, quint64 flags);
        ~StateOrchestrator();
        QXmlStreamWriter *getWriter() const;
        bool initialize();
        bool isSaving();

    public slots:
        bool start();
        void stop();
        void addState(BaseStateAbstract * stateobj);
        void execNext();
    signals:
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);
        void finished();
    private:
        void onFinished();
        QStack<BaseStateAbstract *> executionStack;
        QThread *localthread;
        quint64 flags;
        QString actionName;
        QTime timer;
        QString fileName;
        QFile *file;
        QXmlStreamWriter * writer;
        QXmlStreamReader *reader;
};


#endif // STATEORCHESTRATOR_H
