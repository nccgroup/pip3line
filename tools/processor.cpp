/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "processor.h"
#include "QMutexLocker"
#include <QDebug>

const char Processor::DEFAULT_SEPARATOR = '\n';

Processor::Processor(TransformMgmt * tFactory, QObject *parent) : QThread(parent)
{
    transformFactory = tFactory;
    outputLock = NULL;
    out = NULL;
    in = NULL;
    separator = DEFAULT_SEPARATOR;
    ocount = 0;
    encode = false;
    decode = false;
}

Processor::~Processor()
{
    clearChain();
}


bool Processor::configureFromFile(const QString &fileName)
{
    return setTransformsChain(transformFactory->loadConfFromFile(fileName));
}

bool Processor::configureFromName(const QString &name, TransformAbstract::Way way)
{
    TransformChain chain;
    TransformAbstract * t = transformFactory->getTransform(name);
    if (t != 0) {
        t->setWay(way);
        chain.append(t);
    }
    else
        return false;
    return setTransformsChain(chain);
}

bool Processor::setTransformsChain(TransformChain ntlist)
{

    QMutexLocker lock(&tranformsLock);
    clearChain();
    tlist = ntlist;
    if (tlist.size() == 0) {
        emit error(tr("No Transformation loaded"), "Processor");
        return false;
    } else {
        for (int i = 0; i < tlist.size(); i++) {
            connect(tlist.at(i),SIGNAL(error(QString,QString)), this, SLOT(logError(QString, QString)));
            connect(tlist.at(i),SIGNAL(warning(QString,QString)), this, SLOT(logError(QString, QString)));
        }
        emit status(tr("%1 transformations loaded").arg(tlist.size()), "Processor");
    }
    return true;
}

bool Processor::setTransformsChain(const QString &xmlConf)
{
    QXmlStreamReader streamIn(xmlConf);
    return setTransformsChain(transformFactory->loadConfFromXML(&streamIn));
}

void Processor::setOutputMutex(QMutex *mutex)
{
    outputLock = mutex;
}

void Processor::setOutput(QIODevice *nout)
{
    out = nout;
}

void Processor::setInput(QIODevice *nin)
{
    in = nin;
}

void Processor::clearOutputMutex()
{
    outputLock = NULL;
}

void Processor::setSeparator(char c)
{
    separator = c;
}

long Processor::getStatsOut()
{
    QMutexLocker lock(&statsLock);
    return ocount;
}

void Processor::setEncoding(bool flag)
{
    encode = flag;
}

void Processor::setDecoding(bool flag)
{
    decode = flag;
}

void Processor::logError(const QString mess, const QString id)
{
    emit error(mess, id);
}

void Processor::logStatus(const QString mess, const QString id)
{
    emit status(mess, id);
}

void Processor::writeBlock(const QByteArray &data)
{

    QByteArray temp = data;
    QByteArray temp2;
    QByteArray * outputval;
    int i = 0;

    if (decode)
        temp = QByteArray::fromBase64(temp);

    for (i = 0; i < tlist.size(); i++) {
        if (i % 2 == 0)
            tlist.at(i)->transform(temp,temp2);
        else
            tlist.at(i)->transform(temp2,temp);
    }
    if (i % 2 == 0)
        outputval = &temp;
    else
        outputval = &temp2;

    if (!outputval->isEmpty()) {
        if (encode)
            *outputval = outputval->toBase64();
        outputval->append(separator);
        if (outputLock != 0)
            outputLock->lock();

        while (outputval->size() > 0) {
            int whatHasBeenDone = out->write(*outputval);
            if (whatHasBeenDone < 0) {
                emit error(out->errorString(), "Processor");
                break;
            }

            *outputval = outputval->remove(0,whatHasBeenDone);

        }

        if (outputLock != 0)
            outputLock->unlock();
        statsLock.lock();
        ocount++;
        statsLock.unlock();
    }
}

void Processor::clearChain()
{
    while (!tlist.isEmpty())
        delete tlist.takeLast();
}
