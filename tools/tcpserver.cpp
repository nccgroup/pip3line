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
#if QT_VERSION >= 0x050000
void InternalTcpServer::incomingConnection(qintptr socketDescriptor)
#else
void InternalTcpServer::incomingConnection(int socketDescriptor)
#endif
{
    emit newClient(socketDescriptor);
}

const QString TcpSocketProcessor::TCP_SOCKET = "Tcp socket";

TcpSocketProcessor::TcpSocketProcessor(TransformMgmt *tFactory,
#if QT_VERSION >= 0x050000
                                       qintptr socketDescr,
#else
                                       int socketDescr,
#endif
                                       QObject *parent) :
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
#if QT_VERSION >= 0x050000
        connect(tcpServer, SIGNAL(newClient(qintptr)), this, SLOT(processingNewClient(qintptr)));
#else
        connect(tcpServer, SIGNAL(newClient(int)), this, SLOT(processingNewClient(int)));
#endif

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

#if QT_VERSION >= 0x050000
void TcpServer::processingNewClient(qintptr socketDescriptor)
#else
void TcpServer::processingNewClient(int socketDescriptor)
#endif
{
    TcpSocketProcessor * processor = new(std::nothrow) TcpSocketProcessor(transformFactory, socketDescriptor);
    if (processor != NULL) {
        processor->setOutput(output);

        confLocker.lock();
        processor->setTransformsChain(tconf);
        processor->setSeparator(separator);
        processor->setDecoding(decode);
        processor->setEncoding(encode);
        clientProcessor.append(processor);
        confLocker.unlock();

        connect(processor,SIGNAL(finished(TcpSocketProcessor*)), SLOT(processorFinished(TcpSocketProcessor*)), Qt::QueuedConnection);
        connect(processor,SIGNAL(error(QString,QString)), SLOT(logError(QString,QString)));
        connect(processor,SIGNAL(status(QString,QString)), SLOT(logStatus(QString,QString)));
        connect(this, SIGNAL(newTransformChain(QString)), processor, SLOT(setTransformsChain(QString)));
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
            emit status(tr("Stopped %1:%2").arg(IP).arg(port), TCP_SERVER);
        }
    }
}

bool TcpServer::startServer()
{
    bool ret = false;
    if (tcpServer != NULL) {
        ret = tcpServer->listen( QHostAddress(IP), port);
        if (ret) {
            emit status(tr("Started on %1:%2").arg(IP).arg(port), TCP_SERVER);
        } else {
            emit error(tr("Error while starting: %1").arg(tcpServer->errorString()),TCP_SERVER);
        }
    }
    return ret;
}

QString TcpServer::getLastError()
{
    return tcpServer != NULL ? tcpServer->errorString() : tr("NULL tcp server");
}

void TcpServer::processorFinished(TcpSocketProcessor * target)
{
    confLocker.lock();

    ProcessingStats preStats = target->getStats();
    stats += preStats;
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

