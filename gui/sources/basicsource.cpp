/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "basicsource.h"
#include <QDebug>

const QByteArray BasicSource::TEXT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678 90<>,./?#'@;:!$~%^&*()_-+=\\|{}`[]");
const QString BasicSource::LOGID("BasicSource");

BasicSource::BasicSource(QObject *parent) :
    ByteSourceAbstract(parent)
{
    currentHistoryPointer = -1;
    capabilities = 0xffffffff;
    qDebug() << "Created: " << this;
}

BasicSource::~BasicSource()
{
    qDebug() << "Destroyed: " << this;
}


void BasicSource::setData(QByteArray data, quintptr source)
{
    rawData = data;
    addToHistory(rawData);
    emit updated(source);

}

QByteArray BasicSource::getRawData()
{
    return rawData;
}

qint64 BasicSource::size()
{
    return rawData.size();
}

QByteArray BasicSource::extract(qint64 offset, int length)
{
    offset = qAbs(offset);
    if (!validateOffsetAndSize(offset, 0))
        return QByteArray();

    if (length < 0) {
        offset = (offset + length + 1);
        offset = offset < 0 ? 0 : offset;
        length = qAbs(length);
    }

    return rawData.mid(offset,length);
}

void BasicSource::replace(qint64 offset, int length, QByteArray repData, quintptr source)
{
    if (validateOffsetAndSize(offset, length)) {
        rawData.replace(offset,length,repData);
        addToHistory(rawData);
        emit updated(source);
    }
}

void BasicSource::insert(qint64 offset, QByteArray repData, quintptr source)
{
    if (validateOffsetAndSize(offset, 0)) {
        rawData.insert(offset, repData);
        addToHistory(rawData);
        emit updated(source);
    }
}

void BasicSource::remove(quint64 offset, qint64 length, quintptr source)
{
    if (validateOffsetAndSize(offset, 0)) {
        rawData.remove(offset, length);
        addToHistory(rawData);
        emit updated(source);
    }
}

void BasicSource::clear(quintptr source)
{
    rawData.clear();
    addToHistory(rawData);
    emit updated(source);
}

bool BasicSource::contains(char c)
{
    return rawData.contains(c);
}

bool BasicSource::historyForward()
{
    if (currentHistoryPointer < history.size() - 1) {
        currentHistoryPointer++;
        rawData = history.at(currentHistoryPointer);
        emit updated(INVALID_SOURCE);
        return true;
    }
    return false;
}

bool BasicSource::historyBackward()
{
    if (currentHistoryPointer > 0) {
        currentHistoryPointer--;
        rawData = history.at(currentHistoryPointer);
        emit updated(INVALID_SOURCE);
        return true;
    }
    return false;
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

qint64 BasicSource::indexOf(QByteArray item, qint64 offset)
{
    int smallOffset = 0;
    if (offset < 0 || offset > INT_MAX ) {
        offset = 0; // resetting if invalid
    } else {
        smallOffset = offset;
    }

    return rawData.indexOf(item,smallOffset);
}


bool BasicSource::validateOffsetAndSize(qint64 offset, int length)
{
    if (offset < 0) // obvious
        return false;

    if (offset > INT_MAX || offset > rawData.size()) { // hitting the limit for QByteArray, and data size
        emit log(tr("Offset too large. offset: %1 length: %2").arg(offset).arg(length),LOGID, Pip3lineConst::LERROR);
        return false;
    }

    if (length > 0 && (INT_MAX - length < offset)) { // // hitting the limit for QByteArray
            emit log(tr("Length too large, hitting the int MAX limit. offset: %1 length: %2").arg(offset).arg(length),LOGID,Pip3lineConst::LWARNING);
            return false;
    }

    // generally speaking it does not really matter if length is negative

    if (offset + length > rawData.size()) { // this is behond the end of the data
            return false;
    }

    return true;
}

void BasicSource::addToHistory(QByteArray &newData)
{
    // very very basic history mechanism
    currentHistoryPointer++;
    history = history.mid(0,currentHistoryPointer);
    history.append(newData);
}

