/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "tcpserverlistener.h"
#include "tcplistener.h"
#include "networkconfwidget.h"
#include <QThread>
#include <QDebug>
#include <QTimer>

InTcpServer::InTcpServer(QObject *parent) : QTcpServer(parent)
{

}

InTcpServer::~InTcpServer()
{

}
#if QT_VERSION >= 0x050000
void InTcpServer::incomingConnection(qintptr socketDescriptor)
#else
void InTcpServer::incomingConnection(int socketDescriptor)
#endif
{
    emit newClient(socketDescriptor);
}

const quint16 TcpServerListener::DEFAULT_PORT = 40000;
const QHostAddress TcpServerListener::DEFAULT_ADDRESS = QHostAddress::LocalHost;

TcpServerListener::TcpServerListener(QObject *parent) :
    BlocksSource(parent)
{
    listeningAddress = DEFAULT_ADDRESS;
    port = DEFAULT_PORT;
    server = NULL;
    workerThread = NULL;

    serverThread = new(std::nothrow) QThread();
    if (serverThread == NULL) {
        qFatal("Cannot allocate memory for serverThread X{");
    }
    moveToThread(serverThread);
    serverThread->start();

    separator='\x0a';
}

TcpServerListener::~TcpServerListener() {
    serverThread->quit();
    serverThread->wait();
    serverThread->deleteLater();
    serverThread = NULL;
    //delete server; don't need that

    if (workerThread != NULL) {
        workerThread->quit();
        workerThread->wait();
        delete workerThread;
        workerThread = NULL;
    }
}

void TcpServerListener::sendBlock(const Block & )
{
    qFatal("[TcpServerListener::sendBlock] forbidden secret");
}

bool TcpServerListener::startListening()
{
    startingWorkers();
    if (server == NULL) {
        server = new(std::nothrow) InTcpServer();
        if (server == NULL) {
            qFatal("Cannot allocate memory for QTcpServer X{");
        }
#if QT_VERSION >= 0x050000
        connect(server,SIGNAL(newClient(qintptr)), SLOT(handlingClient(qintptr)));
#else
        connect(server,SIGNAL(newClient(int)), SLOT(handlingClient(int)));
#endif
    }
    if (server->isListening()) { // already listening nothing to do
        return true;
    }


    if (!server->listen(listeningAddress,port)) {
        emit log(tr("could not start TCP server %1").arg(server->errorString()),metaObject()->className(), Pip3lineConst::LERROR);
        delete server;
        server = NULL;
        workerThread->quit();
        return false;
    }
    emit log(tr("TCP server started %1:%2").arg(listeningAddress.toString()).arg(port), "", Pip3lineConst::LSTATUS);
    emit started();
    return true;
}

void TcpServerListener::stopListening()
{
    if (server != NULL && server->isListening()) {
       emit log(tr("TCP server stopped %1:%2").arg(listeningAddress.toString()).arg(port), "", Pip3lineConst::LSTATUS);
       server->close();
       emit shutdownAllClient();
       stoppingWorkers();
       emit stopped();
       server->deleteLater();
       server = NULL;
    }
}

void TcpServerListener::postBlockForSending(Block block)
{
    TcpListener * client = static_cast<TcpListener *>(block.source);

    if (client != NULL) {
        if (clients.contains(client)) {
            client->postBlockForSending(block); // the client is going to take care of encoding the block
        } else {
            emit log(tr("Client disconnected cannot forward data block"), metaObject()->className(), Pip3lineConst::LERROR);
        }
    } else {
        qCritical() << "[TcpServerListener::postBlockForSending] NULL client";
    }
}

void TcpServerListener::clientFinished()
{
    TcpListener * client = static_cast<TcpListener *>(sender());
    if (client == NULL) {
        qWarning() << "[TcpServerListener] NULL client finished T_T";
    } else if (clients.contains(client)) {
        clients.removeAll(client);
       // qWarning() << "Client finished" << client;
    }
    else
        qCritical() << metaObject()->className() << "Could not find the client in the list" << client;

    delete client;
}

void TcpServerListener::onClientReceivedBlock(Block block)
{
    TcpListener *client = qobject_cast<TcpListener *>(sender());
    int index = clients.indexOf(client);
    if (index == -1) {
        qCritical() << tr("Could not find client in list");
    } else {
        block.sourceid = index;
        block.direction = Block::SOURCE;
        emit blockReceived(block);
    }
}

#if QT_VERSION >= 0x050000
void TcpServerListener::handlingClient(qintptr socketDescriptor)
#else
void TcpServerListener::handlingClient(int socketDescriptor)
#endif
{
    TcpListener * listener = new(std::nothrow) TcpListener(socketDescriptor);
    if (listener != NULL) {
       // qDebug() << "Listener created" << listener;
        listener->setDecodeinput(decodeInput);
        listener->setEncodeOutput(encodeOutput);
        clients.append(listener);
        listener->moveToThread(workerThread);
        connect(listener, SIGNAL(blockReceived(Block)), SLOT(onClientReceivedBlock(Block)));
        connect(listener, SIGNAL(stopped()), SLOT(clientFinished()));
        connect(listener, SIGNAL(error(QString,QString)), SIGNAL(error(QString,QString)));
        connect(listener, SIGNAL(status(QString,QString)), SIGNAL(status(QString,QString)));
        connect(this, SIGNAL(shutdownAllClient()), listener, SLOT(stopListening()),Qt::QueuedConnection);
        QTimer::singleShot(0,listener,SLOT(startListening()));
    } else {
        qFatal("Cannot allocate memory for TcpListener X{");
    }
}

void TcpServerListener::startingWorkers()
{
    if (workerThread == NULL) {
        workerThread = new(std::nothrow) QThread();
        if (workerThread == NULL) {
            qFatal("Cannot allocate memory for workerThread X{");
        }
    }
    if (workerThread != NULL) {
        workerThread->start();
    }
}

void TcpServerListener::stoppingWorkers()
{
    workerThread->quit();
    if (!workerThread->wait())
        qCritical() << metaObject()->className() << "Could not stop the worker thread";
}

QWidget *TcpServerListener::requestGui(QWidget *parent)
{
    NetworkConfWidget *ncw = new(std::nothrow)NetworkConfWidget(NetworkConfWidget::TCP_SERVER,parent);
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

void TcpServerListener::setPort(const quint16 &value)
{
    if (value != port) {
        port = value;
    }

}

void TcpServerListener::setListeningAddress(const QHostAddress &value)
{
    if (value != listeningAddress) {
        listeningAddress = value;
    }
}

