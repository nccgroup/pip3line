/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TCPLISTENER_H
#define TCPLISTENER_H

#include <QObject>
#include <QByteArray>
#include <QAbstractSocket>
#include <QHostAddress>
#include "blockssource.h"

class QTcpSocket;

class TcpListener : public BlocksSource
{
        Q_OBJECT
    public:
#if QT_VERSION >= 0x050000
    explicit TcpListener(qintptr socketDescriptor, QObject *parent = 0);
#else
    explicit TcpListener(int socketDescriptor, QObject *parent = 0);
#endif

        explicit TcpListener(QHostAddress remoteAddress, quint16 remotePort, QObject *parent = 0);
        ~TcpListener();
        static const int BLOCK_MAX_SIZE;

    signals:
        void newBlock(QByteArray data);
        void finished();
    public slots:
        void sendBlock(const QByteArray & block);
        void startListening();
        void stopListening();
    private slots:
        void onDataReceived();
        void onSocketError(QAbstractSocket::SocketError error);
    private:
        void processBlock(QByteArray data);
        QTcpSocket *socket;
        QByteArray tempData;
        quint16 remotePort;
        QHostAddress remotePeerAddress;
#if QT_VERSION >= 0x050000
        qintptr socketDescriptor;
#else
        int socketDescriptor;
#endif
};

#endif // TCPLISTENER_H
