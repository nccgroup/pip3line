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
        explicit TcpServerListener(QObject *parent = 0);
        ~TcpServerListener();
        void sendBlock(const QByteArray & block);
        void startListening();
        void stopListening();
        QHostAddress getListeningAddress() const;
        void setListeningAddress(const QHostAddress &value);
    signals:
        void blockReceived(QByteArray data);
        void broadcastBlock(QByteArray data);
    private slots:
        void clientFinished();
#if QT_VERSION >= 0x050000
        void handlingClient(qintptr socketDescriptor);
#else
        void handlingClient(int socketDescriptor);
#endif

    private:
        void startingWorkers();
        void stoppingWorkers();
        QHostAddress listeningAddress;
        quint16 port;
        QThread *workerThread;
        QThread *serverThread;
        QList<TcpListener *> clients;
        InTcpServer *server;
};

#endif // TCPSERVERLISTENER_H
