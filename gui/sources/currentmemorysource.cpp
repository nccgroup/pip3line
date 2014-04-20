/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "currentmemorysource.h"
#include "../crossplatform.h"
#include <QWidget>
#include <QDebug>
#include <QFile>
#include "memorywidget.h"
#include <QSysInfo>
#ifdef Q_OS_LINUX
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#elif defined(Q_OS_WIN)
#include "Windows.h"
#endif

MemRange::MemRange(quint64 lowerVal, quint64 upperVal, QString description):
    OffsetsRange(lowerVal, upperVal, description)
{
    read = false;
    write = false;
    exec = false;
#if defined(Q_OS_WIN)
    copy = false;
#endif
    priv = false;
}

void MemRange::setExec(bool val)
{
    exec = val;
}

bool MemRange::isExec() const
{
    return exec;
}

void MemRange::setRead(bool val)
{
    read = val;
}

bool MemRange::isRead() const
{
    return read;
}

void MemRange::setWrite(bool val)
{
    write = val;
}

bool MemRange::isWrite() const
{
    return write;
}

void MemRange::setPriv(bool val)
{
    priv = val;
}

bool MemRange::isPriv() const
{
    return priv;
}

QString MemRange::toString()
{
    QString ret;
    ret.append(QString::number(lowerVal,16)).append("->");
    ret.append(QString::number(upperVal,16)).append(" ");
    ret.append(read ? "r" : "_");
    ret.append(write ? "w" : "_");
    ret.append(exec ? "x" : "_");
#if defined(Q_OS_WIN)
    ret.append(copy ? "c" : "_");
#endif
    ret.append(priv ? "p" : "_");
    ret.append(":").append(description);

    return ret;

}

bool MemRange::operator<(const MemRange &other) const
{
        qDebug() << "MemRange Comparison";
    return upperVal < other.lowerVal;
}

#if defined(Q_OS_WIN)
bool MemRange::getCopy() const
{
    return copy;
}

void MemRange::setCopy(bool value)
{
    copy = value;
}
#endif


const QFont MemRangeModel::RegularFont = QFont("Courier New",10,2);
const QStringList MemRangeModel::headers = QStringList() << "Start" << "End" << "Permissions" << "Size" << "Description" ;
MemRangeModel::MemRangeModel(QObject *parent) :
    QAbstractListModel(parent)
{
    currentMemRow = -1;
}

MemRangeModel::~MemRangeModel()
{
    clear();
}

QList<MemRange *> MemRangeModel::getList()
{
    return ranges;
}

bool MemRangeModel::isOffsetInRange(quint64 offset)
{
    bool valid = false;
    for (int i = 0; i < ranges.size(); i++) {
        if (ranges.at(i)->isInRange(offset)) {
            valid = true;
            break;
        }
    }
    return valid ;
}
MemRange *MemRangeModel::getRange(const QModelIndex &index)
{
    int i = index.row();
    if (i < ranges.size()) {
        return ranges.at(i);
    }
    return NULL;
}

MemRange *MemRangeModel::getRange(quint64 offset)
{
    for (int i = 0; i < ranges.size(); i++) {
        if (ranges.at(i)->isInRange(offset)) {
            return ranges.at(i);
        }
    }
    return NULL;
}

void MemRangeModel::setCurrentMem(const QModelIndex &index)
{
    if (index.isValid())
        currentMemRow = index.row();
}

int MemRangeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ranges.size();
}

int MemRangeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return 5;
}

QVariant MemRangeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        int i = index.row();
        if (i < ranges.size()) {
            switch (index.column()) {
                case 0:
                return OffsetsRange::offsetToString(ranges.at(i)->getLowerVal());
                case 1:
                    return OffsetsRange::offsetToString(ranges.at(i)->getUpperVal());
                case 2:
                return QString("%1%2%3%4")
                          .arg(ranges.at(i)->isRead()?"r":"-")
                          .arg(ranges.at(i)->isWrite()?"w":"-")
                          .arg(ranges.at(i)->isExec()?"x":"-")
                          .arg(ranges.at(i)->isPriv()?"p":"-");
                case 3:
                        return QString::number(ranges.at(i)->getSize());
                case 4:
                    return ranges.at(i)->getDescription();
            }
        }
    } else if (role == Qt::BackgroundRole && index.row() == currentMemRow) {
        return QVariant(QColor(236,242,118));
    } else if (role == Qt::TextAlignmentRole && index.column() < headers.size()) {
        if (index.column() < 2)
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        else if (index.column() == 3)
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        else
            return Qt::AlignCenter;
    } else if (role == Qt::FontRole) {
        return RegularFont;
    }
    return QVariant();
}

QVariant MemRangeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        return headers.at(section);
    }
    return QVariant();
}

