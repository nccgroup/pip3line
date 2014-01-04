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
    gui = NULL;
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
char BlocksSource::getSeparator() const
{
    return separator;
}

void BlocksSource::setSeparator(char value)
{
    separator = value;
}


void BlocksSource::onGuiDestroyed()
{
    gui = NULL;
}
bool BlocksSource::getBase64Applied() const
{
    return base64Applied;
}

void BlocksSource::setBase64Applied(bool value)
{
    base64Applied = value;
}

