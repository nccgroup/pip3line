/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "udplistener.h"
#include <QUdpSocket>
#include <QDebug>

UdpListener::UdpListener(QHostAddress listeningAddress, quint16 port, QObject *parent) :
    BlocksSource(parent), listeningAddress(listeningAddress), port(port)
{
    socket = NULL;
    readWrite = true;
}

UdpListener::~UdpListener()
{
    stopListening();
}

void UdpListener::sendBlock(const QByteArray &block)
{
    if (socket != NULL) {
        socket->writeDatagram(block,lastClient,lastClientPort);
    } else {
        qCritical() << "No UDP socket present, dropping the block.";
    }
}

void UdpListener::startListening()
{
    //start();
    if (socket == NULL) {
        socket = new(std::nothrow) QUdpSocket(this);
        if (socket->bind(listeningAddress, port)) {
            connect(socket, SIGNAL(readyRead()), SLOT(readPendingDatagrams()));
            qWarning() << "UDP server started" << listeningAddress.toString() << ":" << port;
        } else {
            qCritical() << socket->errorString();
        }
    } else if (socket->isOpen()){
        qCritical() << "UDP socket already listening";
    } else {
        qCritical() << "UDP socket exist but not doing anything T_T";

    }
}

void UdpListener::stopListening()
{
    if (socket != NULL) {
        socket->close();
        delete socket;
    }
}

void UdpListener::readPendingDatagrams()
{
    while (socket->hasPendingDatagrams()) {
             QByteArray datagram;
             datagram.resize(socket->pendingDatagramSize());

             socket->readDatagram(datagram.data(), datagram.size(),
                                     &lastClient, &lastClientPort);

             qDebug() << "UDP datagram received" << lastClient.toString() << ":" << lastClientPort;
             datagram = QByteArray::fromBase64(datagram); // decoding
             emit blockReceived(datagram);
    }
}

QWidget *UdpListener::requestGui(QWidget */*parent*/)
{
    return NULL;
}