void MemRangeModel::clear()
{
    beginResetModel();
    while (!ranges.isEmpty()) {
        delete ranges.takeLast();
    }
    endResetModel();
}

void MemRangeModel::addRange(MemRange *range)
{
    beginInsertRows(QModelIndex(),ranges.size(),ranges.size());
    ranges.append(range);
    qSort(ranges.begin(),ranges.end(), OffsetsRange::lessThanFunc);
    endInsertRows();
}

void MemRangeModel::setCurrentRange(MemRange *range)
{
    currentMemRow = ranges.indexOf(range);
    emit dataChanged ( createIndex(currentMemRow,0), createIndex(currentMemRow,3));
}

CurrentMemorysource::CurrentMemorysource(QObject *parent) :
    LargeRandomAccessSource(parent)
{
    _readonly = true;
    capabilities = CAP_HISTORY;
    currentRange = NULL;
    rangesModel = new(std::nothrow) MemRangeModel(this);
    if (rangesModel == NULL) {
        qFatal("Cannot allocate memory for MemRangeModel X{");
    }
    mapMemory();

    refreshTimer.start(intervalMSec);

    quintptr pwritedata = (quintptr)this;
    setStartingOffset(pwritedata);
//    qDebug() << "writeData" << QString::number((quintptr)&writeData,16);
//    qDebug() << "memcpy" << QString::number((quintptr)&memcpy,16);
}

CurrentMemorysource::~CurrentMemorysource()
{
    delete rangesModel;
}

QString CurrentMemorysource::description()
{
    return name();
}

QString CurrentMemorysource::name()
{
    return tr("Current process memory");
}

quint64 CurrentMemorysource::size()
{
    qDebug() << "Wordsize = " << QSysInfo::WordSize;
    if (QSysInfo::WordSize == 32)
        return ULONG_MAX;

    return ULONG_LONG_MAX;
}

bool CurrentMemorysource::isOffsetValid(quint64 offset)
{
    return rangesModel->isOffsetInRange(offset);
}

MemRangeModel *CurrentMemorysource::getMemRanges() const
{
    return rangesModel;
}

QWidget *CurrentMemorysource::requestGui(QWidget *parent)
{
    MemoryWidget * mw = new(std::nothrow) MemoryWidget(this, parent);
    if (mw == NULL) {
        qFatal("Cannot allocate memory for MemoryWidget X{");
    }
    mw->setProcSelection(false);
    return mw;
}

void CurrentMemorysource::mapMemory()
{
    rangesModel->clear();
#ifdef Q_OS_LINUX
    QFile pfile("/proc/self/maps");
    if (!pfile.open(QIODevice::ReadOnly)) {
        emit log(tr("Cannot open /proc/self/maps for reading"),metaObject()->className(), Pip3lineConst::LERROR);
        return;
    }

    QList<QByteArray> mappings = pfile.readAll().split('\n');
    for (int i = 0 ; i < mappings.size(); i++) {
        QList<QByteArray> entries = mappings.at(i).split(' ');
        if (entries.size() > 6) {

            QList<QByteArray> range = entries.at(0).split('-');
            MemRange *memrange = new(std::nothrow) MemRange(range.at(0).toULongLong(0,16),range.at(1).toULongLong(0,16) - 1,QString::fromUtf8(entries.last()));
            if (memrange == NULL) {
                qFatal("Cannot allocate memory for MemRange X{");
            }
            memrange->setRead(entries.at(1).at(0) == 'r');
            memrange->setWrite(entries.at(1).at(1) == 'w');
            memrange->setExec(entries.at(1).at(2) == 'x');
            memrange->setPriv(entries.at(1).at(3) == 'p');
            rangesModel->addRange(memrange);
            qDebug() << memrange->toString();
        }
    }

    emit mappingChanged();

#elif defined(Q_OS_WIN)
    ULONG_PTR addrCurrent = 0;
    ULONG_PTR lastBase = (-1);
    for(;;)
    {
#ifdef Q_OS_WIN64
        MEMORY_BASIC_INFORMATION64 memMeminfo;
#else
        MEMORY_BASIC_INFORMATION32 memMeminfo;
#endif
        VirtualQuery(reinterpret_cast<LPVOID>(addrCurrent), reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&memMeminfo), sizeof(memMeminfo) );

        if(lastBase == (ULONG_PTR) memMeminfo.BaseAddress) {
            break;
        }

        lastBase = (ULONG_PTR) memMeminfo.BaseAddress;

        if(memMeminfo.State == MEM_COMMIT) {
            MemRange *memrange = new(std::nothrow) MemRange((quint64)memMeminfo.BaseAddress,
                                                            (quint64)(memMeminfo.BaseAddress + memMeminfo.RegionSize - 1)
                                                            );
            switch (memMeminfo.AllocationProtect)
            {
                case PAGE_EXECUTE:
                    memrange->setExec(true);
                    break;
                case PAGE_EXECUTE_READ:
                    memrange->setRead(true);
                    memrange->setExec(true);
                    break;
                case PAGE_EXECUTE_READWRITE:
                    memrange->setRead(true);
                    memrange->setExec(true);
                    memrange->setWrite(true);
                    break;
                case PAGE_EXECUTE_WRITECOPY:
                    memrange->setExec(true);
                    memrange->setWrite(true);
                    memrange->setCopy(true);
                    break;
                case PAGE_READONLY:
                    memrange->setRead(true);
                    break;
                case PAGE_READWRITE:
                    memrange->setRead(true);
                    memrange->setExec(true);
                    memrange->setWrite(true);
                    break;
                case PAGE_WRITECOPY:
                    memrange->setWrite(true);
                    memrange->setCopy(true);
                    break;
                case PAGE_NOACCESS:
                default: // nothing to set
                    break;
            }

            switch (memMeminfo.Type){
                case MEM_IMAGE:
                    memrange->setDescription(tr("Image"));
                    break;
                case MEM_MAPPED:
                    memrange->setDescription(tr("Mapped area"));
                    break;
                case MEM_PRIVATE:
                    memrange->setDescription(tr("Private"));
                    memrange->setPriv(true);
                    break;
            }
            rangesModel->addRange(memrange);
            qDebug() << memrange->toString();
        }
        addrCurrent += memMeminfo.RegionSize;
    }
