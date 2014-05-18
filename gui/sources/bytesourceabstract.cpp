/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QFile>
#include <QWidget>
#include <QDebug>
#include <QMapIterator>
#include <QTimerEvent>
#include <QFileInfo>
#include "bytesourceabstract.h"

const quintptr ByteSourceAbstract::INVALID_SOURCE = 0;

const QString OffsetsRange::HEXFORMAT = "0x%1";


// utility class used to store ranges of quint64 and associated data
OffsetsRange::OffsetsRange(quint64 nlowerVal, quint64 nupperVal, QString ndescription)
{
    if (nlowerVal < nupperVal) {
        lowerVal = nlowerVal;
        upperVal = nupperVal;
    } else {
        lowerVal = nupperVal;
        upperVal = nlowerVal;
    }
    description = ndescription;
    size = upperVal - lowerVal + 1;
}

OffsetsRange::OffsetsRange(const OffsetsRange &other)
{
    *this = other;
}

OffsetsRange &OffsetsRange::operator=(const OffsetsRange &other)
{
    this->description = other.description;
    this->foregroundColor = other.foregroundColor;
    this->backgroundColor = other.backgroundColor;
    this->lowerVal = other.lowerVal;
    this->upperVal = other.upperVal;
    return *this;
}

OffsetsRange::~OffsetsRange()
{

}

bool OffsetsRange::isInRange(int value)
{
    if (value < 0)
        return false;

    return isInRange((quint64)value);
}

bool OffsetsRange::isInRange(quint64 value)
{
    return !(value < lowerVal || value > upperVal);
}

QString OffsetsRange::getDescription() const
{
    return description;
}

void OffsetsRange::setDescription(const QString &descr)
{
    description = descr;
}

QColor OffsetsRange::getForeground() const
{
    return foregroundColor;
}

void OffsetsRange::setForeground(const QColor &color)
{
    foregroundColor = color;
}

QColor OffsetsRange::getBackground() const
{
    return backgroundColor;
}

void OffsetsRange::setBackground(const QColor &color)
{
    backgroundColor = color;
}

quint64 OffsetsRange::getLowerVal() const
{
    return lowerVal;
}

void OffsetsRange::setLowerVal(quint64 val)
{
    lowerVal = val;
}

quint64 OffsetsRange::getUpperVal() const
{
    return upperVal;
}

void OffsetsRange::setUpperVal(quint64 val)
{
    upperVal = val;
}

bool OffsetsRange::operator<(const OffsetsRange &other) const
{
    return upperVal < other.lowerVal;
}

bool OffsetsRange::sameMarkings(const OffsetsRange &other) const
{
    return (backgroundColor == other.backgroundColor) && (foregroundColor == other.foregroundColor) && (description == other.description);
}

QString OffsetsRange::offsetToString(quint64 val)
{
    return HEXFORMAT.arg(val,16,16,QChar('0'));
}

bool OffsetsRange::lessThanFunc(OffsetsRange *or1, OffsetsRange *or2)
{
    return or1->upperVal < or2->lowerVal;
}
quint64 OffsetsRange::getSize() const
{
    return size;
}

void OffsetsRange::setSize(const quint64 &value)
{
    size = value;
}


ComparableRange::ComparableRange(OffsetsRange *pointer)
{
    range = pointer;
}
OffsetsRange *ComparableRange::getRange() const
{
    return range;
}

void ComparableRange::setRange(OffsetsRange *value)
{
    range = value;
}

bool ComparableRange::operator<(const ComparableRange &other) const
{
    return range->upperVal < other.range->lowerVal;
}

bool ComparableRange::operator==(const ComparableRange &other) const
{
    return range == other.range;
}


ByteSourceAbstract::ByteSourceAbstract(QObject *parent) :
    QObject(parent)
{
    capabilities = 0;
    confGui = NULL;
    buttonBar = NULL;
    upperView = NULL;
    searchObj = NULL;
    cachedMarkingRange = NULL;
    currentHistoryPointer = -1;
    applyingHistory = false;
    _readonly = false;
}

