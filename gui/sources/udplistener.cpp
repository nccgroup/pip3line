/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "udplistener.h"
#include "networkconfwidget.h"
#include <QUdpSocket>
#include <QThread>
#include <QDebug>

const quint16 UdpListener::DEFAULT_PORT = 40000;
const QHostAddress UdpListener::DEFAULT_ADDRESS = QHostAddress::LocalHost;

UdpListener::UdpListener(QObject *parent) : BlocksSource(parent)
{
    initialize();
}

UdpListener::UdpListener(QHostAddress listeningAddress, quint16 port, QObject *parent) :
    BlocksSource(parent)
{
    initialize(listeningAddress, port);
}

UdpListener::~UdpListener()
{
    serverThread->quit();
    serverThread->wait();
    stopListening();
}

void UdpListener::initialize(QHostAddress nlisteningAddress, quint16 nport)
{
    currentSocket = NULL;
    readWrite = true;
    listeningAddress = nlisteningAddress;
    port = nport;
    serverThread = new(std::nothrow) QThread();
    if (serverThread == NULL) {
        qFatal("Cannot allocate memory for serverThread X{");
    }

    moveToThread(serverThread);
    serverThread->start();
}

void UdpListener::sendBlock(const Block & block)
{
    QByteArray data;
    if (currentSocket != NULL) {
        if (block.sourceid < 0 || block.sourceid > clients.size())
            qCritical() << tr("[UdpListener::sendBlock] Invalid sourceid: %1").arg(block.sourceid);
        else {
            UDPClient client = clients.at(block.sourceid);
            if (encodeOutput)
                data = block.data.toBase64();
            else
                data = block.data;
            currentSocket->writeDatagram(data ,client.getAdress(),client.getPort());
        }
    } else {
        qCritical() << "[UdpListener::sendBlock] No UDP socket present, dropping the block.";
    }
}

bool UdpListener::startListening()
{
    if (currentSocket != NULL) {
        stopListening();
    }

    // recreate server in all cases
    currentSocket = new(std::nothrow) QUdpSocket(this);
    if (currentSocket == NULL) {
        qFatal("Cannot allocate memory for QUdpSocket X{");
    }
    if (currentSocket->bind(listeningAddress, port)) {
        connect(currentSocket, SIGNAL(readyRead()), SLOT(readPendingDatagrams()));
        emit log(tr("UDP server started %1:%2").arg(listeningAddress.toString()).arg(port), "", Pip3lineConst::LSTATUS);
        emit started();
    } else {
        emit log(currentSocket->errorString(),metaObject()->className(),Pip3lineConst::LERROR);
        delete currentSocket;
        currentSocket = NULL;
        return false;
    }
    return true;
}

void UdpListener::stopListening()
{
    if (currentSocket != NULL) {
        currentSocket->close();
        delete currentSocket;
        currentSocket = NULL;
        emit log(tr("UDP server stopped %1:%2").arg(listeningAddress.toString()).arg(port), "", Pip3lineConst::LSTATUS);
    }
    emit stopped();
}

void UdpListener::readPendingDatagrams()
{
    while (currentSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        QHostAddress currentClient;
        quint16 currentClientPort;
        datagram.resize(currentSocket->pendingDatagramSize());

        currentSocket->readDatagram(datagram.data(), datagram.size(),
                             &currentClient, &currentClientPort);

        if (decodeInput)
            datagram = QByteArray::fromBase64(datagram);

        qDebug() << "UDP datagram received" << currentClient.toString() << ":" << currentClientPort << datagram.toHex();
        Block datab;
        datab.data = datagram;
        datab.source = this;
        datab.sourceid = clients.size();
        datab.direction = Block::SOURCE;
        UDPClient client(currentClient, currentClientPort);
        clients.append(client);
        emit blockReceived(datab);
    }
}

QWidget *UdpListener::requestGui(QWidget * parent)
{
    NetworkConfWidget *ncw = new(std::nothrow)NetworkConfWidget(NetworkConfWidget::UDP_SERVER,parent);
    if (ncw == NULL) {
        qFatal("Cannot allocate memory for NetworkConfWidget X{");
    }
    ncw->setPort(port);
    ncw->setIP(listeningAddress);
    ncw->enableDecodeEncodeOption(true);
    connect(ncw, SIGNAL(newIp(QHostAddress)), this, SLOT(setListeningAddress(QHostAddress)));
    connect(ncw, SIGNAL(newPort(quint16)), this, SLOT(setPort(quint16)));
    connect(ncw, SIGNAL(start()), this, SLOT(startListening()));
    connect(ncw, SIGNAL(stop()), this, SLOT(stopListening()));
    connect(ncw,SIGNAL(restart()), this, SLOT(restart()));
    connect(this, SIGNAL(started()), ncw, SLOT(onServerStarted()));
    connect(this, SIGNAL(stopped()), ncw, SLOT(onServerStopped()));
    return ncw;
}

void UdpListener::setPort(const quint16 &value)
{
    if (value != port) {
        port = value;
    }
}

void UdpListener::setListeningAddress(const QHostAddress &value)
{
    if (value != listeningAddress) {
        listeningAddress = value;
    }
}


UDPClient::UDPClient(const UDPClient &other)
{
    adress = other.adress;
    port = other.port;
}

UDPClient::UDPClient(QHostAddress clientAddress, quint16 clientPort)
{
    adress = clientAddress;
    port = clientPort;
}

QHostAddress UDPClient::getAdress() const
{
    return adress;
}

void UDPClient::setAdress(const QHostAddress &value)
{
    adress = value;
}
quint16 UDPClient::getPort() const
{
    return port;
}

void UDPClient::setPort(const quint16 &value)
{
    port = value;
}

bool UDPClient::operator==(const UDPClient &other) const
{
    return other.adress == adress && other.port == port;
}

UDPClient& UDPClient::operator=(const UDPClient &other)
{
    adress = other.adress;
    port = other.port;
    return *this;
}

