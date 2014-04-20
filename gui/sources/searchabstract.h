/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SEARCHABSTRACT_H
#define SEARCHABSTRACT_H

#include <QMutex>
#include <QThread>
#include <QHash>
#include <QModelIndex>
#include <QAtomicInt>
#include <commonstrings.h>
#include <QIODevice>
#include <QBitArray>
#include <QList>

class QTimerEvent;

class FoundOffsetsModel : public QAbstractListModel
{
        Q_OBJECT
    public:
        struct ItemFound {
                quint64 startOffset;
                quint64 endOffset;
        };

        explicit FoundOffsetsModel(QObject * parent = 0);
        ~FoundOffsetsModel();
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        quint64 getStartingOffset(const QModelIndex & index);
        static bool lessThanFoundOffset(ItemFound i1, ItemFound i2);
    public slots:
        void clear();
        void addOffset(quint64 soffset, quint64 endoffset);
        void addOffset(ItemFound item);
        void addOffsets(QList<ItemFound> list);
    private:
        QList<ItemFound> items;
};

class SearchAbstract : public QThread
{
        Q_OBJECT
    public:
        SearchAbstract(QObject *parent = 0);
        ~SearchAbstract();
        bool getStopAtFirst() const;
        void setStopAtFirst(bool value);
        quint64 getStartOffset() const;
        void setStartOffset(const quint64 &value);
        quint64 getEndOffset() const;
        void setEndOffset(const quint64 &value);
        QByteArray getSearchItem() const;
        void setSearchItem(const QByteArray &value, QBitArray bitmask = QBitArray());
        void setProcessStatsInternally(bool process);
    public slots:
        void startSearch();
        void startThreadedSearch();
        void stopSearch();

    protected slots:
        virtual void processStats(quint64 val);
        void onChildFinished();
        void registerSearchObject(SearchAbstract * sobj);
        bool fastSearch(QIODevice *device, qint64 startOffset, qint64 endOffset);
    signals:
        void itemFound(quint64 soffset,quint64 eoffset);
        void errorStatus(bool val);
        void log(QString mess, QString source, Pip3lineConst::LOGLEVEL level);
        void progressUpdate(quint64); // bytes processed
        void progressStatus(double percent);
        void searchStarted();
        void searchEnded();
    protected:
        virtual void internalStart() = 0;
        virtual void internalThreadedStart();
        bool singleSearch;
        quint64 soffset;
        quint64 eoffset;
        QByteArray sitem;
        char *mask;
        QMutex stopMutex;
        bool stopped;
        QHash<SearchAbstract *, quint64> threads;
        quint64 oldStats;
        quint64 statsSize;
        int threadCount;
        bool threadedSearch;
        int BufferSize;
        int statsStep;

};

#endif // SEARCHABSTRACT_H
