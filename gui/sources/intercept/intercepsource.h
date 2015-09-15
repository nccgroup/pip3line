/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef INTERCEPSOURCE_H
#define INTERCEPSOURCE_H

#include "../bytesourceabstract.h"
#include <QList>
#include <QDateTime>
#include <QStringList>
#include <QQueue>
#include "../blocksources/blockssource.h"
#include "payloadmodel.h"
#include "payload.h"

class IntercepSource : public ByteSourceAbstract
{
        Q_OBJECT
    public:
        explicit IntercepSource(QObject *parent = 0);
        ~IntercepSource();
        QString description();
        void setData(QByteArray data, quintptr source = INVALID_SOURCE);
        QByteArray getRawData();
        quint64 size();
        QByteArray extract(quint64 offset, int length);
        char extract(quint64 offset);
        void replace(quint64 offset, int length, QByteArray repData, quintptr source = INVALID_SOURCE);
        void insert(quint64 offset, QByteArray repData, quintptr source = INVALID_SOURCE);
        void remove(quint64 offset, int length, quintptr source = INVALID_SOURCE);

        int getViewOffset(quint64 realoffset);
        int preferredTabType();
        bool isOffsetValid(quint64 offset);
        bool isReadableText();
        PayloadModel * getModel();
        bool getIntercepting() const;
        void fromLocalFile(QString fileName);

        BaseStateAbstract *getStateMngtObj();

    public slots:
        void setCurrentBlockSource(BlocksSource *value);
        void addNewBlock(Block block);
        void configureCurrentPayload(Payload * payl);
        void forwardCurrentBlock();
        void setIntercepting(bool value);
        void clear(quintptr source = INVALID_SOURCE);
    private slots:
        void logError(QString mess, QString source);
        void logStatus(QString mess, QString source);
        void onBlockSourceDeleted();
    private:
        Q_DISABLE_COPY(IntercepSource)
        BlocksSource * currentBlockSource;
        QWidget *requestGui(QWidget *parent,ByteSourceAbstract::GUI_TYPE type);
        bool validateOffsetAndSize(quint64 offset, int length);
        Payload *currentPayload;
        PayloadModel *model;
        bool intercepting;
        bool processingPayload;
        QQueue<Payload *> payloadQueue;
};

class IntercepSourceStateObj : public ByteSourceStateObj
{
        Q_OBJECT
    public:
        explicit IntercepSourceStateObj(IntercepSource *is);
        ~IntercepSourceStateObj();
    protected:
        void internalRun();
};

#endif // INTERCEPSOURCE_H
