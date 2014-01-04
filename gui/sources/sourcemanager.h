/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SOURCEMANAGER_H
#define SOURCEMANAGER_H

#include <QObject>
#include <QStringList>
#include <QString>
#include "bytesourceabstract.h"

class SourceManager : public QObject
{
        Q_OBJECT
    public:
        explicit SourceManager(QObject *parent = 0);
        QStringList getList();
        ByteSourceAbstract *getSource(const QString &name);
};

#endif // SOURCEMANAGER_H
