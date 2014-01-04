/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "basicsource.h"
#include <QDebug>
#include <QTime>

BasicSearch::BasicSearch(QByteArray *data, QObject *parent) : SearchAbstract(parent)
{
    sdata = data;
}

BasicSearch::~BasicSearch()
{

}

void BasicSearch::internalStart()
{
    if (sdata == NULL) {
         emit log(tr("Null search data"), this->metaObject()->className(),Pip3lineConst::LERROR);
        return;
    }
    int smallOffset = 0;

    if (soffset > INT_MAX || soffset > (quint64)sdata->size()) {
        emit log(tr("Offset too large"), this->metaObject()->className(),Pip3lineConst::LERROR);
        emit errorStatus(true);
        return;
    } else {
        smallOffset = (int)soffset;
    }

    qDebug() << "Searching " << soffset;
    int ret = -1;
    int soffset = smallOffset;
    ret = sdata->indexOf(sitem,smallOffset);
    bool foundItem = false;
    if (ret >= 0) {
        emit itemFound(quint64(ret),quint64(ret + sitem.size() - 1));
        foundItem = true;
    } else if (soffset > 0) {
        smallOffset = 0;
    }

    if (!(foundItem && singleSearch)) {
        bool loopNotDone = true;
        while (true) {
            ret = sdata->indexOf(sitem,smallOffset);
            if (ret < 0) { // not found
                if (smallOffset == soffset || soffset == 0) // if we are at the start of the block of data end the search
                    break;
                else if (loopNotDone) {
                    // if we haven't started from the beginning and current offset is different from starting offset
                    // then we go back to beginning and continue the search (setting the loop flag to avoid infinite loop)
                    smallOffset = 0;
                    loopNotDone = false;
                }
                else // default end the loop
                    break;
            } else {
                emit itemFound(quint64(ret),quint64(ret + sitem.size() - 1));
                foundItem = true;
                if (singleSearch)
                    break;
                else
                    smallOffset = ++ret;
            }
        }
    }

   emit errorStatus(!foundItem);

}

const QByteArray BasicSource::TEXT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678 90<>,./?#'@;:!$~%^&*()_-+=\\|{}`[]");
const QString BasicSource::LOGID("BasicSource");

BasicSource::BasicSource(QObject *parent) :
    ByteSourceAbstract(parent)
{
    capabilities = CAP_RESET | CAP_RESIZE | CAP_HISTORY | CAP_TRANSFORM | CAP_LOADFILE | CAP_SEARCH | CAP_WRITE | CAP_COMPARE;
   // qDebug() << "Created: " << this;
}

BasicSource::~BasicSource()
{
   // qDebug() << "Destroyed: " << this;
}

QString BasicSource::description()
{
    return name();
}

QString BasicSource::name()
{
    return tr("Basic source");
}


void BasicSource::setData(QByteArray data, quintptr source)
{
    if (!_readonly) {
        historyAddReplace(0,rawData,data);
        rawData = data;
        emit updated(source);
        emit sizeChanged();
    }

}

QByteArray BasicSource::getRawData()
{
    return rawData;
}

quint64 BasicSource::size()
{
    return rawData.size();
}

QByteArray BasicSource::extract(quint64 offset, int length)
{
    if (!validateOffsetAndSize(offset, 0))
        return QByteArray();

    if (length < 0) {
        offset = (offset + length + 1);
        length = qAbs(length);
    }

    return rawData.mid(offset,length);
}

char BasicSource::extract(quint64 offset)
{
    if (!validateOffsetAndSize(offset, 1)) {
        return '\00';
    }
    return rawData.at(offset);
}

void BasicSource::replace(quint64 offset, int length, QByteArray repData, quintptr source)
{

    if (!_readonly && validateOffsetAndSize(offset, length)) {
        historyAddReplace(offset, rawData.mid(offset,length),repData);
        rawData.replace(offset,length,repData);
        emit updated(source);
        emit sizeChanged();
    }
}

void BasicSource::insert(quint64 offset, QByteArray repData, quintptr source)
{
    if (!_readonly && validateOffsetAndSize(offset, 0)) {
        historyAddInsert(offset,repData);
        rawData.insert(offset, repData);
        emit updated(source);
        emit sizeChanged();
    }
}

void BasicSource::remove(quint64 offset, int length, quintptr source)
{
    if (!_readonly && validateOffsetAndSize(offset, 0)) {
        historyAddRemove(offset,rawData.mid(offset,length));
        rawData.remove(offset, length);
        emit updated(source);
        emit sizeChanged();
    }
}

void BasicSource::clear(quintptr source)
{
    if (!_readonly) {
        historyAddRemove(0,rawData);
        rawData.clear();
        emit updated(source);
        emit sizeChanged();
    }
}

int BasicSource::getViewOffset(quint64 realoffset)
{
    if (realoffset > (quint64) rawData.size()) {
        emit log(tr("Offset too large: %1").arg(realoffset),LOGID, Pip3lineConst::LERROR);
        return - 1;
    }
    return (int)realoffset;
}

int BasicSource::preferredTabType()
{
    return TAB_TRANSFORM;
}

bool BasicSource::isOffsetValid(quint64 offset)
{
    return offset < ((quint64)rawData.size());
}

bool BasicSource::isReadableText()
{
    if (rawData.contains('\x00'))
        return false;
    int count = 0;

    for (int i=0; i < rawData.size(); i++) {
        if (TEXT.contains(rawData.at(i)))
            count++;
    }
    if ((float)(count)/rawData.size() < 0.7) {
        return false;
    }
    return true;
}

SearchAbstract *BasicSource::requestSearchObject(QObject *parent)
{
    BasicSearch *sobj = new(std::nothrow) BasicSearch(&rawData,parent);
    if (sobj == NULL) {
        qFatal("Cannot allocate memory for BasicSearch X{");
    }
    return sobj;
}

bool BasicSource::validateOffsetAndSize(quint64 offset, int length)
{
    if (offset > (quint64)rawData.size()) { // hitting the limit data size
        emit log(tr("Offset too large: %1 length: %2").arg(offset).arg(length),LOGID, Pip3lineConst::LERROR);
        return false;
    }

    if (length < 0 ) { // trivial
        emit log(tr("Negative length: %2").arg(length),LOGID, Pip3lineConst::LERROR);
        return false;
    }

    if ((quint64)(INT_MAX - length) < offset) { // // hitting the limit
        emit log(tr("Length too large, hitting the int MAX limit. offset: %1 length: %2").arg(offset).arg(length),LOGID,Pip3lineConst::LWARNING);
        return false;
    }

    if (offset + (quint64)length > (quint64)rawData.size()) { // this is behond the end of the data
        emit log(tr("Length too large for the data set. offset: %1 length: %2").arg(offset).arg(length),LOGID,Pip3lineConst::LWARNING);
        return false;
    }

    return true;
}
