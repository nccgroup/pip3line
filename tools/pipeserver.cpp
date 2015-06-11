/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "pipeserver.h"
#include <commonstrings.h>
#include "shared/guiconst.h"
using namespace GuiConst;

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
    QLocalSocket *socket = new(std::nothrow) QLocalSocket();
    if (socket != NULL) {
        socket->setSocketDescriptor(socketDescriptor);

        emit status(tr("New Client for %1").arg(socketDescriptor), PIPE_SOCKET);

        connect(socket, SIGNAL(disconnected()), this, SLOT(stop()));
        in  = socket;
        if (out == NULL)
            out = in;

        StreamProcessor::run();
        emit status(tr("Client for %1 disconnected").arg(socketDescriptor), PIPE_SOCKET);
        emit finished(this);
    }  else {
        qFatal("Cannot allocate memory pipe socket X{");
    }
}

PipeServer::PipeServer(TransformMgmt *tFactory, QObject *parent) :
    ServerAbstract(tFactory)
{
    pipeServer = new(std::nothrow) InternalLocalSocketServer(parent);
    if (pipeServer != NULL) {
        connect(pipeServer, SIGNAL(newClient(quintptr)), this, SLOT(processingNewClient(quintptr)));
    }  else {
        qFatal("Cannot allocate memory pipe server X{");
    }
}

PipeServer::~PipeServer()
{
    if (pipeServer != NULL) {
        pipeServer->close();
        delete pipeServer;
    }
}

void PipeServer::setPipeName(const QString &name)
{
    pipeName = name;
}

bool PipeServer::startServer()
{
    bool ret = false;
    if (pipeServer != NULL) {
        ret = pipeServer->listen(pipeName);
        if (ret) {
            logStatus(tr("Started for %1").arg(pipeName), PIPE_SERVER);
        } else {
            logError(tr("Error while starting:\n%1").arg(pipeServer->errorString()),PIPE_SERVER);
        }
    }
    return ret;
}

void PipeServer::stopServer()
{
    if (pipeServer != NULL) {
        if (pipeServer->isListening()) {
            pipeServer->close();
            ServerAbstract::stopServer();
            logStatus(tr("Stopped (%1)").arg(pipeName),PIPE_SERVER);
        }
    }
}

QString PipeServer::getLastError()
{
    return pipeServer != NULL ? pipeServer->errorString() : "Null pipe server" ;
}

void PipeServer::processingNewClient(quintptr socketDescriptor)
{
    LocalSocketProcessor * processor = new(std::nothrow) LocalSocketProcessor(transformFactory, socketDescriptor,this);
    if (processor != NULL) {
        connect(processor,SIGNAL(finished(LocalSocketProcessor *)), this, SLOT(processorFinished(LocalSocketProcessor *)), Qt::QueuedConnection);
        connect(processor,SIGNAL(error(QString,QString)), this, SLOT(logError(QString,QString)));
        connect(processor,SIGNAL(status(QString,QString)), this, SLOT(logStatus(QString,QString)));
        connect(this, SIGNAL(newTransformChain(QString)), processor, SLOT(setTransformsChain(QString)));

        processor->setOutput(output);

        confLocker.lock();
        processor->setTransformsChain(tconf);
        processor->setSeparator(separator);
        processor->setDecoding(decode);
        processor->setEncoding(encode);
        clientProcessor.append(processor);
        confLocker.unlock();

        processor->start();
    } else {
        qFatal("Cannot allocate memory pipe processor X{");
    }
}

void PipeServer::processorFinished(LocalSocketProcessor * target)
{
    confLocker.lock();
    ProcessingStats preStats = target->getStats();
    stats += preStats;
    if (clientProcessor.contains(target)) {
        clientProcessor.removeAll(target);
    } else {
        qWarning("[Pipe server] Processor finished, but target not found in the list ... T_T");
    }

    confLocker.unlock();
    target->deleteLater();
}

QString PipeServer::getServerType()
{
    return PIPE_SERVER;
}
