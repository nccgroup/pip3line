/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QObject>
#include <QThread>
#include <QIODevice>
#include <QTextStream>
#include <QList>
#include <transformmgmt.h>
#include <transformchain.h>
#include <transformabstract.h>
#include <QMutex>

class Processor : public QThread
{
        Q_OBJECT
    public:
        static const char DEFAULT_SEPARATOR;
        Processor(TransformMgmt * transformFactory, QObject * parent);
        virtual ~Processor();
        bool configureFromFile(const QString &fileName);
        bool configureFromName(const QString &name, TransformAbstract::Way way = TransformAbstract::INBOUND);
        bool setTransformsChain(TransformChain tlist); // will dispose of the list itself
        bool setTransformsChain(const QString &xmlConf);
        void setOutputMutex(QMutex * mutex);
        void setOutput(QIODevice * nout);
        void setInput(QIODevice * nin);
        void clearOutputMutex();
        void setSeparator(char c);
        long getStatsOut();
        void setEncoding(bool flag);
        void setDecoding(bool flag);
        virtual void stop() {}
    signals:
        void error(const QString, const QString);
        void status(const QString, const QString);

    protected slots:
        void logError(const QString mess, const QString id);
        void logStatus(const QString mess, const QString id);
    protected:
        Processor() {}
        virtual void writeBlock(const QByteArray &data);
        TransformChain tlist;
        QMutex * outputLock;
        QIODevice *out;
        QIODevice *in;
        char separator;
        long ocount;
    private:
        inline void clearChain();
        TransformMgmt * transformFactory;
        QMutex tranformsLock;
        QMutex statsLock;
        bool encode;
        bool decode;
};

#endif // PROCESSOR_H
