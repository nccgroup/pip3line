/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "closingstate.h"
#include <QSettings>
#include <QTime>
#include <QXmlStreamWriter>
#include <QDebug>

ClosingState::ClosingState()
{
    name = metaObject()->className();
 //   qDebug() << "Create" << this;
}

ClosingState::~ClosingState()
{

}

void ClosingState::run()
{
    genCloseElement();
}


