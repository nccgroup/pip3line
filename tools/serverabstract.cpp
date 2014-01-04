/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "serverabstract.h"
#include "processingstats.h"
#include <QObject>

ServerAbstract::ServerAbstract(TransformMgmt *tFactory)
{
    transformFactory = tFactory;
    output = NULL;
    separator = Processor::DEFAULT_SEPARATOR;
    encode = false;
    decode = false;
    allowForwardingLogs = false;
    stats.reset();
}

ServerAbstract::~ServerAbstract()
{

}

void ServerAbstract::setOutput(QIODevice *out)
{
    output = out;
}

void ServerAbstract::clearOutput()
{
    output = NULL;
}

void ServerAbstract::setTransformations(const QString &conf)
{
    confLocker.lock();
    tconf = conf;
    confLocker.unlock();
    emit newTransformChain(conf);
}

void ServerAbstract::setSeparator(char c)
{
    confLocker.lock();
    separator = c;
    confLocker.unlock();
}

void ServerAbstract::stopServer()
{
    Processor * processor;
    confLocker.lock();

    for (int i = 0; i < clientProcessor.size(); i++) {
        processor = clientProcessor.at(i);
        processor->stop();
        if (!processor->wait(10000))
            logError(QObject::tr("%1 Client Processor %1 seems stuck, this may cause a crash ...").arg(getServerType()).arg((long)processor,0,16),"");

    }
    stats.reset();

    confLocker.unlock();
}

ProcessingStats ServerAbstract::getStats()
{
    confLocker.lock();
    ProcessingStats ret;
    ProcessingStats newStats;
    ret += stats;
    for (int i = 0; i < clientProcessor.size(); i++) {
        newStats = clientProcessor.at(i)->getStats();
        ret += newStats;
    }
    confLocker.unlock();
    return ret;
}

void ServerAbstract::setEncoding(bool flag)
{
    confLocker.lock();
    encode = flag;
    confLocker.unlock();
}

void ServerAbstract::setDecoding(bool flag)
{
    confLocker.lock();
    decode = flag;
    confLocker.unlock();
}

void ServerAbstract::logError(const QString mess, const QString id)
{
    confLocker.lock();
    stats.incrErrorsCount();
    confLocker.unlock();
    if (allowForwardingLogs)
        emit error(mess, id);
}

void ServerAbstract::logStatus(const QString mess, const QString id)
{
    confLocker.lock();
    stats.incrStatusCount();
    confLocker.unlock();
    if (allowForwardingLogs)
        emit status(mess, id);
}
bool ServerAbstract::getAllowForwardingLogs() const
{
    return allowForwardingLogs;
}

void ServerAbstract::setAllowForwardingLogs(bool value)
{
    allowForwardingLogs = value;
}

