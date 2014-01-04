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

class QTimerEvent;

class FoundOffsetsModel : public QAbstractListModel
{
        Q_OBJECT
    public:
        explicit FoundOffsetsModel(QObject * parent = 0);
        ~FoundOffsetsModel();
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        quint64 getStartingOffset(const QModelIndex & index);
        quint64 getEndOffset(const QModelIndex & index);
        quint64 getEndOffset(const quint64 startOffset);
    public slots:
        void clear();
        void addOffset(quint64 soffset, quint64 endoffset);
    private:
        QMap<quint64, quint64> offsets;
};

class SearchAbstract : public QThread
{
        Q_OBJECT
    public:
        SearchAbstract(QObject *parent = 0);
        ~SearchAbstract();
        virtual void initialize();

        bool getStopAtFirst() const;
        void setStopAtFirst(bool value);

        quint64 getStartOffset() const;
        void setStartOffset(const quint64 &value);

        quint64 getEndOffset() const;
        void setEndOffset(const quint64 &value);

        QByteArray getSearchItem() const;
        void setSearchItem(const QByteArray &value);

        bool getProcessStatsInternally() const;
        void setProcessStatsInternally(bool value);

    public slots:
        void startSearch();
        void startThreadedSearch();
        void stopSearch();
    protected slots:
        virtual void processStats(quint64 val);
        void onChildFinished();
        void registerSearchObject(SearchAbstract * sobj);
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
        void timerEvent(QTimerEvent *event);
        bool singleSearch;
        quint64 soffset;
        quint64 eoffset;
        QByteArray sitem;
        QMutex stopMutex;
        bool stopped;
        bool processStatsInternally;
        QHash<SearchAbstract *, quint64> threads;
        quint64 oldStats;
        quint64 statsSize;
        int timer;
        int threadCount;
        bool threadedSearch;
};

#endif // SEARCHABSTRACT_H
