/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "payloadmodel.h"
#include <QIcon>
#include <QDebug>

const int PayloadModel::DIRECTION_COLUMN = 0;
const int PayloadModel::TIMESPTAMP_COLUMN = 1;
const int PayloadModel::PAYLOAD_COLUMN = 2;

PayloadModel::PayloadModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    columnNames << "D" << "Timestamp" << "Payload";
}

PayloadModel::~PayloadModel()
{
    clear();
}

int PayloadModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return payloadList.size();
}

int PayloadModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return columnNames.size();
}

QVariant PayloadModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int column = index.column();
    Payload * payl = payloadList.at(row);
    if (role == Qt::DisplayRole) {
        if ( column == TIMESPTAMP_COLUMN)
            return QVariant(payl->getTimestamp());
        else if (column == PAYLOAD_COLUMN)
            return QVariant(payl->getPayload().toHex());
        else
            return QVariant();
    } else if (role == Qt::DecorationRole) {
        if (column == DIRECTION_COLUMN) {
            switch (payl->getDirection()) {
                case Payload::LEFTRIGHT:
                    return QVariant(QIcon(":/Images/icons/arrow-right-3-mod.png"));
                case Payload::RIGHTLEFT:
                    return QVariant(QIcon(":/Images/icons/arrow-left-3.png"));
                default:
                    return QVariant();
            }
        }
    }else if (role == Qt::BackgroundRole) {
        return (payl->hasBeenModified() ? QVariant() : QVariant(QColor(218,160,255,255)));
    } else if (role == Qt::FontRole) {
        return QVariant(GuiStyles::DEFAULT_REGULAR_FONT);
    }

    return QVariant();
}

QVariant PayloadModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant empty;
    if (role != Qt::DisplayRole)
             return empty;

    if (orientation == Qt::Horizontal) {
        return section < columnNames.size() ? QVariant(columnNames.at(section)) : empty;
    } else {
        return  QVariant(section);
    }
}

void PayloadModel::addPayload(Payload *payload)
{
    if (payload == NULL) {
        qCritical() << "[PayloadModel::addPayload] NULL pointer, ignoring";
    } else {
        int index = payloadList.size();
        beginInsertRows(QModelIndex(),index,index);
        payloadList.append(payload);
        endInsertRows();
    }
}

Payload *PayloadModel::getPayload(int i)
{
    return payloadList.at(i);
}

void PayloadModel::clear()
{
    beginResetModel();
    while (!payloadList.isEmpty())
        delete payloadList.takeFirst();
    endResetModel();
}

QStringList PayloadModel::getColumnNames() const
{
    return columnNames;
}

void PayloadModel::setColumnNames(const QStringList &value)
{
    columnNames = value;
}
