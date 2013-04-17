/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "pipeserver.h"
#include <commonstrings.h>
using namespace Pip3lineConst;

InternalLocalSocketServer::InternalLocalSocketServer(QObject *parent):
    QLocalServer(parent)
{
}

void InternalLocalSocketServer::incomingConnection(quintptr socketDescriptor)
{
    emit newClient(socketDescriptor);
}

LocalSocketProcessor::LocalSocketProcessor(TransformMgmt *tFactory, quintptr socketDescr, QObject *parent):
    StreamProcessor(tFactory,parent)
{
    socketDescriptor = socketDescr;
}

LocalSocketProcessor::~LocalSocketProcessor()
{
}

void LocalSocketProcessor::run()
{
    QLocalSocket *socket = new QLocalSocket();
    socket->setSocketDescriptor(socketDescriptor);

    emit status(tr("New Client for %1").arg(socketDescriptor), PIPE_SOCKET);

    connect(socket, SIGNAL(disconnected()), this, SLOT(stop()));
    in  = socket;
    if (out == 0)
        out = in;

    StreamProcessor::run();

    emit status(tr("Client for %1 disconnected").arg(socketDescriptor), PIPE_SOCKET);
    emit finished(this);
}

PipeServer::PipeServer(TransformMgmt *tFactory, QObject *parent) :
    ServerAbstract(tFactory)
{
    pipeServer = new InternalLocalSocketServer(parent);
    connect(pipeServer, SIGNAL(newClient(quintptr)), this, SLOT(processingNewClient(quintptr)));
}

PipeServer::~PipeServer()
{
    pipeServer->close();
    delete pipeServer;
}

void PipeServer::setPipeName(const QString &name)
{
    pipeName = name;
}

bool PipeServer::startServer()
{
    bool ret = pipeServer->listen(pipeName);
    if (ret) {
        logStatus(tr("Started for %1").arg(pipeName), PIPE_SERVER);
    } else {
        logError(tr("Error while starting:\n%1").arg(pipeServer->errorString()),PIPE_SERVER);
    }
    return ret;
}

void PipeServer::stopServer()
{
    if (pipeServer->isListening()) {
        pipeServer->close();
        ServerAbstract::stopServer();
        logStatus(tr("Stopped (%1)").arg(pipeName),PIPE_SERVER);
    }
}

QString PipeServer::getLastError()
{
    return pipeServer->errorString();
}

void PipeServer::processingNewClient(quintptr socketDescriptor)
{
    LocalSocketProcessor * processor = new LocalSocketProcessor(transformFactory, socketDescriptor,this);
    connect(processor,SIGNAL(finished(LocalSocketProcessor *)), this, SLOT(processorFinished(LocalSocketProcessor *)), Qt::QueuedConnection);
    connect(processor,SIGNAL(error(QString,QString)), this, SLOT(logError(QString,QString)));
    connect(processor,SIGNAL(status(QString,QString)), this, SLOT(logStatus(QString,QString)));

    processor->setOutput(output);

    confLocker.lock();
    processor->setTransformsChain(tconf);
    processor->setSeparator(separator);
    processor->setDecoding(decode);
    processor->setEncoding(encode);
    clientProcessor.append(processor);
    confLocker.unlock();

    processor->start();
}

void PipeServer::processorFinished(LocalSocketProcessor * target)
{
    confLocker.lock();

    if (clientProcessor.contains(target)) {
        clientProcessor.removeAll(target);
    } else {
        logError(tr("Processor finished, but target not found in the list ... T_T"), PIPE_SERVER);
    }

    confLocker.unlock();
    target->deleteLater();
}

QString PipeServer::getServerType()
{
    return PIPE_SERVER;
}
