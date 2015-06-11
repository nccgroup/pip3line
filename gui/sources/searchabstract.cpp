/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QTimerEvent>
#include <QDebug>
#include <QApplication>
#if QT_VERSION >= 0x050000
#include <QtConcurrent>
#endif
#include <QtConcurrentRun>
#include "searchabstract.h"
#include "bytesourceabstract.h"

int SourceReader::MAX_READ_SIZE = 0x4000; // 16KB, this should be more than enough
SourceReader::SourceReader()
{

}

SourceReader::~SourceReader()
{

}

QColor SearchAbstract::SEARCH_COLOR = Qt::yellow;

SearchWorker::SearchWorker(SourceReader * device,QObject *parent)
    : QObject(parent)
{
    startOffset = 0;
    endOffset = 0;
    BufferSize = 4096;
    statsStep = 4096;
    cancelled = true;
    searchMask = NULL;
    listSend = false;
    searchSize = 0;
    hasError = false;
    rawitem = NULL;
    targetdevice = device; // no check on the validity of the device itself this has to be done by the caller. The object take ownership as well.
    foundList = new (std::nothrow) BytesRangeList();
    if (foundList == NULL) {
        qFatal("Cannot allocate memory for BytesRangeList X{");
    }
}

SearchWorker::~SearchWorker()
{
    if (listSend) // no need to delete the list if it was taken by someone
        foundList = NULL;
    else { // unless the list was never requested
        while (!foundList->isEmpty())
            delete foundList->takeFirst();
        delete foundList;
    }
    searchMask = NULL;
    delete targetdevice; // we have ownership
    targetdevice = NULL;
}

void SearchWorker::cancel()
{
    cancelled = true;
}

void SearchWorker::search()
{
    cancelled = false;
    quint64 nextStat = startOffset + statsStep;
    int len = searchSize -1;
    quint64 curOffset = startOffset;

    if (rawitem == NULL) {
        qCritical() << "rawitem is null at the beginning of search() T_T";
        emit finished();
        return;
    }

    if (startOffset == endOffset) {
        emit finished();
        return;
    }

    if (!targetdevice->isReadable()) {
        emit log(tr("Could not open the device, aborting search"),this->metaObject()->className(), Pip3lineConst::LERROR);
        hasError = true;
        emit finished();
        return;
    }

    if (!targetdevice->seek(curOffset)) {
        emit log(tr("Error while seeking, aborting search"),this->metaObject()->className(), Pip3lineConst::LERROR);
        hasError = true;
        emit finished();
        return;
    }

    char * readbuffer = new(std::nothrow) char[BufferSize];
    if (readbuffer == NULL) {
        qFatal("Cannot allocate memory for the find buffer X{");
        emit finished();
        return;
    }

    while (true)
    {
        int bRead;
        memmove(readbuffer, readbuffer + BufferSize - len, len);
        bRead = targetdevice->read(readbuffer + len, (quint64) (BufferSize - len));

        if (bRead < 0) {
            emit log(tr("Error while reading, aborting search"),this->metaObject()->className(), Pip3lineConst::LERROR);
            hasError = true;
            delete[] readbuffer;
            readbuffer = NULL;
            emit finished();
            return;
        } else if (bRead == 0)  {// end of read for some reasons ...
            qWarning() << "[SearchWorker::search] zero byte read T_T" << this;
            delete[] readbuffer;
            readbuffer = NULL;
            emit finished();
            return;
        }

        int off, i;
        for (off = (curOffset == 0) ? 0 : len; off < bRead; ++off)
        {
            quint64 realOffset = curOffset + (quint64)off;
            for (i = 0; i <= len; ++i)
                if ((readbuffer[off + i] & searchMask[i]) != rawitem[i])
                        break;
            if (i > len)
            {
                BytesRange * br = new(std::nothrow) BytesRange(realOffset - (quint64)len, realOffset);
                br->setBackground(SearchAbstract::SEARCH_COLOR);
                if (br == NULL) {
                    qFatal("Cannot allocate memory for BytesRange X{");
                    return;
                }
                foundList->append(br);
            }
            // updating stats
            if (realOffset > nextStat) {
                emit progressUpdate(realOffset - startOffset);
                nextStat += statsStep;
            }

            if (realOffset >= endOffset)  {// end of the travel
                delete[] readbuffer;
                readbuffer = NULL;
                emit finished();
                return;
            }
        }

        curOffset += (quint64)bRead;
        if (cancelled) {
            delete[] readbuffer;
            readbuffer = NULL;
            emit finished();
            return;
        }
    }
}

void SearchWorker::setStatsStep(int value)
{
    statsStep = value;
}


bool SearchWorker::getHasError() const
{
    return hasError;
}

BytesRangeList *SearchWorker::getFoundList()
{
    listSend = true;
    return foundList;
}

quint64 SearchWorker::getEndOffset() const
{
    return endOffset;
}

void SearchWorker::setEndOffset(const quint64 &value)
{
    endOffset = value;
}

void SearchWorker::setSearchItem(QByteArray &sitem, char *mask)
{
    rawitem = sitem.data();
    searchSize = sitem.size();
    searchMask = mask; // again no checks here, the caller is reponsible for that
}

