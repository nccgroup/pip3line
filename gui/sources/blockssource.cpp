/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "blockssource.h"
#include <QWidget>
#include <QDebug>

const int BlocksSource::BLOCK_MAX_SIZE = 0x8000000;

BlocksSource::BlocksSource(QObject *parent) :
    QObject(parent)
{
    readWrite = false;
    encodeOutput = true;
    decodeInput = true;
    gui = NULL;
    connect(this, SIGNAL(blockToBeSend(Block)), SLOT(sendBlock(Block)), Qt::QueuedConnection);
    //qDebug() << this << "created";
}

BlocksSource::~BlocksSource()
{
    //qDebug() << this << "Destroying" << isRunning();
    delete gui;
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

void BlocksSource::postBlockForSending(Block block)
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