#endif
}

quint64 CurrentMemorysource::lowByte()
{
    MemRange * range = rangesModel->getRange(currentStartingOffset);
    if (range != NULL)
        return range->getLowerVal();

    emit log(tr("The current starting offset was not found in any registered memory range T_T"),metaObject()->className(), Pip3lineConst::LERROR);
    return currentStartingOffset;
}

quint64 CurrentMemorysource::highByte()
{
    MemRange * range = rangesModel->getRange(currentStartingOffset);
    if (range != NULL)
        return range->getUpperVal();

    emit log(tr("The current starting offset was not found in any registered memory range T_T"),metaObject()->className(), Pip3lineConst::LERROR);
    return currentStartingOffset;
}

bool CurrentMemorysource::tryMoveUp(int sizeToMove)
{
    return tryMoveView(-sizeToMove);
}

bool CurrentMemorysource::tryMoveDown(int sizeToMove)
{
    return tryMoveView(sizeToMove);
}

bool CurrentMemorysource::tryMoveView(int sizeToMove)
{
    quint64 newOffset = 0;
    quint64 upperBound = currentRange->getUpperVal() + 1;
    if (sizeToMove == 0)
        return false;

    if (sizeToMove < 0) {
        if (currentStartingOffset == 0)
            return false; // already at the beginning, nothing to see here
        if (currentStartingOffset < (quint64)(-1 * sizeToMove)) { // checking how much we can go up
            newOffset = 0;
        } else {
            newOffset = currentStartingOffset + sizeToMove;
        }

    } else {
        if (ULONG_LONG_MAX - (quint64)sizeToMove - (quint64)chunksize< currentStartingOffset)
            return false; // checking overflow

        if (currentStartingOffset + (quint64)sizeToMove + (quint64)chunksize > upperBound) {
            return false; // no more data
        }

        newOffset = currentStartingOffset + sizeToMove;
    }

    if (!isOffsetValid(newOffset))
        return false; // just return if the offset is not accessible

    if (!isOffsetValid(newOffset +  chunksize - 1))
        return false; // just return if the offset is not accessible

    int readsize = qMin(upperBound - newOffset,(quint64)chunksize);
    QByteArray temp;
    if (!readData(newOffset, temp,readsize)) {
        return false;
    }
    currentStartingOffset = newOffset;
    dataChunk = temp;
    emit updated(INVALID_SOURCE);
    emit sizeChanged();
    return true;
}

bool CurrentMemorysource::setStartingOffset(quint64 offset)
{
    bool found = false;
    QList<MemRange *> ranges = rangesModel->getList();
    for (int i = 0; i < ranges.size(); i++) {
        if (ranges.at(i)->isInRange(offset)) {
            found = true;
            if (ranges.at(i) != currentRange) {
                currentRange = ranges.at(i);
                rangesModel->setCurrentRange(currentRange);
            }
            QByteArray temp;
            quint64 newOffset = offset;
            quint64 segupper = currentRange->getUpperVal() + 1;

            if (segupper - newOffset < (quint64)chunksize) { // if the data size between offset and the end is inferior to chunksize
                newOffset = segupper - (quint64)chunksize; // then starting offset is put back
            }

            if (newOffset % 16 != 0) { // aligning on a 16 bytes boundary
                newOffset = newOffset - newOffset % 16 + (offset >= newOffset + chunksize ? 16 : 0);
            }
            if (readData(newOffset,temp,chunksize)) {
                dataChunk = temp;
                currentStartingOffset = newOffset;
                emit updated(INVALID_SOURCE);
                emit sizeChanged();
                addToHistory(newOffset);
                return true;
            }
        }
    }
    if (!found)
        emit log(tr("The current starting offset was not found in any registered memory range T_T"),metaObject()->className(), Pip3lineConst::LERROR);

    return false;
}

