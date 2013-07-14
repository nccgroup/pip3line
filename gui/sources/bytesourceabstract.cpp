/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QFile>
#include "bytesourceabstract.h"

const quintptr ByteSourceAbstract::INVALID_SOURCE = 0;

ByteSourceAbstract::ByteSourceAbstract(QObject *parent) :
    QObject(parent)
{
    capabilities = 0;
}

ByteSourceAbstract::~ByteSourceAbstract()
{
}

void ByteSourceAbstract::setData(QByteArray, quintptr )
{
}

QByteArray ByteSourceAbstract::getRawData()
{
    return QByteArray();
}

qint64 ByteSourceAbstract::size()
{
    return 0;
}

void ByteSourceAbstract::replace(qint64 , int , QByteArray , quintptr )
{
}

void ByteSourceAbstract::insert(qint64 , QByteArray , quintptr )
{
}

void ByteSourceAbstract::remove(quint64 , qint64 , quintptr )
{
}

void ByteSourceAbstract::clear(quintptr)
{
}

bool ByteSourceAbstract::contains(char)
{
    return false;
}

bool ByteSourceAbstract::historyForward()
{
    return false;
}

bool ByteSourceAbstract::historyBackward()
{
    return false;
}

bool ByteSourceAbstract::hasCapability(ByteSourceAbstract::CAPABILITIES cap)
{
    return (capabilities & cap);
}

bool ByteSourceAbstract::isReadableText()
{
    return false;
}

qint64 ByteSourceAbstract::indexOf(QByteArray , qint64 )
{
    return -1;
}

QString ByteSourceAbstract::toPrintableString(const QByteArray &val)
{
    QString line;
    for (int i = 0; i < val.size(); i++) {
        line.append((val.at(i) > 32 && val.at(i) < 127) ? val.at(i) : '.');
    }
    return line;
}

void ByteSourceAbstract::fromLocalFile(QString fileName)
{
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            emit log(tr("Failed to open %1:\n %2").arg(fileName).arg(file.errorString()),"ByteSourceAbstract",Pip3lineConst::LERROR);
            return;
        }

        setData(file.readAll());
    }
}