ByteSourceAbstract::~ByteSourceAbstract()
{
    if (searchObj != NULL) {
        searchObj->stopSearch();
        searchObj->quit();
        if (!searchObj->wait(10000)) {
            qCritical() << "Could not stop thread for" << this;
        }
        delete searchObj;
    }
    clearAllMarkings();
    delete confGui;
}

QString ByteSourceAbstract::name()
{
    return _name;
}

void ByteSourceAbstract::setName(QString newName)
{
    if (_name.compare(newName) != 0) {
        _name = newName;
        emit nameChanged(newName);
    }
}

void ByteSourceAbstract::setData(QByteArray, quintptr )
{
}

QByteArray ByteSourceAbstract::getRawData()
{
    return QByteArray();
}

quint64 ByteSourceAbstract::size()
{
    return 0;
}

QByteArray ByteSourceAbstract::viewExtract(int offset, int length)
{
    if (offset < 0 || length < 0) {
        emit log(tr("[viewReplace] Negative offset/length o:%1 l:%2, ignoring").arg(offset).arg(length),metaObject()->className(),Pip3lineConst::LERROR);
        return QByteArray();
    }

    return extract(offset,length);
}

char ByteSourceAbstract::viewExtract(int offset)
{
    if (offset < 0) {
        emit log(tr("[viewExtract] Negative offset %1, ignoring").arg(offset),metaObject()->className(),Pip3lineConst::LERROR);
        return '\00';
    }
    return extract(offset);
}

void ByteSourceAbstract::replace(quint64 , int , QByteArray , quintptr )
{
    emit log(tr("[replace] Not implemented"),metaObject()->className(),Pip3lineConst::LERROR);
}

void ByteSourceAbstract::viewReplace(int offset, int length, QByteArray repData, quintptr source)
{
    if (offset < 0 || length < 0) {
        emit log(tr("[viewReplace] Negative offset/length o:%1 l:%2, ignoring").arg(offset).arg(length),metaObject()->className(),Pip3lineConst::LERROR);
        return;
    }
    replace(offset,length,repData,source);
}

void ByteSourceAbstract::insert(quint64 , QByteArray , quintptr )
{
    emit log(tr("[insert] Not implemented"),metaObject()->className(),Pip3lineConst::LERROR);
}

void ByteSourceAbstract::viewInsert(int offset, QByteArray repData, quintptr source)
{
    if (offset < 0) {
        emit log(tr("[viewInsert] Negative offset %1, ignoring").arg(offset),metaObject()->className(),Pip3lineConst::LERROR);
        return;
    }
    insert(offset,repData,source);
}

void ByteSourceAbstract::remove(quint64, int , quintptr )
{
    emit log(tr("[remove] Not implemented"),metaObject()->className(),Pip3lineConst::LERROR);
}

void ByteSourceAbstract::viewRemove(int offset, int length, quintptr source)
{

    if (offset < 0 || length < 0) {
        emit log(tr("[viewRemove] Negative offset/length o:%1 l:%2, ignoring").arg(offset).arg(length),metaObject()->className(),Pip3lineConst::LERROR);
        return;
    }
    remove(offset,length,source);
}

void ByteSourceAbstract::clear(quintptr)
{
}

bool ByteSourceAbstract::hasCapability(ByteSourceAbstract::CAPABILITIES cap)
{
    return (capabilities & cap);
}

quint32 ByteSourceAbstract::getCapabilities() const
{
    return capabilities;
}

bool ByteSourceAbstract::setReadOnly(bool readonly)
{
    if (!(capabilities & CAP_WRITE))
            return readonly;

    _readonly = readonly;
    emit readOnlyChanged(_readonly);

    return true;
}

bool ByteSourceAbstract::isReadonly()
{
    return !(capabilities & CAP_WRITE) || _readonly;
}

int ByteSourceAbstract::preferredTabType()
{
    return TAB_GENERIC;
}

bool ByteSourceAbstract::isReadableText()
{
    return false;
}

