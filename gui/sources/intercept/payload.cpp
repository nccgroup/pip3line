/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "payload.h"

const int Payload::USER_SOURCE = -1;

Payload::Payload(QByteArray data, int sourceid) :
    originalPayload(data),
    payload(data),
    sourceid(sourceid)
{
    timestamp = QDateTime::currentDateTime();
    direction = NODIRECTION;
}

Payload::Payload(Block * block)
{
    originalPayload = block->getData();
    sourceid = block->getSourceid();
    timestamp = QDateTime::currentDateTime();
    direction = NODIRECTION;
}

Payload::~Payload()
{
    history.clear();
    sourceid = USER_SOURCE;
}

QByteArray Payload::getOriginalPayload() const
{
    return originalPayload;
}

QByteArray Payload::getPayload() const
{
    return payload;
}

void Payload::setPayload(const QByteArray &value)
{
    payload = value;
}

QList<ByteSourceAbstract::HistItem> Payload::getHistory() const
{
    return history;
}

void Payload::setHistory(const QList<ByteSourceAbstract::HistItem> &value)
{
    history = value;
}

QStringList Payload::getSourceString() const
{
    return sourceString;
}

void Payload::setSourceString(const QStringList &value)
{
    sourceString = value;
}

QDateTime Payload::getTimestamp() const
{
    return timestamp;
}

void Payload::setTimestamp(const QDateTime &value)
{
    timestamp = value;
}

Payload::Direction Payload::getDirection() const
{
    return direction;
}

void Payload::setDirection(const Direction &value)
{
    direction = value;
}

bool Payload::hasBeenModified()
{
    return (originalPayload == payload);
}

int Payload::size()
{
    return payload.size();
}

Block *Payload::toBlock()
{
    Block * datab = new(std::nothrow) Block(payload,sourceid);
    if (datab == NULL) qFatal("Cannot allocate Block for Payload X{");

    return datab;
}
int Payload::getSourceid() const
{
    return sourceid;
}

