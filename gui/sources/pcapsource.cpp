/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/
#ifdef PCAP_SOURCE
#include "pcapsource.h"

#include <pcap.h>

const QString PcapSource::LOGID = "Pcap";

PcapSource::PcapSource(QObject *parent) :
    ByteSourceAbstract(parent)
{
    capabilities = 0;
    currentFile = 0;
}

PcapSource::~PcapSource()
{

}

QString PcapSource::description()
{
    return tr("Pcap source");
}

QByteArray PcapSource::getRawData()
{
    return currentData;
}

quint64 PcapSource::size()
{
    return currentData.size();
}

QByteArray PcapSource::extract(quint64 offset, int length)
{
    if (!validateOffsetAndSize(offset, 0))
        return QByteArray();

    if (length < 0) {
        offset = (offset + length + 1);
        length = qAbs(length);
    }

    return currentData.mid(offset,length);
}

char PcapSource::extract(quint64 offset)
{
    if (!validateOffsetAndSize(offset, 1)) {
        return '\00';
    }
    return currentData.at(offset);
}

void PcapSource::replace(quint64 ,int ,QByteArray ,quintptr )
{
    emit log(tr("Cannot edit the packet"), LOGID,Pip3lineConst::LERROR);
}

void PcapSource::insert(quint64 , QByteArray , quintptr )
{
    emit log(tr("Cannot edit the packet"), LOGID,Pip3lineConst::LERROR);
}

void PcapSource::remove(quint64 , int , quintptr )
{
    emit log(tr("Cannot edit the packet"), LOGID,Pip3lineConst::LERROR);
}

void PcapSource::clear(quintptr )
{
    emit log(tr("Cannot edit the packet"), LOGID,Pip3lineConst::LERROR);
}

bool PcapSource::historyForward()
{
    emit log(tr("History feature not implemented"), LOGID,Pip3lineConst::LERROR);
    return false;
}

bool PcapSource::historyBackward()
{
    emit log(tr("History feature not implemented"), LOGID,Pip3lineConst::LERROR);
    return false;
}

void PcapSource::fromLocalFile(QString )
{
    // todo
}

bool PcapSource::isReadableText()
{
    return false;
}

bool PcapSource::validateOffsetAndSize(quint64 offset, int length)
{
    if (offset > (quint64)currentData.size()) { // hitting the limit data size
        emit log(tr("Offset too large. offset: %1 length: %2").arg(offset).arg(length),LOGID, Pip3lineConst::LERROR);
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

    if (offset + (quint64)length > (quint64)currentData.size()) { // this is behond the end of the data
        emit log(tr("Length too large for the data set. offset: %1 length: %2").arg(offset).arg(length),LOGID,Pip3lineConst::LWARNING);
        return false;
    }

    return true;
}

#endif