SearchAbstract *ByteSourceAbstract::getSearchObject(QObject * parent, bool singleton)
{
    if (singleton) {
        if (searchObj == NULL) {
            searchObj = requestSearchObject(parent);
//            if (searchObj != NULL)
//                searchObj->initialize();
        }
        return searchObj;
    }
    return requestSearchObject(parent);
}

void ByteSourceAbstract::viewMark(int start, int end, const QColor &bgcolor, const QColor &fgColor, QString toolTip)
{
    if (start < 0 || end < 0 ) {
        emit log(tr("[viewMark] Negative end/start %1/%2, ignoring").arg(start).arg(end),metaObject()->className(),Pip3lineConst::LERROR);
    } else {
        mark(start,end,bgcolor,fgColor,toolTip);
    }
}

void ByteSourceAbstract::mark(quint64 start, quint64 end, const QColor &bgcolor, const QColor &fgColor, QString toolTip)
{
    markNoUpdate(start,end,bgcolor,fgColor, toolTip);
    emit updated(INVALID_SOURCE);
}

void ByteSourceAbstract::markNoUpdate(quint64 start, quint64 end, const QColor &bgcolor, const QColor &fgColor, QString toolTip)
{
    // stupidity protection
    if (start > end) {
        quint64 temp = start;
        start = end;
        end = temp;
    }

    OffsetsRange *newRange = NULL;
    newRange = new(std::nothrow)OffsetsRange(start,end);
    if (newRange == NULL) {
        qFatal("Cannot allocate memory for OffsetsRange X{");
    }
    newRange->setBackground(bgcolor);
    newRange->setForeground(fgColor);
    newRange->setDescription(toolTip);
    ComparableRange cr(newRange);

    Markings ma;
    ma.bgcolor = bgcolor;
    ma.fgcolor = fgColor;
    ma.text = toolTip;

    bool sorted = false;

    QMapIterator<ComparableRange, Markings> i(userMarkingsRanges);
    while (i.hasNext()) {
        i.next();
        OffsetsRange *currange = i.key().range;
        bool sameMarkings = i.value() == ma;
        if (newRange->upperVal < currange->lowerVal) {
            if (sameMarkings && newRange->upperVal == currange->lowerVal - 1) {
                currange->lowerVal = newRange->lowerVal;
                delete newRange; // cleaning
                sorted = true;
                break;
            }
            // if the new range is "under" the current range, just add it to the QMap
            // it will be sorted automatically
            userMarkingsRanges.insert(cr,ma);
            sorted = true;
            break;
        } else if (newRange->lowerVal > currange->upperVal) {
            if (sameMarkings && newRange->lowerVal == currange->upperVal + 1) { // merging same markings
                currange->upperVal = newRange->upperVal;
                delete newRange;
                newRange = currange;
                cr = ComparableRange(currange);
            }
            // ranges are disjoints just continue, and check the next range
            continue;
        } else if (newRange->lowerVal <= currange->lowerVal) { // overlapping occured
            if (newRange->upperVal >= currange->upperVal) {
                // if new range overlap entirely the current range
                // just remove the old range and continue
                // the iterator will just continue to the next range (or end)
                userMarkingsRanges.remove(i.key());
                delete currange;
                continue;
            } else { // new range overlap partially the current range
                if (sameMarkings) { // in case the markings are the same
                    // just extend the current range
                    currange->lowerVal = newRange->lowerVal;
                    delete newRange; // cleaning
                    sorted = true;
                    break;
                } else {
                    // otherwise we have to "reduce" the current range
                    currange->lowerVal = newRange->upperVal + 1;
                    // and store the new one
                    userMarkingsRanges.insert(cr,ma);
                    sorted = true;
                    break;
                }
            }

        } else {
            // at this point we know that start > currange->lowerVal and start <= currange->upperVal
            if (sameMarkings) {
            // if markings are the same, all done, no need for the new range, just grow the current one
                if ( newRange->upperVal > currange->upperVal)
                    currange->upperVal = newRange->upperVal;
                delete newRange;
                newRange = currange;
                cr = ComparableRange(currange);
                // we need to continue in case the current range now expand over other ones
                // we don't need to remove the current range from the QMap, if an insert occurs later, it
                // won't have any effect
                continue;
            }

            // we can already chop the current range , but make sure to conserve the old value
            quint64 oldupperval = currange->upperVal;
            currange->upperVal = newRange->lowerVal - 1;
            if (newRange->upperVal >= oldupperval) {
                // just continue the loop to check with the next range(s)
                continue;
            } else {
                // final case if the new range is included entirely into the current one
                // we need to create a new one after
                OffsetsRange *newRange2 = new(std::nothrow)OffsetsRange(newRange->upperVal + 1,oldupperval);
                if (newRange2 == NULL) {
                    qFatal("Cannot allocate memory for OffsetsRange X{");
                }
                newRange2->setBackground(currange->backgroundColor);
                newRange2->setForeground(currange->foregroundColor);
                newRange2->setDescription(currange->description);
                //insert the new range
                userMarkingsRanges.insert(cr,ma);
                // and the newest created one, from the split
                userMarkingsRanges.insert(ComparableRange(newRange2),i.value());
                sorted = true;
                break;
            }

        }
    }

    if (!sorted) { // this means that the range is above all the other ones or this is the first one
        userMarkingsRanges.insert(cr,ma);
    }


   // qDebug() << "Markings ranges" << userMarkingsRanges.size();
    cachedMarkingRange = NULL;
}

