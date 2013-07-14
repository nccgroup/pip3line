/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BASICSOURCE_H
#define BASICSOURCE_H

#include "bytesourceabstract.h"

class BasicSource : public ByteSourceAbstract
{
        Q_OBJECT
    public:
        BasicSource(QObject *parent = 0);
        ~BasicSource();
        void setData(QByteArray data, quintptr source = INVALID_SOURCE);
        QByteArray getRawData();
        qint64 size();
        QByteArray extract(qint64 offset, int length);
        void replace(qint64 offset, int length, QByteArray repData, quintptr source = INVALID_SOURCE);
        void insert(qint64 offset, QByteArray repData, quintptr source = INVALID_SOURCE);
        void remove(quint64 offset, qint64 length, quintptr source = INVALID_SOURCE);
        void clear(quintptr source = INVALID_SOURCE);
        bool contains(char c);
        bool historyForward();
        bool historyBackward();

        bool isReadableText();

        qint64 indexOf(QByteArray item, qint64 offset = 0);
    private:
        Q_DISABLE_COPY(BasicSource)
        static const QByteArray TEXT;
        static const QString LOGID;
        bool validateOffsetAndSize(qint64 offset, int length);
        void addToHistory(QByteArray &newData);
        QByteArray rawData;
        QList<QByteArray> history;
        int currentHistoryPointer;
};

#endif // BASICSOURCE_H
