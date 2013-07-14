/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BYTESOURCEABSTRACT_H
#define BYTESOURCEABSTRACT_H

#include <QObject>
#include <commonstrings.h>

class ByteSourceAbstract : public QObject
{
        Q_OBJECT
    public:
        enum CAPABILITIES {
            CAP_RESET = 1,
            CAP_REPLACE = 2,
            CAP_INSERT = 4,
            CAP_REMOVE = 8,
            CAP_HISTORY = 16,
            CAP_TRANSFORM = 32
                          };
        explicit ByteSourceAbstract(QObject *parent = 0);
        virtual ~ByteSourceAbstract();
        virtual void setData(QByteArray data, quintptr source = INVALID_SOURCE);
        virtual QByteArray getRawData();
        virtual qint64 size();
        virtual QByteArray extract(qint64 offset, int length) = 0;
        virtual void replace(qint64 offset, int length, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void insert(qint64 offset, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void remove(quint64 offset, qint64 length, quintptr source = INVALID_SOURCE);
        virtual void clear(quintptr source = INVALID_SOURCE);
        virtual bool contains(char c);
        virtual bool historyForward();
        virtual bool historyBackward();
        virtual void fromLocalFile(QString fileName);

        virtual bool isReadableText();
        virtual qint64 indexOf(QByteArray item, qint64 offset = 0);

        bool hasCapability(CAPABILITIES);
        static QString toPrintableString(const QByteArray &val);

    signals:
        void updated(quintptr source);
        void log(QString mess, QString source, Pip3lineConst::LOGLEVEL level);
        
    protected:
        static const quintptr INVALID_SOURCE;
        quint32 capabilities;
};

#endif // BYTESOURCEABSTRACT_H