void ByteSourceAbstract::viewClearMarking(int start, int end)
{
    if (start < 0 || end < 0 ) {
        emit log(tr("[viewClearMarking] Negative end/start %1/%2, ignoring").arg(start).arg(end),metaObject()->className(),Pip3lineConst::LERROR);
    } else {
        clearMarking(start,end);
    }
}

void ByteSourceAbstract::clearMarking(quint64 start, quint64 end)
{
    qDebug() << "clearing";
    // stupidity protection
    if (start > end) {
        quint64 temp = start;
        start = end;
        end = temp;
    }
    QMapIterator<ComparableRange, Markings> i(userMarkingsRanges);
    while (i.hasNext()) {
        i.next();
        OffsetsRange * currRange = i.key().range;
        if (start <= currRange->lowerVal) { //possible overlap
            if (end < currRange->lowerVal) // clearing a non-existing marking, nothing to see here
                break;
            else if (end < currRange->upperVal) { // currRange need to be reduced
                currRange->lowerVal = end  + 1;
                break;// we are done here
            } else {// just clear the entire range
                userMarkingsRanges.remove(i.key());
                delete currRange;
                continue; // we still need to check if it overlap the next ranges
            }
        } else if (start > currRange->upperVal) { // the clearing range is above the current range
            // just continue to the next one
            continue;
        } else { // start > currRange->lowerVal && start <= currRange->upperVal
            if (end >= currRange->upperVal) { // overlapping above the upper val
                currRange->upperVal = start - 1;
                continue; // need to check the next range
            } else { // some split need to be performed
                OffsetsRange *newRange2 = new(std::nothrow)OffsetsRange(end + 1,currRange->upperVal);
                if (newRange2 == NULL) {
                    qFatal("Cannot allocate memory for OffsetsRange X{");
                }
                newRange2->setBackground(currRange->backgroundColor);
                newRange2->setForeground(currRange->foregroundColor);
                newRange2->setDescription(currRange->description);
                currRange->upperVal = start - 1;
                // Insert the newest created one, from the split
                userMarkingsRanges.insert(ComparableRange(newRange2),i.value());
                break; //job done
            }
        }
    }
    cachedMarkingRange = NULL;

    emit updated(INVALID_SOURCE);
}

void ByteSourceAbstract::setNewMarkingsMap(QMap<ComparableRange, ByteSourceAbstract::Markings> newUserMarkingsRanges)
{
    userMarkingsRanges = newUserMarkingsRanges;
    emit updated(INVALID_SOURCE);
}

void ByteSourceAbstract::clearAllMarkings()
{
    clearAllMarkingsNoUpdate();
    emit updated(INVALID_SOURCE);
}

