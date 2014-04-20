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
    return items.size();
}

QVariant FoundOffsetsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            return QVariant(QString::number(items.at(index.row()).startOffset,16).prepend("0x"));
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
        return items.at(index.row()).startOffset;
    }
    return 0;
}

bool FoundOffsetsModel::lessThanFoundOffset(FoundOffsetsModel::ItemFound i1, FoundOffsetsModel::ItemFound i2)
{
    return i1.startOffset < i2.startOffset;
}

void FoundOffsetsModel::clear()
{
    if (!items.isEmpty()) {
        beginRemoveRows(QModelIndex(),0,items.size() - 1);
        items.clear();
        endRemoveRows();
    }
}

void FoundOffsetsModel::addOffset(quint64 soffset, quint64 endoffset)
{
    ItemFound item;
    item.startOffset = soffset;
    item.endOffset = endoffset;
    addOffset(item);
}

void FoundOffsetsModel::addOffset(FoundOffsetsModel::ItemFound item)
{
    beginResetModel();
    items.append(item);
    qSort(items.begin(),items.end(),FoundOffsetsModel::lessThanFoundOffset);
    endResetModel();
}

void FoundOffsetsModel::addOffsets(QList<FoundOffsetsModel::ItemFound> list)
{
    beginResetModel();
    items.append(list);
    qSort(items.begin(),items.end(),FoundOffsetsModel::lessThanFoundOffset);
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
    threadedSearch = false;
    BufferSize = 4096; // for now, not sure if it is worth being tweakable
    statsStep = 4096; // by default report every 4k bytes
    mask = NULL;
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

    if (mask != NULL) {
        delete[] mask;
    }
}

void SearchAbstract::processStats(quint64 val)
{
    if (stopped) // ignoring if the search has already ended
        return;
    SearchAbstract *sob = static_cast<SearchAbstract *>(sender());
    if (statsSize > 0 && threads.contains(sob)) {
        threads.insert(sob,val);
        quint64 aggregatestats = oldStats;
        foreach (quint64 value, threads)
            aggregatestats += value;
        double newstats = (double)aggregatestats / (double)statsSize;
        emit progressStatus(newstats);
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

void SearchAbstract::internalThreadedStart()
{
    qWarning() << "[" << metaObject()->className() << "] Multi-threaded search not implemented, falling back to single threaded";
    threadedSearch = false;
    internalStart();
    emit searchEnded();
}

void SearchAbstract::startSearch()
{
    if (sitem.isEmpty())
        return;

    threads.insert(this,0);
    threadedSearch = false;
    stopMutex.lock();
    stopped = false;
    stopMutex.unlock();
    emit searchStarted();
    oldStats = 0;
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

bool SearchAbstract::fastSearch(QIODevice *device, qint64 startOffset, qint64 endOffset)
{
    bool found = false;
    if (startOffset == endOffset)
        return true;

    qint64 cursorTravel= 0;
    qint64 nextStat = statsStep;
    int len = sitem.size() -1;
    char * value = sitem.data();
    qint64 curOffset = startOffset;
    char * readbuffer = new(std::nothrow) char[BufferSize];
    if (readbuffer == NULL) {
        qFatal("Cannot allocate memory for the find buffer X{");
        return false;
    }

    if (!device->seek(curOffset)) {
        emit log(tr("Error while seeking: %1").arg(device->errorString()),this->metaObject()->className(), Pip3lineConst::LERROR);
        return found;
    }

    while (true)
    {
        qint64 bRead;
        memmove(readbuffer, readbuffer + BufferSize - len, len);
        bRead = device->read(readbuffer + len, BufferSize - len);

        if (bRead < 0)
        {
            emit log(tr("Error while reading the file: %1").arg(device->errorString()),this->metaObject()->className(), Pip3lineConst::LERROR);
            return found;
        } else if (bRead == 0) // end of file ... hopefully
            return found;

        int off, i;
        for (off = curOffset ? 0 : len; off < bRead; ++off)
        {
            for (i = 0; i <= len; ++i)
                if ((readbuffer[off + i] & mask[i]) != value[i])
                        break;
            if (i > len)
            {
                emit itemFound(curOffset + (quint64)off - (quint64)len,curOffset + (quint64)off);
             //   qDebug() << "found" << curOffset;
                if (!found) {
                    if (singleSearch)
                        return true;
                    else
                        found = true;
                }
            }
            // updating stats
            cursorTravel++;
            if (cursorTravel > nextStat) {
                emit progressUpdate((quint64)cursorTravel);
                nextStat += statsStep;
            }

            if (startOffset + cursorTravel > endOffset) // end of the travel
                return found;
        }

        curOffset += bRead;
        stopMutex.lock();
        if (stopped) {
            stopMutex.unlock();
            break;
        }
        stopMutex.unlock();
    }

    return found;
}

void SearchAbstract::setProcessStatsInternally(bool process)
{
    if (process) {
        connect(this, SIGNAL(progressUpdate(quint64)), SLOT(processStats(quint64)), Qt::UniqueConnection);
    } else {
        disconnect(this, SIGNAL(progressUpdate(quint64)), this, SLOT(processStats(quint64)));
    }
}

QByteArray SearchAbstract::getSearchItem() const
{
    return sitem;
}

void SearchAbstract::setSearchItem(const QByteArray &value, QBitArray bitmask)
{
    sitem = value;
    int searchSize = sitem.size();
    if (bitmask.size() < searchSize) {
        int index = bitmask.isEmpty() ? 0 : bitmask.size() - 1;
        bitmask.resize(searchSize);
        for (; index < searchSize; index++)
            bitmask.setBit(index);
    }
    if (mask != NULL)
        delete[] mask;

    mask = new(std::nothrow) char[searchSize];

    if (mask == NULL) {
        qFatal("Cannot allocate memory for the mask X{");
        return;
    }

    for (int i = 0; i < searchSize;i++) {
        mask[i] = bitmask.testBit(i) ? '\xFF' : '\x00';
    }
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
