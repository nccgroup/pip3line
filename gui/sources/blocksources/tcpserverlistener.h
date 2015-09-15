/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/


#ifndef TCPSERVERLISTENER_H
#define TCPSERVERLISTENER_H

#include "blockssource.h"
#include <QObject>
#include <QTcpServer>

class TcpListener;
class QThread;

class InTcpServer : public QTcpServer
{
        Q_OBJECT
    public:
        explicit InTcpServer(QObject *parent = 0);
        ~InTcpServer();
#if QT_VERSION >= 0x050000
    signals:
        void newClient(qintptr socketDescriptor);
    private slots:
        void incomingConnection(qintptr socketDescriptor);
#else
    signals:
        void newClient(int socketDescriptor);
    private slots:
        void incomingConnection(int socketDescriptor);
#endif
};

class TcpServerListener : public BlocksSource
{
        Q_OBJECT
    public:
        static const QString ID;
        explicit TcpServerListener(QObject *parent = 0);
        ~TcpServerListener();
        void sendBlock(Block * block);
        bool startListening();
        void stopListening();
        void postBlockForSending(Block *block);
        QString getName();
        bool isReflexive();
    public slots:
        void setListeningAddress(const QHostAddress &value);
        void setPort(const quint16 &value);
    signals:
        void blockToClient(Block *block);
        void shutdownAllClient();
    private slots:
        void clientFinished();
        void onClientReceivedBlock(Block *block);
#if QT_VERSION >= 0x050000
        void handlingClient(qintptr socketDescriptor);
#else
        void handlingClient(int socketDescriptor);
#endif
    private:
        static const quint16 DEFAULT_PORT;
        static const QHostAddress DEFAULT_ADDRESS;
        void startingWorkers();
        void stoppingWorkers();
        QWidget *requestGui(QWidget * parent);
        QHostAddress listeningAddress;
        quint16 port;
        QThread *workerThread;
        QThread *serverThread;
        QHash<int,TcpListener *> clientsID;
        QHash<TcpListener *, int> clientsList;
        InTcpServer *server;
};

#endif // TCPSERVERLISTENER_H
