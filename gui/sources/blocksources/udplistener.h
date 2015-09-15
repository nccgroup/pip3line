/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef UDPLISTENER_H
#define UDPLISTENER_H

#include <QHostAddress>
#include <QHash>
#include "blockssource.h"

class QUdpSocket;
class QThread;

class UDPClient {
    public:
        explicit UDPClient(QHostAddress clientAddress, quint16 clientPort);
        UDPClient(const UDPClient &other); // copy constructor
        QHostAddress getAdress() const;
        void setAdress(const QHostAddress &value);
        quint16 getPort() const;
        void setPort(const quint16 &value);
        bool operator==(const UDPClient& other) const;
        UDPClient& operator=(const UDPClient& other);

    private:
        QHostAddress adress;
        quint16 port;
};

class UdpListener : public BlocksSource
{
        Q_OBJECT
    public:
        static const QString ID;
        explicit UdpListener(QObject *parent = 0);
        explicit UdpListener(QHostAddress listeningAddress, quint16 port , QObject *parent = 0);
        ~UdpListener();
    public slots:
        void sendBlock(Block *block);
        void setPort(const quint16 &value);
        void setListeningAddress(const QHostAddress &value);
        bool startListening();
        void stopListening();
        QString getName();
        bool isReflexive();
    private slots:
        void readPendingDatagrams();
    private:
        static const quint16 DEFAULT_PORT;
        static const QHostAddress DEFAULT_ADDRESS;
        void initialize(QHostAddress listeningAddress = DEFAULT_ADDRESS, quint16 port = DEFAULT_PORT);
        QWidget *requestGui(QWidget * parent);
        QHostAddress listeningAddress;
        quint16 port;
        QUdpSocket *currentSocket;
        QThread *serverThread;
        QHash<int, UDPClient *> clients;
};

#endif // UDPLISTENER_H
