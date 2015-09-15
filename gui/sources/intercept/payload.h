/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <QByteArray>
#include "../blocksources/blockssource.h"
#include <sources/bytesourceabstract.h>
#include <QDateTime>
#include <QList>

class Payload
{
    public:
        enum Direction {RIGHTLEFT, LEFTRIGHT, NODIRECTION};
        static const int USER_SOURCE;
        explicit Payload(QByteArray data, int sourceid = USER_SOURCE);
        explicit Payload(Block *block);
        ~Payload();
        QByteArray getOriginalPayload() const;
        QByteArray getPayload() const;
        void setPayload(const QByteArray &value);
        QList<ByteSourceAbstract::HistItem> getHistory() const;
        void setHistory(const QList<ByteSourceAbstract::HistItem> &value);
        QStringList getSourceString() const;
        void setSourceString(const QStringList &value);
        QDateTime getTimestamp() const;
        void setTimestamp(const QDateTime &value);
        Payload::Direction getDirection() const;
        void setDirection(const Direction &value);
        bool hasBeenModified();
        int size();
        Block * toBlock();
        int getSourceid() const;

private:
        QByteArray originalPayload;
        QByteArray payload;
        QList<ByteSourceAbstract::HistItem> history;
        int sourceid;
        QStringList sourceString;
        QDateTime timestamp;
        Direction direction;
};

#endif // PAYLOAD_H
