/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "serverabstract.h"
#include <QObject>

ServerAbstract::ServerAbstract(TransformMgmt *tFactory)
{
    transformFactory = tFactory;
    output = 0;
    separator = Processor::DEFAULT_SEPARATOR;
}

void ServerAbstract::setOutput(QIODevice *out)
{
    output = out;
}

void ServerAbstract::clearOutput()
{
    output = 0;
}

void ServerAbstract::setTransformations(const QString &conf)
{
    confLocker.lock();
    tconf = conf;
    for (int i = 0; i < clientProcessor.size(); i++) {
        clientProcessor.at(i)->setTransformsChain(conf);
    }
    confLocker.unlock();
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
        bool ok = processor->wait();
        if (!ok)
            logError(QObject::tr("%1 Client Processor %1 seems stuck, this may cause a crash ...").arg(getServerType()).arg((long)processor,0,16),"");

    }

    confLocker.unlock();
}

long ServerAbstract::getStatsOut()
{
    long ret = 0;
    confLocker.lock();
    for (int i = 0; i < clientProcessor.size(); i++) {
        ret += clientProcessor.at(i)->getStatsOut();
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
    emit error(mess, id);
}

void ServerAbstract::logStatus(const QString mess, const QString id)
{
    emit status(mess, id);
}
