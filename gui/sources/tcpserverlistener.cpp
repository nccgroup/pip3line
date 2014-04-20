/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "tcpserverlistener.h"
#include "tcplistener.h"
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

TcpServerListener::TcpServerListener(QObject *parent) :
    BlocksSource(parent)
{
    listeningAddress = QHostAddress::LocalHost;
    port = 40000;
    server = NULL;
    workerThread = new(std::nothrow) QThread();
    if (workerThread == NULL) {
        qFatal("Cannot allocate memory for workerThread X{");
    }
    qDebug() << "Worker" << workerThread;

    serverThread = new(std::nothrow) QThread();
    if (serverThread == NULL) {
        qFatal("Cannot allocate memory for serverThread X{");
    }
    qDebug() << "Server" << serverThread;

    separator='\x0a';
}

TcpServerListener::~TcpServerListener() {
    stopListening();
    delete server;
    delete workerThread;
}

void TcpServerListener::sendBlock(const QByteArray &block)
{
    emit broadcastBlock(block);
}

void TcpServerListener::startListening()
{
    startingWorkers();
    if (server == NULL) {
        server = new(std::nothrow) InTcpServer(serverThread);
        if (server == NULL) {
            qFatal("Cannot allocate memory for QTcpServer X{");
        }
#if QT_VERSION >= 0x050000
        connect(server,SIGNAL(newClient(qintptr)), SLOT(handlingClient(qintptr)));
#else
        connect(server,SIGNAL(newClient(int)), SLOT(handlingClient(int)));
#endif
    }
    if (server->isListening())
        return;


    if (!server->listen(listeningAddress,port)) {
        qCritical() << "could not start TCP server" << server->errorString();
        workerThread->quit();
        serverThread->quit();
    } else {
        qDebug() << tr("Tcp server started %1:%2").arg(listeningAddress.toString()).arg(port);
    }
}

void TcpServerListener::stopListening()
{
    if (server != NULL && server->isListening()) {
       qDebug() << tr("Stopping TCP Server %1:%2").arg(listeningAddress.toString()).arg(port);
       server->close();
       for (int i = 0; i < clients.size(); i++)
           QTimer::singleShot(0,clients.at(i),SLOT(stopListening()));
       stoppingWorkers();

    }
}

void TcpServerListener::clientFinished()
{
    TcpListener * client = dynamic_cast<TcpListener *>(sender());
    if (client == NULL) {
        qWarning() << "[TcpServerListener] NULL client finished";
    } else if (clients.contains(client)) {
        clients.removeAll(client);
       // qWarning() << "Client finished" << client;
    }
    else
        qCritical() << metaObject()->className() << "Could not find the client in the list" << client;

    delete client;
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
        listener->setBase64Applied(base64Applied);
        clients.append(listener);
        listener->moveToThread(workerThread);
        connect(listener, SIGNAL(newBlock(QByteArray)), SIGNAL(blockReceived(QByteArray)));
        connect(listener, SIGNAL(finished()), SLOT(clientFinished()));
        connect(listener, SIGNAL(error(QString,QString)), SIGNAL(error(QString,QString)));
        connect(listener, SIGNAL(status(QString,QString)), SIGNAL(status(QString,QString)));
        QTimer::singleShot(0,listener,SLOT(startListening()));
    } else {
        qFatal("Cannot allocate memory for TcpListener X{");
    }
}

void TcpServerListener::startingWorkers()
{
    if (serverThread != QThread::currentThread())
        moveToThread(serverThread);
    serverThread->start();

    workerThread->start();
}

void TcpServerListener::stoppingWorkers()
{
    workerThread->quit();
    if (!workerThread->wait())
        qCritical() << metaObject()->className() << "Could not stop the worker thread";

    serverThread->quit();
    if (!serverThread->wait())
        qCritical() << metaObject()->className() << "Could not stop the server thread";
}

QHostAddress TcpServerListener::getListeningAddress() const
{
    return listeningAddress;
}

void TcpServerListener::setListeningAddress(const QHostAddress &value)
{
    if (value != listeningAddress) {
        if (server != NULL && server->isListening()) {
            qDebug() << "The new address will be used once the server is restarted";
        }

        listeningAddress = value;
    }
}

