/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "byteitemmodel.h"
#include <QTextStream>
#include <QTimer>
#include <QColor>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>

int ByteItemModel::INVALID_POSITION = -1;

ByteItemModel::ByteItemModel(ByteSourceAbstract * nbyteSource,  QObject *parent) :
    QAbstractTableModel(parent)
{
    hexColumncount = 16;
    byteSource = NULL;
    setSource(nbyteSource);
    //qDebug() << "Created: " << this;
}

ByteItemModel::~ByteItemModel()
{
    byteSource = NULL;
    //qDebug() << "Destroyed: " << this;
}

void ByteItemModel::setSource(ByteSourceAbstract *nbyteSource)
{
    if (nbyteSource == NULL) {
        qCritical("ByteSource pointer null");
        return;
    }
    if (byteSource != NULL)
        disconnect(byteSource, SIGNAL(updated(quintptr)), this, SLOT(receivedSourceUpdate(quintptr)));
    beginResetModel();
    byteSource = nbyteSource;
    endResetModel();
    connect(byteSource, SIGNAL(updated(quintptr)), this, SLOT(receivedSourceUpdate(quintptr)));
}

ByteSourceAbstract *ByteItemModel::getSource() const
{
    return byteSource;
}

int ByteItemModel::size()
{
    int lsize = byteSource->viewSize();
    if (lsize < 0) {
        qWarning() << tr("Bytes source size is negative [x%1]").arg((quintptr)byteSource);
        lsize = 0;
    }

    return lsize;
}

int ByteItemModel::columnCount(const QModelIndex & parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return hexColumncount + 1;
}

int ByteItemModel::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    int viewSize = byteSource->viewSize();

    int rowCountVal = (viewSize / hexColumncount) + ((viewSize % hexColumncount) == 0 ? 0 : 1);

    if (rowCountVal < 0) {
        rowCountVal = 0;
    }
    return rowCountVal;
}

QVariant ByteItemModel::data(const QModelIndex &index, int role) const
{
    int pos = position(index);
    switch (role)
    {
        case Qt::DisplayRole:
        {
            if (index.column() == hexColumncount) {
                int datalength = hexColumncount;
                if (index.row() == rowCount() - 1) {
                    datalength = qMin(hexColumncount,byteSource->viewSize() % hexColumncount);
                    if (datalength == 0)
                        datalength = hexColumncount;
                }
                return byteSource->toPrintableString(byteSource->viewExtract(hexColumncount * index.row()
                                                                             ,datalength));
            }
            else if (pos != INVALID_POSITION)
                return QString::fromUtf8(byteSource->viewExtract(pos,1).toHex());
        }
            break;
        case Qt::BackgroundRole:
        {
            if (index.column() == hexColumncount)
                return QVariant();
            else if (pos != INVALID_POSITION){
                QColor bg = byteSource->getBgViewColor(pos);
                if (bg.isValid()) {
                   return QVariant(bg);
                } else if ((index.column() / 4) % 2 == 0)
                    return QVariant(QColor(Qt::white));
                else if ((index.column() / 4) % 2 == 1)
                    return QVariant(QColor(224,222,255,255));
                else
                    return QVariant();
            }
        }
            break;
        case Qt::ForegroundRole:
            {
        if (pos != INVALID_POSITION) {
            return QVariant(byteSource->getFgViewColor(pos));
        }
            }
            break;
        case Qt::ToolTipRole:
            {
                if (pos != INVALID_POSITION) {
                    return QVariant(byteSource->getViewToolTip(pos));
                }
            }
            break;
        case Qt::TextAlignmentRole:
        {
            return Qt::AlignCenter;
        }
        break;
    }
    return QVariant();
}

QVariant ByteItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
             return QVariant();

    if (orientation == Qt::Horizontal) {
        if (section < hexColumncount)
            return QString("%1").arg(section,2,hexColumncount,QChar('0'));
        else if (section == hexColumncount)
            return QString("Raw");
        else
            return QVariant();
    } else {
        if (section < rowCount())
            return QString("0x%1").arg((section * hexColumncount) + byteSource->startingRealOffset(),0,16);
        else
            return QVariant();
    }
}

