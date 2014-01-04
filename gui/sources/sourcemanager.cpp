/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "basicsource.h"
#include "sourcemanager.h"

SourceManager::SourceManager(QObject *parent) :
    QObject(parent)
{
}

QStringList SourceManager::getList()
{
    QStringList list;
    return list;
}

ByteSourceAbstract *SourceManager::getSource(const QString &)
{
    ByteSourceAbstract * bsa = NULL;

    return bsa;
}
