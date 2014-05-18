/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef INTERCEPSOURCE_H
#define INTERCEPSOURCE_H

#include "bytesourceabstract.h"
#include <QList>
#include <QDateTime>
#include <QStringList>
#include <QAbstractTableModel>
#include <QQueue>
#include <QFont>
#include <QIcon>
#include "blockssource.h"

class QAbstractItemModel;

class Payload
{
    public:
        QByteArray originalPayload;
        QByteArray payload;
        QList<ByteSourceAbstract::HistItem> history;
        BlocksSource * source;
        int sourceid;
        QDateTime timestamp;
        Block::BLOCK_ORIGIN direction;
};

class PayloadModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        static const int TIMESPTAMP_COLUMN;
        static const int DIRECTION_COLUMN;
        static const int PAYLOAD_COLUMN;
        explicit PayloadModel(QObject * parent = 0);
        ~PayloadModel();
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        void addPayload(Payload * payload);
        Payload *getPayload(int i);
        QStringList getColumnNames() const;
        void setColumnNames(const QStringList &value);
    public slots:
        void clear();
    private:
        static const QFont RegularFont;
        QList<Payload *> payloadList;
        QStringList columnNames;
};

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

#endif // INTERCEPSOURCE_H