Qt::ItemFlags ByteItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || byteSource->viewSize() <= 0)
        return Qt::ItemIsEnabled;

    if (index.column() >= hexColumncount)
        return Qt::ItemIsEnabled;

    if (hexColumncount * index.row() + index.column() + byteSource->startingRealOffset() < byteSource->size())
        return QAbstractItemModel::flags(index) | (byteSource->isReadonly() ? Qt::ItemIsEnabled : Qt::ItemIsEditable);
    else
        return Qt::ItemIsEnabled;
}

bool ByteItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        QByteArray hexVal = QByteArray::fromHex(value.toByteArray());
        if (hexVal.isEmpty())
            return false;

        qint64 pos = hexColumncount * index.row() + index.column();
        qint64 size = byteSource->viewSize();
        if (size <= 0)
            return false;

        if (pos > size) {
            qint64 temp = pos - size;
            if (temp > INT_MAX) {
                qWarning("Hitting int limit in setData");
                temp = INT_MAX;
            }

            byteSource->viewInsert(size,QByteArray(temp,0x00),(quintptr) this);
        }
        byteSource->viewReplace(pos,1, hexVal, (quintptr) this);

        emit dataChanged(index, index);
        return true;
    }
    return false;
}

void ByteItemModel::setHexColumnCount(int val)
{
    if (val > 0)
        hexColumncount = val;
    else {
        qWarning("invalid column count for ByteItemModel, ignoring");
    }
}

QModelIndex ByteItemModel::createIndex(int pos)
{
    if ( pos < 0 || pos >= byteSource->viewSize()) {
        return QAbstractTableModel::createIndex(-1, -1);
    }

    return QAbstractTableModel::createIndex(pos / hexColumncount, pos % hexColumncount);
}

QModelIndex ByteItemModel::createIndex(int row, int column) const
{
    return QAbstractTableModel::createIndex(row, column);
}

bool ByteItemModel::insert(int pos, const QByteArray &data)
{
    if (byteSource->hasCapability(ByteSourceAbstract::CAP_RESIZE) && !byteSource->isReadonly()) {
        beginResetModel();
        byteSource->viewInsert(pos,data, (quintptr) this);
        endResetModel();
        return true;
    }

    return false;
}

bool ByteItemModel::remove(int pos, int length)
{
    if (byteSource->hasCapability(ByteSourceAbstract::CAP_RESIZE) && !byteSource->isReadonly()) {
        beginResetModel();
        byteSource->viewRemove(pos,length, (quintptr) this);
        endResetModel();
        return true;
    }
    return false;
}

bool ByteItemModel::replace(int pos, int length, QByteArray val)
{
    if (!byteSource->isReadonly()) {
        if ((val.size() != length &&
             !byteSource->hasCapability(ByteSourceAbstract::CAP_RESIZE))) {
            return false;
        }
        byteSource->viewReplace(pos,length,val, (quintptr) this);
        emit dataChanged(createIndex(pos), createIndex(pos + length));
        return true;
    }
    return false;
}

QByteArray ByteItemModel::extract(int pos, int length)
{
    return byteSource->viewExtract(pos,length);
}

void ByteItemModel::clear()
{
    byteSource->clear();
}

int ByteItemModel::position(const QModelIndex &index) const
{
    if (index.isValid() && index.column() < hexColumncount && index.column() > -1 ) {
        qint64 pos = (qint64)hexColumncount * (qint64)index.row() + (qint64)index.column();
        return ((pos < byteSource->viewSize() && pos < INT_MAX) ? pos : INVALID_POSITION);
    } else {
        return INVALID_POSITION;
    }
}

void ByteItemModel::receivedSourceUpdate(quintptr viewSource)
{
    if (viewSource != (quintptr) this) {
        beginResetModel();
        endResetModel();
    }
}

bool ByteItemModel::historyForward()
{
    if (byteSource->hasCapability(ByteSourceAbstract::CAP_HISTORY)) {
        byteSource->historyForward();
        return true;
    }
    return false;
}

bool ByteItemModel::historyBackward()
{
    if (byteSource->hasCapability(ByteSourceAbstract::CAP_HISTORY)) {
        byteSource->historyBackward();
        return true;
    }
    return false;
}