quint64 SearchWorker::getStartOffset() const
{
    return startOffset;
}

void SearchWorker::setStartOffset(const quint64 &value)
{
    startOffset = value;
}

const int SearchAbstract::MAX_SEARCH_ITEM_SIZE = 1024;

SearchAbstract::SearchAbstract():
    QObject(NULL)
{
    cursorOffset = 0;
    jumpToNext = true;
    oldStats = 0;
    totalSearchSize = 0;
    stopped = true;
    hasError = false;
    statsStep = 4096; // by default report every 4k bytes
    mask = NULL;
    globalFoundList = NULL;
    moveToThread(&eventThread);
    eventThread.start();

  //  qDebug() << this << "created" << "Thread: [" << thread() << "]";
}

SearchAbstract::~SearchAbstract()
{
  //  qDebug() << this << "Destroying";
    eventThread.quit();
    if (!eventThread.wait(10000)) {
            qCritical() << "Could not stop the SearchAbstract thread" << this;
    }

    if (mask != NULL) {
        delete[] mask;
    }
    globalFoundList = NULL; // never touch that
}

void SearchAbstract::processStats(quint64 val)
{
    if (stopped) // ignoring if the search has already ended
        return;

    SearchWorker *sob = static_cast<SearchWorker *>(sender());
    if (totalSearchSize > 0 && workers.contains(sob)) {
        workers.insert(sob,val);
        quint64 aggregatestats = oldStats;
        foreach (quint64 value, workers)
            aggregatestats += value;
        double newstats = (double)aggregatestats / (double)totalSearchSize;
        emit progressStatus(newstats);
    }
}
bool SearchAbstract::getJumpToNext() const
{
    return jumpToNext;
}

void SearchAbstract::setJumpToNext(bool value)
{
    jumpToNext = value;
}

quint64 SearchAbstract::getCursorOffset() const
{
    return cursorOffset;
}

void SearchAbstract::setCursorOffset(const quint64 &value)
{
    cursorOffset = value;
}


void SearchAbstract::onChildFinished()
{
    SearchWorker *worker = dynamic_cast<SearchWorker *>(sender());
    if (workers.contains(worker)) {
        oldStats += workers.value(worker);
        hasError = hasError || worker->getHasError();
        BytesRangeList *tempList = worker->getFoundList();
        if (stopped) { // no need to process anything just ignore

            while (!tempList->isEmpty())
                delete tempList->takeFirst();
        } else {
            globalFoundList->append(*tempList); // range objects are not owned by the global list
        }
        delete tempList; // don't need to free the range objects in any case
        workers.remove(worker);

    } else {
        qCritical() << tr("[ThreadedSearch] Unknown worker T_T") << worker;
    }
    delete worker;
    if (workers.isEmpty()) {
        oldStats = 0;
        if (globalFoundList->isEmpty())
            hasError = true;
        else {
            globalFoundList->unify();
            globalFoundList->moveToThread(QApplication::instance()->thread());// dirty, there must be a better way
            if (jumpToNext) {
                BytesRange * range = globalFoundList->at(0); // if not found take the first
                int size = globalFoundList->size();
                for(int i = 0 ; i < size; i++) {
                    if (cursorOffset < globalFoundList->at(i)->getLowerVal()) {
                        range = globalFoundList->at(i);
                        break;
                    }
                }
                emit jumpRequest(range->getLowerVal(), range->getUpperVal());
            }
            emit foundList(globalFoundList);
        }
        emit searchEnded();
        emit errorStatus(hasError);
        hasError = false; // resetting error status
    }
}

void SearchAbstract::addSearchWorker(SearchWorker *worker)
{
    workers.insert(worker,0);
    worker->setSearchItem(sitem,mask);
    connect(worker, SIGNAL(finished()), SLOT(onChildFinished()),Qt::QueuedConnection);
    connect(worker,SIGNAL(progressUpdate(quint64)), SLOT(processStats(quint64)),Qt::QueuedConnection);
    connect(worker,SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)));
    QtConcurrent::run(worker, &SearchWorker::search);
}
BytesRangeList *SearchAbstract::getGlobalFoundList()
{
    return globalFoundList;
}

void SearchAbstract::startSearch()
{
    if (sitem.isEmpty())
        return;
    globalFoundList = new(std::nothrow)BytesRangeList(NULL);
    if (globalFoundList == NULL) {
        qFatal("Cannot allocate memory for BytesRangeList X{");
        return;
    }
    stopped = false;
    oldStats = 0;
    emit searchStarted();
    internalStart();
}

void SearchAbstract::stopSearch()
{
    stopped = true;
    QList<SearchWorker *> list =  workers.keys();
    for (int i = 0; i < list.size(); ++i) {
         list.at(i)->cancel();
    }
}

void SearchAbstract::setSearchItem(const QByteArray &value, QBitArray bitmask)
{

    if (value.size() > MAX_SEARCH_ITEM_SIZE) { // limiting the size of the search to 1k
        sitem = value.mid(0,MAX_SEARCH_ITEM_SIZE);
        emit log(tr("The searched item is larger than %1 bytes, truncating it").arg(MAX_SEARCH_ITEM_SIZE),metaObject()->className(), Pip3lineConst::LERROR);
    } else {
        sitem = value;
    }

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
