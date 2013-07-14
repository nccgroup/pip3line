/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BYTEITEMMODEL_H
#define BYTEITEMMODEL_H

#include <QAbstractTableModel>
#include <QItemSelection>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMutex>
#include <QHash>
#include <QColor>
#include "../sources/bytesourceabstract.h"

class ByteItemModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        enum UpdateSource {TEXTVIEW = 0, HEXVIEW = 1, EXTERNAL = 2};
        static int INVALID_POSITION;
        explicit ByteItemModel(ByteSourceAbstract * byteSource, QObject *parent = 0);
        ~ByteItemModel();
        void setSource(ByteSourceAbstract * byteSource);
        ByteSourceAbstract *getSource() const;
        int size();
        int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
        void setHexColumnCount(int val);

        QModelIndex createIndex(qint64 pos);
        QModelIndex createIndex(int row, int column) const;
        void insert(qint64 pos, const QByteArray &data);
        void remove(qint64 pos, int length);
        void replace(qint64 pos, int length, QByteArray val);
        QByteArray extract(qint64 pos, int length);
        void clear();
        qint64 position(const QModelIndex &index) const;

        void mark(qint64 start, qint64 end, const QColor &color, QString toolTip = QString());
        void clearMarking(qint64 start, qint64 end);
        void clearAllMarkings();
        bool hasMarking() const;

        void historyForward();
        void historyBackward();
    signals:
        void error(QString message);
        void warning(QString message);
    public slots:
        void receivedSourceUpdate(quintptr viewSource);
    private:
        Q_DISABLE_COPY(ByteItemModel)
        int hexColumncount;
        struct Markings {
                QColor color;
                QString text;
        };

        QHash<qint64, Markings> marked;
        ByteSourceAbstract * byteSource;
};

#endif // BYTEITEMMODEL_H
