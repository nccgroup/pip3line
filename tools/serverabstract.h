/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SERVERABSTRACT_H
#define SERVERABSTRACT_H

#include <QMutex>
#include <QObject>
#include <transformmgmt.h>
#include "processor.h"

class ServerAbstract : public QObject
{
        Q_OBJECT
    public:
        ServerAbstract(TransformMgmt *tFactory);
        virtual ~ServerAbstract() {}
        virtual void setOutput(QIODevice *out);
        void clearOutput();
        long getStatsOut();

        virtual void stopServer();
        virtual bool startServer() = 0;
        virtual QString getLastError() = 0;
        virtual QString getServerType() = 0;
    public slots:
        void setEncoding(bool flag);
        void setDecoding(bool flag);
        void setTransformations(const QString &conf);
        void setSeparator(char c);
    signals:
        void error(const QString message, const QString src);
        void status(const QString message, const QString src);
    protected slots:
        virtual void logError(const QString mess, const QString id);
        virtual void logStatus(const QString mess, const QString id);
    protected:
        QIODevice * output;
        QMutex confLocker;
        TransformMgmt * transformFactory;
        QString tconf;
        char separator;
        bool encode;
        bool decode;
        QList<Processor *> clientProcessor;
};

#endif // SERVERABSTRACT_H
