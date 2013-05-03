/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "tcpserver.h"
#include <QHostAddress>
#include <commonstrings.h>
using namespace Pip3lineConst;

#include <QDebug>


InternalTcpServer::InternalTcpServer(QObject *parent):
    QTcpServer(parent)
{
}

void InternalTcpServer::incomingConnection(int socketDescriptor)
{
    emit newClient(socketDescriptor);
}

const QString TcpSocketProcessor::TCP_SOCKET = "Tcp socket";

TcpSocketProcessor::TcpSocketProcessor(TransformMgmt *tFactory, int socketDescr, QObject *parent) :
    StreamProcessor(tFactory,parent)
{
    socketDescriptor = socketDescr;
    socket = NULL;
}

TcpSocketProcessor::~TcpSocketProcessor()
{
    if (socket != NULL)
        delete socket;
}

void TcpSocketProcessor::run()
{
    socket = new(std::nothrow) QTcpSocket();

    if (socket != NULL) {

        bool ret = socket->setSocketDescriptor(socketDescriptor);

        if (!ret) {
            emit error(tr("Could set socket descriptor for the client: %1").arg(socket->errorString()), TCP_SOCKET);
        }
        QString cAddr = socket->peerAddress().toString();
        int port = socket->peerPort();
        emit status(tr("New Client : %1:%2").arg(cAddr).arg(port), TCP_SOCKET);
        connect(socket, SIGNAL(disconnected()), this, SLOT(stop()));
        in  = socket;
        if (out == NULL)
            out = in;

        StreamProcessor::run();
        emit status(tr("Client disconnected : %1:%2").arg(cAddr).arg(port), TCP_SOCKET);
        emit finished(this);

    } else {
        qFatal("Cannot allocate memory for tcp socket X{");
    }
}

TcpServer::TcpServer(TransformMgmt * tFactory, QObject *parent) :
    ServerAbstract(tFactory)
{
    tcpServer = new(std::nothrow) InternalTcpServer(parent);
    if (tcpServer != NULL)
        connect(tcpServer, SIGNAL(newClient(int)), this, SLOT(processingNewClient(int)));
    else {
        qFatal("Cannot allocate memory for tcpServer X{");
    }
}

TcpServer::~TcpServer()
{
    if (tcpServer != NULL) {
        tcpServer->close();
        delete tcpServer;
    }
}

void TcpServer::setIP(const QString &ip)
{
    IP = ip;
}

void TcpServer::processingNewClient(int socketDescriptor)
{
    TcpSocketProcessor * processor = new(std::nothrow) TcpSocketProcessor(transformFactory, socketDescriptor,this);
    if (processor != NULL) {
        processor->setOutput(output);

        confLocker.lock();
        processor->setTransformsChain(tconf);
        processor->setSeparator(separator);
        processor->setDecoding(decode);
        processor->setEncoding(encode);
        clientProcessor.append(processor);
        confLocker.unlock();

        connect(processor,SIGNAL(finished(TcpSocketProcessor*)), this, SLOT(processorFinished(TcpSocketProcessor*)), Qt::QueuedConnection);
        connect(processor,SIGNAL(error(QString,QString)), this, SLOT(logError(QString,QString)));
        connect(processor,SIGNAL(status(QString,QString)), this, SLOT(logStatus(QString,QString)));
        processor->start();
    } else {
        qFatal("Cannot allocate memory for TCP processor X{");
    }
}

QString TcpServer::getServerType()
{
    return TCP_SERVER;
}

void TcpServer::stopServer()
{
    if (tcpServer != NULL) {
        if (tcpServer->isListening()) {
            tcpServer->close();
            ServerAbstract::stopServer();
            logStatus(tr("Stopped %1:%2").arg(IP).arg(port), TCP_SERVER);
        }
    }
}

bool TcpServer::startServer()
{
    bool ret = false;
    if (tcpServer != NULL) {
        ret = tcpServer->listen( QHostAddress(IP), port);
        if (ret) {
            logStatus(tr("Started on %1:%2").arg(IP).arg(port), TCP_SERVER);
        } else {
            logError(tr("Error while starting: %1").arg(tcpServer->errorString()),TCP_SERVER);
        }
    }
    return ret;
}

QString TcpServer::getLastError()
{
    return tcpServer != NULL ? tcpServer->errorString() : "NULL tcp server";
}

void TcpServer::processorFinished(TcpSocketProcessor * target)
{
    confLocker.lock();

    if (clientProcessor.contains(target)) {
        clientProcessor.removeAll(target);
    } else {
        qWarning("[TCP server] Processor finished, but target not found in the list ... T_T");
    }

    confLocker.unlock();
    target->deleteLater();
}


void TcpServer::setPort(int nport)
{
    port = nport;
}