bool CurrentMemorysource::readData(quint64 offset, QByteArray &data, int size)
{
    qDebug() << "read current memory data" << QString::number(offset,16).prepend("0x") << size;
    if (size < 0) { // trivial
        qDebug() << "Negative size";
        return false;
    }
    if (size == 0) { // trivial
        qDebug() << "Null size";
        data.clear();
        return true;
    }

    QList<MemRange *> ranges = rangesModel->getList();
    MemRange * curRange = NULL;
    bool valid = false;
    for (int i = 0; i < ranges.size(); i++) {
        curRange = ranges.at(i);
        if (curRange->isInRange(offset)) {
            if (curRange->isRead()) {
                valid = true;
                if (!curRange->isInRange(offset + size - 1)) {
                    size = curRange->getUpperVal() - offset;
                }
            } else {
                qDebug() << "not readable" << offset;
                emit log(tr("This memory region is not readable [0x%1-0x%2].").arg(QString::number(curRange->getLowerVal(),16)).arg(QString::number(curRange->getUpperVal(),16)),metaObject()->className(), Pip3lineConst::LWARNING);
                return false;
            }
            break;
        }
    }

    if (!valid) {
        qDebug() << "invalid offset";
        emit log(tr("offset %1 is not in a valid memory blocks").arg(QString::number(offset,16)),metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }

#ifdef Q_OS_UNIX
    // testing if the address is readable, just in case the previous check did not worked
    int pipefd[2];
    int ret = pipe(pipefd);
    if (ret != 0) {
        emit log(tr("Cannot create a pipe for testing the address"),metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }

    ssize_t ret2 = write(pipefd[1],(void *)offset,size);
    if (ret2 < 0 && errno != 0) {
        emit log(tr("Cannot access the address 0x%1 for reading [%1]").arg(errorString(errno)),metaObject()->className(), Pip3lineConst::LERROR);
        close(pipefd[0]);
        close(pipefd[1]);
        return false;
    } else if (ret2 < size) {
        emit log(tr("Could only read %1 bytes from 0x%2").arg(ret2).arg(QString::number(offset,16)),metaObject()->className(), Pip3lineConst::LERROR);
        size = ret2;
    }

    close(pipefd[0]);
    close(pipefd[1]);
#endif
    data.resize(size);
    void *dest = memcpy(data.data(), (void *)offset, size);

    if (dest != (void *)data.data()) {
        emit log(tr("Uuuh??"),metaObject()->className(), Pip3lineConst::LERROR);
    }

    //qDebug() << "Data read" << QString::number(offset,16).prepend("0x") << data.size();
    return true;
}

bool CurrentMemorysource::writeData(quint64 offset,  QByteArray &data, int size)
{
    QList<MemRange *> ranges = rangesModel->getList();
    bool valid = false;
    MemRange * curRange = NULL;
    if (size < 0)
        return valid;

#ifdef Q_OS_UNIX
    for (int i = 0; i < ranges.size(); i++) {
        curRange = ranges.at(i);
        if (curRange->isInRange(offset)) {
            if (curRange->isWrite()) {
                valid = true;
                if (!curRange->isInRange(offset + size - 1)) {
                    size = curRange->getUpperVal() - offset;
                    memcpy((void *)&offset,(void *)data.data(), size);
                }
            } else {
                qDebug() << "not writable" << offset;
                emit log(tr("This memory region is not writable [0x%1-0x%2].").arg(QString::number(curRange->getLowerVal(),16)).arg(QString::number(curRange->getUpperVal(),16)),metaObject()->className(), Pip3lineConst::LWARNING);
                return false;
            }
            break;
        }
    }

#elif defined(Q_OS_WIN)

#endif

    if (!valid) {
        emit log(tr("offset %1 is not in a valid memory blocks").arg(QString::number(offset,16)),metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }
    return true;
}

QString CurrentMemorysource::errorString(int errnoVal)
{
#ifdef Q_OS_LINUX
    switch(errnoVal) {
    case EACCES:
        return QString("EACCES");
    case EINVAL:
        return QString("EINVAL");
    case ENOMEM:
        return QString("ENOMEM");
    case EFAULT:
        return QString("EFAULT");
    default:
        return QString("Unmanaged [%1]").arg(errnoVal);
    }

#endif

    return QString("Unmanaged [%1]").arg(errnoVal);
}

