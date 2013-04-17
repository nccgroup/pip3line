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

class ByteItemModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        enum UpdateSource {TEXTVIEW = 0, HEXVIEW = 1, EXTERNAL = 2};
        static int INVALID_POSITION;
        explicit ByteItemModel(QObject *parent = 0);
        ~ByteItemModel();
        void setRawData(const QByteArray &data, UpdateSource source = ByteItemModel::EXTERNAL);
        int size() const;
        QByteArray getRawData();
        int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

        int getColumnNumbers() const;
        QModelIndex createIndex(int pos) const;
        void insert(int pos, const QByteArray &data);
        void remove(int pos, int length);
        void replace(int pos, int length, QByteArray val);
        QByteArray extract(int pos, int length);
        void clear();
        int position(const QModelIndex &index) const;

        bool isStringValid();
        bool isReadableText();

        void mark(int start, int end, const QColor &color, QString toolTip = QString());
        void clearMarking(int start, int end);
        void clearAllMarkings();
        bool hasMarking() const;

        void historyForward();
        void historyBackward();
    signals:
        void error(QString message);
        void warning(QString message);
        void updatedFrom(ByteItemModel::UpdateSource);
    public slots:
        void fromLocalFile(QString filename);
    private:
        static const QByteArray TEXT;
        inline QString toPrintableString(const QByteArray &val) const;
        inline void notifyUpdate(UpdateSource source = ByteItemModel::EXTERNAL);
        inline void addDataToHistory(const QByteArray &data);
        QByteArray rawData;
        int columnNumbers;
        QMutex rawDataMutex;
        struct Markings {
                QColor color;
                QString text;
        };

        QHash<int, Markings> marked;
        QList<QByteArray> history;
        int currentHistoryPointer;
};

#endif // BYTEITEMMODEL_H
