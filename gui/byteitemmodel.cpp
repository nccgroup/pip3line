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
const QByteArray ByteItemModel::TEXT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678 90<>,./?#'@;:!$~%^&*()_-+=\\|{}`[]");

ByteItemModel::ByteItemModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    columnNumbers = 16;
    currentHistoryPointer = -1;
    qDebug() << "Created: " << this;
}

ByteItemModel::~ByteItemModel()
{
    qDebug() << "Destroyed: " << this;
}

void ByteItemModel::setRawData(const QByteArray &data, UpdateSource source)
{
    beginResetModel();
    rawDataMutex.lock();
    rawData = data;
    addDataToHistory(rawData);
    marked.clear();
    rawDataMutex.unlock();
    endResetModel();
    notifyUpdate(source);
}

int ByteItemModel::size() const
{
    return rawData.size();
}

QByteArray ByteItemModel::getRawData()
{
    return rawData;
}

int ByteItemModel::columnCount(const QModelIndex & parent) const
{

    if (parent.isValid()) {
        return 0;
    }
    return columnNumbers + 1;
}

int ByteItemModel::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return (rawData.size() / columnNumbers) + (rawData.size() % columnNumbers == 0 ? 0 : 1);
}

QVariant ByteItemModel::data(const QModelIndex &index, int role) const
{
    int pos = position(index);
    switch (role)
    {
        case Qt::DisplayRole:
        {
            if (index.column() == columnNumbers)
                return toPrintableString(rawData.mid(columnNumbers * index.row(),16));
            else if (pos != INVALID_POSITION)
                return QString::fromUtf8(rawData.mid(pos,1).toHex());
        }
            break;
        case Qt::BackgroundRole:
        {
            if (index.column() == columnNumbers)
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
        if (section < columnNumbers)
            return QString("%1").arg(section,2,columnNumbers,QChar('0'));
        else if (section == columnNumbers)
            return QString("Raw");
        else
            return QVariant();
    } else {
        if (section < rowCount())
            return QString("0x%1").arg(section * columnNumbers,0,16);
        else
            return QVariant();
    }
}

Qt::ItemFlags ByteItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if (index.column() >= columnNumbers)
        return Qt::ItemIsEnabled;

    if (columnNumbers * index.row() + index.column() < rawData.size())
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
        beginResetModel();
        int pos = columnNumbers * index.row() + index.column();
        if (pos > rawData.size()) {
            rawData.append(QByteArray(pos - rawData.size(),0x00));
        }
        rawData.replace(pos,1, hexVal);
        addDataToHistory(rawData);
        endResetModel();
        emit updatedFrom(HEXVIEW);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

int ByteItemModel::getColumnNumbers() const
{
    return columnNumbers;
}

QModelIndex ByteItemModel::createIndex(int pos) const
{
    return QAbstractTableModel::createIndex(pos / columnNumbers, pos % columnNumbers);
}

void ByteItemModel::insert(int pos, const QByteArray &data)
{
    beginResetModel();
    rawData.insert(pos,data);
    addDataToHistory(rawData);
    endResetModel();
    notifyUpdate();
}

void ByteItemModel::remove(int pos, int length)
{
    beginResetModel();
    rawData.remove(pos,length);
    addDataToHistory(rawData);
    endResetModel();
    notifyUpdate();
}

void ByteItemModel::replace(int pos, int length, QByteArray val)
{
    if (val.size() > length || val.size() < length) {
        rawData.remove(pos,length);
        rawData.insert(pos,val);
    } else {
        rawData.replace(pos,length, val);
    }
    addDataToHistory(rawData);
    notifyUpdate();
}

QByteArray ByteItemModel::extract(int pos, int length)
{
    if (length < 0) {
        pos = pos + length + 1;
        length = qAbs(length);
    }

    return rawData.mid(pos,length);
}

void ByteItemModel::clear()
{
    beginResetModel();
    rawData.clear();
    addDataToHistory(rawData);
    endResetModel();
    notifyUpdate();
}

int ByteItemModel::position(const QModelIndex &index) const
{
    if (index.isValid() && index.column() < columnNumbers && index.column() > -1 ) {
        int pos = columnNumbers * index.row() + index.column();
        return (pos < rawData.size() ? pos : INVALID_POSITION);
    } else {
        return INVALID_POSITION;
    }
}

bool ByteItemModel::isStringValid()
{
    return !rawData.contains('\x00');
}

bool ByteItemModel::isReadableText()
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

void ByteItemModel::mark(int start, int end, const QColor &ncolor, QString toolTip)
{
    if (start >= 0 && end > 0 && end < rawData.size() && start < rawData.size()) {
        int temp = start;
        start = qMin(start,end);
        end = qMax(temp, end);
        for (int i = start; i <= end; i++) {
            Markings ma;
            ma.color = ncolor;
            ma.text = toolTip;

            marked.insert(i, ma);
        }
    }
}

void ByteItemModel::clearMarking(int start, int end)
{
    if (start >= 0 && end > 0 && end < rawData.size() && start < rawData.size()) {
        int temp = start;
        start = qMin(start,end);
        end = qMax(temp, end);
        for (int i = start; i <= end; i++) {
            marked.remove(i);
        }
    }
}

void ByteItemModel::clearAllMarkings()
{
    marked.clear();
//    notifyUpdate(TEXTVIEW);
}

bool ByteItemModel::hasMarking() const
{
    return !marked.isEmpty();
}

void ByteItemModel::fromLocalFile(QString fileName)
{
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            emit error(tr("Failed to open %1:\n %2").arg(fileName).arg(file.errorString()));
            return;
        }

        beginResetModel();
        rawData = file.readAll();
        addDataToHistory(rawData);
        endResetModel();
        notifyUpdate();

    }
}

inline QString ByteItemModel::toPrintableString(const QByteArray &val) const
{
    QString line;
    for (int i = 0; i < val.size(); i++) {
        line.append(val.at(i) > 20 ? val.at(i) : '.');
    }
    return line;
}

void ByteItemModel::notifyUpdate(UpdateSource source)
{
    switch (source) {
    case EXTERNAL:
    case HEXVIEW:
        emit updatedFrom(HEXVIEW);
    case TEXTVIEW:
        emit dataChanged(QAbstractTableModel::createIndex(0,0),QAbstractTableModel::createIndex(rawData.size() / columnNumbers, columnNumbers));
        break;
    default:
        emit error("Unkonwn update source T_T");
    }
}

void ByteItemModel::historyForward()
{
    if (currentHistoryPointer < history.size() - 1) {
        currentHistoryPointer++;
        beginResetModel();
        rawData = history.at(currentHistoryPointer);
        endResetModel();
        notifyUpdate();
    }
}

void ByteItemModel::historyBackward()
{
    if (currentHistoryPointer > 0) {
        currentHistoryPointer--;
        beginResetModel();
        rawData = history.at(currentHistoryPointer);
        endResetModel();
        notifyUpdate();
    }
}

void ByteItemModel::addDataToHistory(const QByteArray &data)
{
    currentHistoryPointer++;
    history = history.mid(0,currentHistoryPointer);
    history.append(data);
}
