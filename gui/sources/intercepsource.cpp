/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/


#include "intercepsource.h"
#include "interceptwidget.h"
#include "blockssource.h"
#include <QDebug>
#include <QAbstractItemModel>
#include <QSize>
#include <QVariant>
#include <QHBoxLayout>
#include <QPushButton>

const QFont PayloadModel::RegularFont = QFont("Courier New",10);
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
            return QVariant(payl->timestamp);
        else if (column == PAYLOAD_COLUMN)
            return QVariant(payl->payload.toHex());
        else
            return QVariant();
    } else if (role == Qt::DecorationRole) {
        if (column == DIRECTION_COLUMN)
                    return payl->direction == Block::SOURCE ?
                                QVariant(QIcon(":/Images/icons/arrow-right-3-mod.png")):
                                QVariant(QIcon(":/Images/icons/arrow-left-3.png"));
    }else if (role == Qt::BackgroundRole) {
        return (payl->originalPayload == payl->payload ? QVariant() : QVariant(QColor(218,160,255,255)));
    } else if (role == Qt::FontRole) {
        return QVariant(RegularFont);
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


IntercepSource::IntercepSource(QObject *parent) :
    ByteSourceAbstract(parent)
{
    _name = tr("Intercept source");
    capabilities = CAP_RESIZE | CAP_SEARCH | CAP_WRITE | CAP_COMPARE;
    currentBlockSource = NULL;
    currentPayload = NULL;
    processingPayload = false;
    intercepting = false;
    model = new(std::nothrow) PayloadModel();
    if (model == NULL) {
        qFatal("Cannot allocate memory for PayloadModel X{");
    }
}

IntercepSource::~IntercepSource()
{
    delete currentBlockSource;
}

QString IntercepSource::description()
{
    return "Interceptor";
}

void IntercepSource::setData(QByteArray data, quintptr source)
{
    if (currentPayload != NULL) {
        currentPayload->payload = data;
        emit updated(source);
    }
}

QByteArray IntercepSource::getRawData()
{
    if (currentPayload != NULL) {
        return currentPayload->payload;
    }
    return QByteArray();
}

quint64 IntercepSource::size()
{
    if (currentPayload != NULL) {
        return currentPayload->payload.size();
    }
    return 0;
}

QByteArray IntercepSource::extract(quint64 offset, int length)
{
    if (!validateOffsetAndSize(offset, 0) || currentPayload == NULL)
        return QByteArray();

    if (length < 0) {
        offset = (offset + length + 1);
        length = qAbs(length);
    }

    return currentPayload->payload.mid(offset,length);
}

char IntercepSource::extract(quint64 offset)
{
    if (!validateOffsetAndSize(offset, 1) || currentPayload != NULL) {
        return '\00';
    }
    return currentPayload->payload.at(offset);
}

void IntercepSource::replace(quint64 offset, int length, QByteArray repData, quintptr source)
{
    if (currentPayload != NULL || (!_readonly && validateOffsetAndSize(offset, length))) {
        historyAddReplace(offset, currentPayload->payload.mid(offset,length),repData);
        currentPayload->payload.replace(offset,length,repData);
        emit updated(source);
        emit sizeChanged();
    }
}

void IntercepSource::insert(quint64 offset, QByteArray repData, quintptr source)
{
    if (currentPayload != NULL || (!_readonly && validateOffsetAndSize(offset, 0))) {
        historyAddInsert(offset,repData);
        currentPayload->payload.insert(offset, repData);
        emit updated(source);
        emit sizeChanged();
    }
}

void IntercepSource::remove(quint64 offset, int length, quintptr source)
{
    if (currentPayload != NULL || (!_readonly && validateOffsetAndSize(offset, 0))) {
        historyAddRemove(offset,currentPayload->payload.mid(offset,length));
        currentPayload->payload.remove(offset, length);
        emit updated(source);
        emit sizeChanged();
    }
}

void IntercepSource::clear(quintptr source)
{
    currentPayload = NULL;
    historyClear();
    model->clear();
    emit updated(source);
}

int IntercepSource::getViewOffset(quint64 realoffset)
{
    if (currentPayload == NULL) {
        return -1;
    }

    if (realoffset > (quint64) currentPayload->payload.size()) {
        emit log(tr("Offset too large: %1").arg(realoffset),metaObject()->className(), Pip3lineConst::LERROR);
        return - 1;
    }
    return (int)realoffset;
}

int IntercepSource::preferredTabType()
{
    return TAB_GENERIC;
}

bool IntercepSource::isOffsetValid(quint64 offset)
{
    if (currentPayload == NULL) {
        return false;
    }

    return offset < ((quint64)currentPayload->payload.size());
}

bool IntercepSource::isReadableText()
{
    return false;
}

PayloadModel *IntercepSource::getModel()
{
    return model;
}

void IntercepSource::setCurrentBlockSource(BlocksSource *value)
{
    if (currentBlockSource != NULL) {
        disconnect(currentBlockSource, SIGNAL(blockReceived(Block)),this, SLOT(addNewBlock(Block)));
        disconnect(currentBlockSource, SIGNAL(destroyed()), this, SLOT(onBlockSourceDeleted()));
        disconnect(currentBlockSource, SIGNAL(error(QString,QString)), this, SLOT(logError(QString,QString)));
        disconnect(currentBlockSource, SIGNAL(status(QString,QString)), this, SLOT(logStatus(QString,QString)));
    }
    currentBlockSource = value;
    connect(currentBlockSource, SIGNAL(blockReceived(Block)), SLOT(addNewBlock(Block)));
    connect(currentBlockSource, SIGNAL(destroyed()), SLOT(onBlockSourceDeleted()));
    connect(currentBlockSource, SIGNAL(error(QString,QString)), SLOT(logError(QString,QString)));
    connect(currentBlockSource, SIGNAL(status(QString,QString)), SLOT(logStatus(QString,QString)));
}

void IntercepSource::addNewBlock(Block block)
{
    Payload * payl = new(std::nothrow) Payload();
    if (payl == NULL) {
        qFatal("Cannot allocate memory for Payload X{");
    }
    payl->payload = block.data;
    payl->originalPayload = block.data;
    payl->source = block.source;
    payl->direction = block.direction;
    payl->timestamp = QDateTime::currentDateTime();
    payl->sourceid = block.sourceid;
    if (intercepting && processingPayload) {
        payloadQueue.enqueue(payl);
    } else {
        configureCurrentPayload(payl);
        model->addPayload(payl);
        processingPayload = true;
        if (!intercepting)
            currentBlockSource->postBlockForSending(block);
    }
}

void IntercepSource::forwardCurrentBlock()
{
    if (currentPayload != NULL && processingPayload) {
        Block block;
        block.data = currentPayload->payload;
        block.source = currentPayload->source;
        block.direction = currentPayload->direction;
        block.sourceid = currentPayload->sourceid;
        currentBlockSource->postBlockForSending(block);
        if (payloadQueue.size() > 0) {
            Payload * payl = payloadQueue.dequeue();
            model->addPayload(payl);
            configureCurrentPayload(payl);
        } else {
            processingPayload = false;
        }
    }
}

void IntercepSource::logError(QString mess, QString source)
{
    emit log(mess, source, Pip3lineConst::LERROR);
}

void IntercepSource::logStatus(QString mess, QString source)
{
    emit log(mess, source, Pip3lineConst::LSTATUS);
}

void IntercepSource::onBlockSourceDeleted()
{
    QObject * deletedSource = sender();
    if (currentBlockSource == deletedSource)
        currentBlockSource = NULL;
    else
        qCritical() << "[IntercepSource::onBlockSourceDeleted] deleted object does not correspond to current one";
}

void IntercepSource::configureCurrentPayload(Payload *payl)
{
    if (currentPayload != NULL) {
        currentPayload->history = history;
    }
    currentPayload = payl;
    history = currentPayload->history;
    emit updated(ByteSourceAbstract::INVALID_SOURCE);
}


QWidget *IntercepSource::requestGui(QWidget *parent,ByteSourceAbstract::GUI_TYPE type)
{
    QWidget * iw = NULL;

    if (type == ByteSourceAbstract::GUI_UPPER_VIEW) {
        iw = new(std::nothrow) InterceptWidget(this, parent);
        if (iw == NULL) {
            qFatal("Cannot allocate memory for InterceptWidget X{");
        }
    } else if (type == ByteSourceAbstract::GUI_BUTTONS) {
        iw = new(std::nothrow) QWidget(parent);
        if (iw == NULL) {
            qFatal("Cannot allocate memory for QWidget X{");
        }

        QHBoxLayout * layout = new(std::nothrow) QHBoxLayout(iw);
        if (layout == NULL) {
            qFatal("Cannot allocate memory for QHBoxLayout X{");
        }

        iw->setLayout(layout);

        QPushButton * play = new(std::nothrow) QPushButton(iw);
        if (play == NULL) {
            qFatal("Cannot allocate memory for QPushButton X{");
        }

        play->setIcon(QIcon(":/Images/icons/media-seek-forward-7.png"));
        play->setMaximumWidth(25);
        play->setToolTip(tr("Forward current payload"));
        play->setFlat(true);
        connect(play, SIGNAL(clicked()), SLOT(forwardCurrentBlock()));
        layout->addWidget(play);

        QPushButton * intercep = new(std::nothrow) QPushButton(iw);
        if (intercep == NULL) {
            qFatal("Cannot allocate memory for QPushButton X{");
        }

        intercep->setIcon(QIcon(":/Images/icons/system-switch-user-3.png"));
        intercep->setMaximumWidth(25);
        intercep->setToolTip(tr("enable/disable interception"));
        intercep->setFlat(true);
        intercep->setCheckable(true);
        intercep->setChecked(intercepting);
        connect(intercep, SIGNAL(toggled(bool)), SLOT(setIntercepting(bool)));
        layout->addWidget(intercep);

        QPushButton * clearAll = new(std::nothrow) QPushButton(iw);
        if (clearAll == NULL) {
            qFatal("Cannot allocate memory for QPushButton X{");
        }

        clearAll->setIcon(QIcon(":/Images/icons/edit-clear-2.png"));
        clearAll->setMaximumWidth(25);
        clearAll->setToolTip(tr("Clear all data"));
        clearAll->setFlat(true);
        connect(clearAll, SIGNAL(clicked()), SLOT(clear()));
        layout->addWidget(clearAll);
    }

    return iw;
}

bool IntercepSource::validateOffsetAndSize(quint64 offset, int length)
{
    if (currentPayload == NULL)
        return false;

    if (offset > (quint64)currentPayload->payload.size()) { // hitting the limit data size
        emit log(tr("Offset too large: %1 length: %2").arg(offset).arg(length),metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }

    if (length < 0 ) { // trivial
        emit log(tr("Negative length: %2").arg(length),metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }

    if ((quint64)(INT_MAX - length) < offset) { // // hitting the limit
        emit log(tr("Length too large, hitting the int MAX limit. offset: %1 length: %2").arg(offset).arg(length),metaObject()->className(),Pip3lineConst::LWARNING);
        return false;
    }

    if (offset + (quint64)length > (quint64)currentPayload->payload.size()) { // this is behond the end of the data
        emit log(tr("Length too large for the data set. offset: %1 length: %2").arg(offset).arg(length),metaObject()->className(),Pip3lineConst::LWARNING);
        return false;
    }

    return true;
}
bool IntercepSource::getIntercepting() const
{
    return intercepting;
}

void IntercepSource::setIntercepting(bool value)
{
    if (value != intercepting) {
        intercepting = value;
        processingPayload = false;
    }
}