void ByteSourceAbstract::clearAllMarkingsNoUpdate()
{
    QMapIterator<ComparableRange, Markings> i(userMarkingsRanges);
    while(i.hasNext()) {
        i.next();
        delete i.key().range;
    }
    userMarkingsRanges.clear();
    cachedMarkingRange = NULL;
}

bool ByteSourceAbstract::hasMarking() const
{
    return !userMarkingsRanges.isEmpty();
}

QColor ByteSourceAbstract::getBgColor(quint64 pos)
{
    if (cachedMarkingRange != NULL && cachedMarkingRange->isInRange(pos)) {
        return cachedMarkingRange->backgroundColor;
    }
    cachedMarkingRange = NULL;
    QColor color;
    QMapIterator<ComparableRange, Markings> i(userMarkingsRanges);
    while(i.hasNext()) {
        i.next();
        if (i.key().range->isInRange(pos)) {
            color = i.value().bgcolor;
            cachedMarkingRange = i.key().range;
            break;
        }
    }
    return color;
}

QColor ByteSourceAbstract::getBgViewColor(int pos)
{
    if (pos < 0) {
        emit log(tr("[getBgViewColor] Negative offset/length o:%1, ignoring").arg(pos),metaObject()->className(),Pip3lineConst::LERROR);
        return QColor();
    }

    return getBgColor(pos);
}

QColor ByteSourceAbstract::getFgColor(quint64 pos)
{
    if (cachedMarkingRange != NULL && cachedMarkingRange->isInRange(pos)) {
        return cachedMarkingRange->foregroundColor;
    }
    cachedMarkingRange = NULL;
    QColor color;
    QMapIterator<ComparableRange, Markings> i(userMarkingsRanges);
    while(i.hasNext()) {
        i.next();
        if (i.key().range->isInRange(pos)) {
            color = i.value().fgcolor;
            cachedMarkingRange = i.key().range;
            break;
        }
    }
    return color;
}

QColor ByteSourceAbstract::getFgViewColor(int pos)
{
    if (pos < 0) {
        emit log(tr("[getFgViewColor] Negative offset/length o:%1, ignoring").arg(pos),metaObject()->className(),Pip3lineConst::LERROR);
        return QColor();
    }
    return getFgColor(pos);
}

QString ByteSourceAbstract::getToolTip(quint64 pos)
{
    if (cachedMarkingRange != NULL && cachedMarkingRange->isInRange(pos)) {
        return cachedMarkingRange->description;
    }
    cachedMarkingRange = NULL;
    QString texttip;
    QMapIterator<ComparableRange, Markings> i(userMarkingsRanges);
    while(i.hasNext()) {
        i.next();
        if (i.key().range->isInRange(pos)) {
            texttip = i.value().text;
            cachedMarkingRange = i.key().range;
            break;
        }
    }
    return texttip;
}

QString ByteSourceAbstract::getViewToolTip(int pos)
{
    if (pos < 0) {
        emit log(tr("[getViewToolTip] Negative offset/length o:%1, ignoring").arg(pos),metaObject()->className(),Pip3lineConst::LERROR);
        return QString();
    }

    return getToolTip(pos);
}

QString ByteSourceAbstract::toPrintableString(const QByteArray &val)
{
    QString line;
    for (int i = 0; i < val.size(); i++) {
        line.append((val.at(i) > 32 && val.at(i) < 127) ? val.at(i) : '.');
    }
    return line;
}

quint64 ByteSourceAbstract::lowByte()
{
    return 0;
}

quint64 ByteSourceAbstract::highByte()
{
    return size() - 1;
}

int ByteSourceAbstract::textOffsetSize()
{
    return QString::number(size(),16).size();
}

bool ByteSourceAbstract::tryMoveUp(int )
{
    return false;
}

bool ByteSourceAbstract::tryMoveDown(int )
{
    return false;
}

bool ByteSourceAbstract::hasDiscreetView()
{
    return false;
}

