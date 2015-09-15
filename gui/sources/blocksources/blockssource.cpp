/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "blockssource.h"
#include <QWidget>
#include <QDebug>

Block::Block(QByteArray data, int sourceid):
    data(data),
    sourceid(sourceid)
{
}

Block::~Block()
{
    //qDebug() << "Destroying" << this;
}

QByteArray Block::getData() const
{
    return data;
}

void Block::setData(const QByteArray &value)
{
    data = value;
}

int Block::getSourceid() const
{
    return sourceid;
}

void Block::setSourceid(int value)
{
    sourceid = value;
}


int BlocksSource::currentid = 0;
QMutex BlocksSource::idlock;
QHash<int,BlocksSource *> BlocksSource::idSourceTable;
const int BlocksSource::BLOCK_MAX_SIZE = 0x8000000;

BlocksSource::BlocksSource(QObject *parent) :
    QObject(parent)
{
    readWrite = false;
    encodeOutput = true;
    decodeInput = true;
    gui = NULL;
    type = INVALID_TYPE;
    sid = BlocksSource::newSourceID(this);
    connect(this, SIGNAL(blockToBeSend(Block *)), SLOT(sendBlock(Block *)), Qt::QueuedConnection);
    //qDebug() << this << "created";
}

BlocksSource::~BlocksSource()
{
    //qDebug() << this << "Destroying" << isRunning();
    delete gui;
    BlocksSource::releaseID(sid);
}

QWidget *BlocksSource::getGui(QWidget * parent)
{
    if (gui == NULL) {
        gui = requestGui(parent);
        if (gui != NULL) {
            connect(gui, SIGNAL(destroyed()), this, SLOT(onGuiDestroyed()), Qt::UniqueConnection);
        }
    }
    return gui;
}

QWidget *BlocksSource::requestGui(QWidget *)
{
    return NULL;
}

BlocksSource::BSOURCETYPE BlocksSource::getType() const
{
    return type;
}

bool BlocksSource::getDecodeinput() const
{
    return decodeInput;
}

void BlocksSource::setDecodeinput(bool value)
{
    decodeInput = value;
}

bool BlocksSource::getEncodeOutput() const
{
    return encodeOutput;
}

void BlocksSource::setEncodeOutput(bool value)
{
    encodeOutput = value;
}

char BlocksSource::getSeparator() const
{
    return separator;
}

void BlocksSource::setSeparator(char value)
{
    separator = value;
}

void BlocksSource::postBlockForSending(Block *block)
{
    emit blockToBeSend(block);
}

void BlocksSource::restart()
{
    stopListening();
    startListening();
}


void BlocksSource::onGuiDestroyed()
{
    gui = NULL;
}


int BlocksSource::newSourceID(BlocksSource *source)
{
    QMutexLocker lock(&idlock);

    if (currentid == INT_MAX) {
        qWarning() << "BlocksSource IDs is wrapping";
        if (idSourceTable.size() == INT_MAX) {
            qFatal("BlocksSource IDs table is full ... really? X{");
        }
        currentid = 0;
    }

    while (idSourceTable.contains(currentid++));

    idSourceTable.insert(currentid, source);
    return currentid;
}

void BlocksSource::releaseID(int id)
{
    QMutexLocker lock(&idlock);
    idSourceTable.remove(id);
}

BlocksSource *BlocksSource::getSourceObject(int id)
{
    QMutexLocker lock(&idlock);
    qWarning() << QObject::tr("No Source object for id %1 T_T").arg(id);
    return idSourceTable.value(id,NULL);
}
