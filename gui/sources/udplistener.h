/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef UDPLISTENER_H
#define UDPLISTENER_H

#include <QHostAddress>
#include "blockssource.h"

class QUdpSocket;

class UdpListener : public BlocksSource
{
        Q_OBJECT
    public:
        explicit UdpListener(QHostAddress listeningAddress = QHostAddress::LocalHost, quint16 port = 40000 , QObject *parent = 0);
        ~UdpListener();
    public slots:
        void sendBlock(const QByteArray & block);
        void startListening();
        void stopListening();
    private slots:
        void readPendingDatagrams();
    private:
        QWidget *requestGui(QWidget * parent);
        QHostAddress listeningAddress;
        quint16 port;
        QUdpSocket *socket;
        QHostAddress lastClient;
        quint16 lastClientPort;
};

#endif // UDPLISTENER_H
