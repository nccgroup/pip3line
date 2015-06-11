/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BASESTATEABSTRACT_H
#define BASESTATEABSTRACT_H

#include <QObject>
#include <QColor>
#include <QBitArray>
#include <commonstrings.h>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

class BaseStateAbstract : public QObject
{
        Q_OBJECT
    public:
        explicit BaseStateAbstract(QObject *parent = 0);
        virtual ~BaseStateAbstract();
        QString getName() const;
        void setName(const QString &value);
        virtual void run();
        QXmlStreamWriter *getWriter() const;
        void setWriter(QXmlStreamWriter *value);

        QXmlStreamReader *getReader() const;
        void setReader(QXmlStreamReader *value);

        quint64 getFlags() const;
        void setFlags(const quint64 &value);

        bool readNext(QString expected);
        bool readNextStart();
        bool readNextStart(QString expected);
        bool skipUntilStartElement();
        bool skipUntilStartElement(QString expected);
        bool genCloseElement();
        bool writeCloseElement();
        bool readEndElement();
        bool readEndElement(QString expected);
        bool readEndAndNext(QString expected);

        QString write(QByteArray data);
        QByteArray readByteArray(QStringRef data);
        QByteArray readByteArray(QString data);

        QString write(bool value);
        bool readBool(QStringRef val);

        QString write(QColor color);
        QColor readColor(QStringRef val);

        QString write(int val);
        int readInt(QStringRef val, bool *ok);

        QString write(quint64 val);
        quint64 readUInt64(QStringRef val, bool *ok);

        QString write(QBitArray barray);
        QBitArray readBitArray(QStringRef val);

        QString write(QString val, bool compress = false); // for user strings, can be copmressed
        QString readString(QStringRef val);

    signals:
        void finished();
        void addNewState(BaseStateAbstract * stateobj);
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);

    public slots:
        void start();

    protected:
        void logWasExpectingClosing(QString source, QString expected);
        void logReaderIsNull(QString source);
        void logWriterIsNull(QString source);
        void logNotAdequate(QString source, QString expected);
        void logCannotRead(QString source);
        void logStatus(QString mess = QString("Status"));
        QXmlStreamWriter *writer;
        QXmlStreamReader *reader;
        QXmlStreamAttributes attributes;
        quint64 flags;
        QString name;
        QString actionName;
        static const QString NAME_NOT_SET;
        static const QChar B_ZERO;
        static const QChar B_ONE;
        static const char COMPRESSED_MARKER;

};

#endif // BASESTATEABSTRACT_H
