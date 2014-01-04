/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QTimerEvent>
#include <QDebug>
#include "searchabstract.h"

FoundOffsetsModel::FoundOffsetsModel(QObject *parent) :
    QAbstractListModel(parent)
{

}

FoundOffsetsModel::~FoundOffsetsModel()
{

}

int FoundOffsetsModel::rowCount(const QModelIndex &) const
{
    return offsets.size();
}

QVariant FoundOffsetsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        QList<quint64> keys = offsets.keys();
        if (role == Qt::DisplayRole) {
            return QVariant(QString::number(keys.at(index.row()),16).prepend("0x"));
        }
    }
    return QVariant();
}

QVariant FoundOffsetsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
             return QVariant();

    if (orientation == Qt::Horizontal) {
        if (section == 0)
            return QString("Offset");
    }
    return QVariant();
}

quint64 FoundOffsetsModel::getStartingOffset(const QModelIndex &index)
{
    if (index.isValid()) {
        QList<quint64> keys = offsets.keys();
        return keys.at(index.row());
    }
    return 0;
}

quint64 FoundOffsetsModel::getEndOffset(const QModelIndex &index)
{
    if (index.isValid()) {
        QList<quint64> keys = offsets.keys();
        return offsets.value(keys.at(index.row()));
    }
    return 0;
}

quint64 FoundOffsetsModel::getEndOffset(const quint64 startOffset)
{
    return offsets.value(startOffset,0);
}

void FoundOffsetsModel::clear()
{
    if (!offsets.isEmpty()) {
        beginRemoveRows(QModelIndex(),0,offsets.size() - 1);
        offsets.clear();
        endRemoveRows();
    }
}

void FoundOffsetsModel::addOffset(quint64 soffset, quint64 endoffset)
{
    beginResetModel();
    offsets.insert(soffset,endoffset);
    endResetModel();
}

SearchAbstract::SearchAbstract(QObject *parent):
    QThread(parent)
{
    singleSearch = false;
    soffset = 0;
    eoffset = 0;
    oldStats = 0;
    statsSize = 0;
    stopped = true;
    threadCount = QThread::idealThreadCount();
    timer = 0;
    processStatsInternally = true;
    threadedSearch = false;
    //qDebug() << this << "created";
}

SearchAbstract::~SearchAbstract()
{
    //qDebug() << this << "Destroying" << isRunning();
    if (isRunning()) {
        stopSearch();
        quit();
        if (!wait(10000)) {
            qCritical() << "Could not stop the SearchAbstract thread" << this;
        }
    }
}

void SearchAbstract::initialize()
{
    moveToThread(this);
    this->start();
}

void SearchAbstract::processStats(quint64 val)
{
    SearchAbstract *sob = static_cast<SearchAbstract *>(sender());
    if (threads.contains(sob)) {
        threads.insert(sob,val);
    }
}

void SearchAbstract::onChildFinished()
{
    SearchAbstract *sob = static_cast<SearchAbstract *>(sender());
    if (threads.contains(sob)) {
        oldStats += threads.value(sob);
        threads.remove(sob);
        sob->quit();
        if (!sob->wait(10000)) {
            qCritical() << tr("[ThreadedSearch] could not stop the child thread") << sob;
        }
        sob->deleteLater();
    } else {
        qCritical() << tr("[ThreadedSearch] Unkown thread") << sob;
    }
    if (threads.isEmpty()) {
        oldStats = 0;
        killTimer(timer);
        timer = 0;
        emit searchEnded();
    }
}

void SearchAbstract::registerSearchObject(SearchAbstract *sobj)
{
    threads.insert(sobj,0);
    sobj->setStopAtFirst(false);
    sobj->setSearchItem(sitem);
    sobj->setProcessStatsInternally(false);
    sobj->moveToThread(sobj);
    sobj->start();
    connect(sobj, SIGNAL(searchEnded()), SLOT(onChildFinished()),Qt::QueuedConnection);
    connect(sobj,SIGNAL(progressUpdate(quint64)), SLOT(processStats(quint64)),Qt::QueuedConnection);
    connect(sobj,SIGNAL(itemFound(quint64,quint64)), SIGNAL(itemFound(quint64,quint64)),Qt::QueuedConnection);
}

void SearchAbstract::timerEvent(QTimerEvent *event)
{
    if (statsSize > 0) {
        quint64 aggregatestats = oldStats;
        foreach (quint64 value, threads)
            aggregatestats += value;
        emit progressStatus((double)aggregatestats / (double)statsSize);
        event->accept();
    }
}

void SearchAbstract::internalThreadedStart()
{
    qWarning() << "[" << metaObject()->className() << "] Multi-threaded search not implemented, falling back to single threaded";
    threadedSearch = false;
    timer = 0;
    internalStart();
    emit searchEnded();
}

void SearchAbstract::startSearch()
{
    if (sitem.isEmpty())
        return;
    threadedSearch = false;
    stopMutex.lock();
    stopped = false;
    stopMutex.unlock();
    emit searchStarted();
    internalStart();
    emit searchEnded();
}

void SearchAbstract::startThreadedSearch()
{
    if (sitem.isEmpty())
        return;
    threadedSearch = true;
    singleSearch = false;
    emit searchStarted();
    oldStats = 0;
    internalThreadedStart();
}

void SearchAbstract::stopSearch()
{
    if (threadedSearch) {
        QList<SearchAbstract *> list =  threads.keys();
        for (int i = 0; i < list.size(); ++i) {
             list.at(i)->stopSearch();
         }
    } else {
        stopMutex.lock();
        stopped = true;
        stopMutex.unlock();
    }
 //   qDebug()  << "SearchAbstract stopped requested";
}

bool SearchAbstract::getProcessStatsInternally() const
{
    return processStatsInternally;
}

void SearchAbstract::setProcessStatsInternally(bool value)
{
    processStatsInternally = value;
}

QByteArray SearchAbstract::getSearchItem() const
{
    return sitem;
}

void SearchAbstract::setSearchItem(const QByteArray &value)
{
    sitem = value;
}

quint64 SearchAbstract::getEndOffset() const
{
    return eoffset;
}

void SearchAbstract::setEndOffset(const quint64 &value)
{
    eoffset = value;
}

quint64 SearchAbstract::getStartOffset() const
{
    return soffset;
}

void SearchAbstract::setStartOffset(const quint64 &value)
{
    soffset = value;
}

bool SearchAbstract::getStopAtFirst() const
{
    return singleSearch;
}

void SearchAbstract::setStopAtFirst(bool value)
{
    singleSearch = value;
}
