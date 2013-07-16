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
    qint64 lsize = byteSource->size();
    if (lsize > INT_MAX) {
        qCritical("Hitting int limit in size()");

        lsize = INT_MAX;
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

    qint64 rowCountVal = (byteSource->size() / hexColumncount) + ((byteSource->size() % hexColumncount) == 0 ? 0 : 1);
    if (rowCountVal > INT_MAX) {
        qCritical("Hitting int limit in rowCount()");
        rowCountVal = INT_MAX;
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
            if (index.column() == hexColumncount)
                return byteSource->toPrintableString(byteSource->extract(hexColumncount * index.row(),hexColumncount));
            else if (pos != INVALID_POSITION)
                return QString::fromUtf8(byteSource->extract(pos,1).toHex());
        }
            break;
        case Qt::BackgroundRole:
        {
            if (index.column() == hexColumncount)
                return QVariant(QColor(Qt::white));
            else if (marked.contains(pos))
                return QVariant(marked.value(pos).color);
            else if ((index.column() / 4) % 2 == 0)
                return QVariant(QColor(Qt::white));
            else if ((index.column() / 4) % 2 == 1)
                return QVariant(QColor(243,243,243,255));
            else
                return QVariant(QColor(Qt::white));
        }
            break;
        case Qt::ToolTipRole:
        {
            if (marked.contains(pos)) {
                return QVariant(marked.value(pos).text);
            }
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
            return QString("0x%1").arg(section * hexColumncount,0,16);
        else
            return QVariant();
    }
}

Qt::ItemFlags ByteItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if (index.column() >= hexColumncount)
        return Qt::ItemIsEnabled;

    if (hexColumncount * index.row() + index.column() < byteSource->size())
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
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
        qint64 size = byteSource->size();
        if (pos > size) {
            qint64 temp = pos - size;
            if (temp > INT_MAX) {
                qWarning("Hitting int limit in setData");
                temp = INT_MAX;
            }

            byteSource->insert(size,QByteArray(temp,0x00),(quintptr) this);
        }
        byteSource->replace(pos,1, hexVal, (quintptr) this);

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

QModelIndex ByteItemModel::createIndex(qint64 pos)
{
    if (pos >= size()) {
        return QAbstractTableModel::createIndex(-1, -1);
    }
    qint64 row = pos / hexColumncount;
    if (row > INT_MAX) {
        qCritical("Hitting the int limit for row");
        row = INT_MAX;
    }
    qint64 column = pos % hexColumncount;
    if (column > INT_MAX) {
        qCritical("Hitting the int limit for column");
        column = INT_MAX;
    }
    return QAbstractTableModel::createIndex(row, column);
}

QModelIndex ByteItemModel::createIndex(int row, int column) const
{
    return QAbstractTableModel::createIndex(row, column);
}

void ByteItemModel::insert(qint64 pos, const QByteArray &data)
{
    beginResetModel();
    byteSource->insert(pos,data, (quintptr) this);
    endResetModel();
}

void ByteItemModel::remove(qint64 pos, int length)
{
    beginResetModel();
    byteSource->remove(pos,length, (quintptr) this);
    endResetModel();
}

void ByteItemModel::replace(qint64 pos, int length, QByteArray val)
{
    beginResetModel();
    byteSource->replace(pos,length,val, (quintptr) this);
    endResetModel();
}

QByteArray ByteItemModel::extract(qint64 pos, int length)
{
    return byteSource->extract(pos,length);
}

void ByteItemModel::clear()
{
    byteSource->clear();
}

qint64 ByteItemModel::position(const QModelIndex &index) const
{
    if (index.isValid() && index.column() < hexColumncount && index.column() > -1 ) {
        qint64 pos = (qint64)hexColumncount * (qint64)index.row() + (qint64)index.column();
        return (pos < byteSource->size() ? pos : INVALID_POSITION);
    } else {
        return INVALID_POSITION;
    }
}

void ByteItemModel::mark(qint64 start, qint64 end, const QColor &ncolor, QString toolTip)
{
    qint64 size = byteSource->size();
    if (start >= 0 && end >= 0 && end < size && start < size) {
        qint64 temp = start;
        start = qMin(start,end);
        end = qMax(temp, end);
        for (qint64 i = start; i <= end; i++) {
            Markings ma;
            ma.color = ncolor;
            ma.text = toolTip;
            beginResetModel();
            marked.insert(i, ma);
            endResetModel();
        }
    }
}

void ByteItemModel::clearMarking(qint64 start, qint64 end)
{
    qint64 size = byteSource->size();
    if (start >= 0 && end >= 0 && end < size && start < size) {
        qint64 temp = start;
        start = qMin(start,end);
        end = qMax(temp, end);
        beginResetModel();
        for (qint64 i = start; i <= end; i++) {
            marked.remove(i);
        }
        endResetModel();
    }
}

void ByteItemModel::clearAllMarkings()
{
    beginResetModel();
    marked.clear();
    endResetModel();
}

bool ByteItemModel::hasMarking() const
{
    return !marked.isEmpty();
}

void ByteItemModel::receivedSourceUpdate(quintptr viewSource)
{
    if (viewSource != (quintptr) this) {
        beginResetModel();
        endResetModel();
    }
}

void ByteItemModel::historyForward()
{
    byteSource->historyForward();
}

void ByteItemModel::historyBackward()
{
    byteSource->historyBackward();
}