QWidget *ByteSourceAbstract::getGui(QWidget *parent,ByteSourceAbstract::GUI_TYPE type)
{
    QWidget **requestedGui = NULL;

    switch (type) {
        case (ByteSourceAbstract::GUI_CONFIG):
            requestedGui = &confGui;
            break;
        case (GUI_BUTTONS):
            requestedGui = &buttonBar;
            break;
        case (GUI_UPPER_VIEW):
            requestedGui = &upperView;
            break;
        default:
            qCritical() << tr("Unmanaged ByteSourceAbstract::GUI_TYPE:%1").arg(type);
            return NULL;
    }

    if ((*requestedGui) == NULL) {
        (*requestedGui) = requestGui(parent, type);
        if ((*requestedGui) != NULL) {
            connect((*requestedGui), SIGNAL(destroyed()), this, SLOT(onGuiDestroyed()), Qt::UniqueConnection);
        }
    }
    return (*requestedGui);
}

int ByteSourceAbstract::viewSize()
{
    quint64 csize = size();
    if (csize > INT_MAX) {
        emit log(tr("Size of the byte source is hitting the INT_MAX limit for the gui view, consider implementing a discreet view"),metaObject()->className(),Pip3lineConst::LERROR);
        csize = INT_MAX;
    }
    return (int)csize;
}

void ByteSourceAbstract::setViewSize(int)
{
    emit log(tr("[setViewSize] Not implemented"),metaObject()->className(),Pip3lineConst::LERROR);
}

void ByteSourceAbstract::onGuiDestroyed()
{
    QWidget *gobj = qobject_cast<QWidget *>(sender());
    if (gobj == confGui) {
        confGui = NULL;
    }
    else if (gobj == buttonBar) {
        buttonBar = NULL;
    }
    else if (gobj == upperView){
        upperView = NULL;
    }
    else
        qCritical() << "[ByteSourceAbstract::onGuiDestroyed] Unknown destroyed widget";
}

QWidget *ByteSourceAbstract::requestGui(QWidget *, ByteSourceAbstract::GUI_TYPE)
{
    return NULL;
}

SearchAbstract *ByteSourceAbstract::requestSearchObject(QObject *)
{
    return NULL;
}

bool ByteSourceAbstract::historyForward()
{
    if (_readonly) {
        emit log(tr("[historyForward] source is set readonly, ignoring."),metaObject()->className(),Pip3lineConst::LWARNING);
        return false;
    }

    if (currentHistoryPointer < history.size() - 1) {
        currentHistoryPointer++;
        historyApply(history.at(currentHistoryPointer),true);
        return true;
    }
    return false;
}

bool ByteSourceAbstract::historyBackward()
{
    if (_readonly) {
        emit log(tr("[historyBackward] source is set readonly, ignoring."),metaObject()->className(),Pip3lineConst::LWARNING);
        return false;
    }

    if (currentHistoryPointer >= 0) {
        historyApply(history.at(currentHistoryPointer),false);
        currentHistoryPointer--;
        return true;
    }
    return false;
}

void ByteSourceAbstract::historyClear()
{
    currentHistoryPointer = -1;
    history.clear();
}

void ByteSourceAbstract::historyApply(ByteSourceAbstract::HistItem item, bool forward)
{
    applyingHistory = true;
    if (item.action == REPLACE) {
        if (forward) {
            replace(item.offset,item.before.length(), item.after);
        } else {
            replace(item.offset,item.after.length(), item.before);
        }
    } else if (item.action == REMOVE) {
        if (forward) {
            remove(item.offset,item.before.length());
        } else {
            insert(item.offset,item.before);
        }
    } else if (item.action == INSERT) {
        if (forward)
            insert(item.offset,item.after);
        else
            remove(item.offset,item.after.length());
    } else {
        emit log(tr("Unknown history action: %1").arg(item.action),metaObject()->className(),Pip3lineConst::LERROR);
    }
    applyingHistory = false;
}

