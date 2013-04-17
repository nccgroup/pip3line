/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMutex>
#include <transformmgmt.h>
#include <QSemaphore>
#include "streamprocessor.h"
#include "serverabstract.h"

class TcpSocketProcessor : public StreamProcessor
{
        Q_OBJECT
        friend class TcpServer;
    public:
        ~TcpSocketProcessor();
        void run();
    signals:
        void finished(TcpSocketProcessor *);
    private :
        explicit TcpSocketProcessor(TransformMgmt * tFactory,int socketDescr, QObject * parent = 0);
        static const QString TCP_SOCKET;
        int socketDescriptor;
        QTcpSocket * socket;
};

class InternalTcpServer : public QTcpServer
{
        Q_OBJECT
        friend class TcpServer;
    signals:
        void newClient(int socketDescriptor);
    private:
        explicit InternalTcpServer(QObject *parent = 0);
        void incomingConnection(int socketDescriptor);
};

class TcpServer : public ServerAbstract
{
        Q_OBJECT
    public:
        explicit TcpServer(TransformMgmt *tFactory, QObject *parent = 0);
        ~TcpServer();
        bool startServer();
        void stopServer();
        QString getLastError();
        QString getServerType();
    public slots:
        void setIP(const QString &ip);
        void setPort(int port);
    private slots:
        void processorFinished(TcpSocketProcessor *);
        void processingNewClient(int socketDescriptor);
    private:

        QMutex confLocker;
        InternalTcpServer *tcpServer;
        QString IP;
        int port;
};

#endif // TCPSERVER_H