void ByteSourceAbstract::historyAddInsert(quint64 offset, QByteArray after)
{
    if (applyingHistory) // don't touch the history if we are applying a history item
        return;

    HistItem item;
    item.action = INSERT;
    item.offset = offset;
    item.after = after;
    historyAdd(item);
}

void ByteSourceAbstract::historyAddRemove(quint64 offset, QByteArray before)
{
    if (applyingHistory) // don't touch the history if we are applying a history item
        return;

    HistItem item;
    item.action = REMOVE;
    item.offset = offset;
    item.before = before;
    historyAdd(item);
}

void ByteSourceAbstract::historyAddReplace(quint64 offset, QByteArray before, QByteArray after)
{
    if (applyingHistory) // don't touch the history if we are applying a history item
        return;

    HistItem item;
    item.action = REPLACE;
    item.offset = offset;
    item.before = before;
    item.after = after;
    historyAdd(item);
}

void ByteSourceAbstract::historyAdd(ByteSourceAbstract::HistItem item)
{
    if (applyingHistory) // don't touch the history if we are applying a history item (this one should not be needed)
        return;

    currentHistoryPointer++;
    history = history.mid(0,currentHistoryPointer);

    history.append(item);
}

void ByteSourceAbstract::fromLocalFile(QString fileName)
{
    // by default just replace all data via setData (although this method is not garantee to be implemented)
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            emit log(tr("Failed to open %1:\n %2").arg(fileName).arg(file.errorString()),metaObject()->className(),Pip3lineConst::LERROR);
            return;
        }

        setData(file.readAll());
        file.close();
        QFileInfo finfo(fileName);
        setName(finfo.fileName());
    }

}

void ByteSourceAbstract::writeToFile(QString destFilename, QByteArray data)
{
    // by default write/overwrite the destination file
    if (!destFilename.isEmpty()) {
        QFile file(destFilename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            emit log(tr("Failed to open %1:\n %2").arg(destFilename).arg(file.errorString()),metaObject()->className(),Pip3lineConst::LERROR);
            return;
        }
        qint64 written = 0;
        while ((written = file.write(data)) > 0) {
            if (written == data.length())
                break;
            else
                data = data.mid(written - 1);
        };

        file.close();
    } else {
        emit log(tr("Destination file is empty, ignoring save."),metaObject()->className(),Pip3lineConst::LERROR);
    }
}

void ByteSourceAbstract::saveToFile(QString destFilename, quint64 startOffset, quint64 endOffset)
{
    quint64 t = size();
    if (startOffset < t && endOffset < t) {
        if (endOffset < startOffset) {
            t = startOffset - endOffset + 1;
        } else {
            t = endOffset - startOffset + 1;
        }
        writeToFile(destFilename, extract(startOffset,t));
    } else {
        emit log(tr("Invalid offsets, ignoring save."),metaObject()->className(),Pip3lineConst::LERROR);
    }
}

void ByteSourceAbstract::saveToFile(QString destFilename)
{ // by default we call the getRawData function, but some sources may have more efficient way to do that
  // or/and may not have this function implemented (for instance FileSource)
    writeToFile(destFilename, getRawData());
}

int ByteSourceAbstract::getViewOffset(quint64 realoffset)
{ // return the parameter value casted to int by default
    if (realoffset < INT_MAX) { // checking the int limit
        return realoffset;
    } else {
        emit log(tr("Offset value is hitting the INT_MAX limit for the gui view, consider implementing a discreet view"),metaObject()->className(),Pip3lineConst::LERROR);
        return -1;
    }
}

quint64 ByteSourceAbstract::getRealOffset(int viewOffset) // by default return the same value
{
    if (viewOffset < 0) {
        emit log(tr("View Offset is negative, returning zero"),metaObject()->className(),Pip3lineConst::LERROR);
        return 0;
    } else {
        return (quint64)viewOffset;
    }
}

quint64 ByteSourceAbstract::startingRealOffset()
{   // return 0 by default as the view size is the same as the data size
    // in case of restricted view, return the real offset of the first item of the view
    return 0;
}

bool ByteSourceAbstract::isOffsetValid(quint64 )
{
    return false;
}

