/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef INTERCEPSOURCE_H
#define INTERCEPSOURCE_H

#include "basicsource.h"

#include <QList>

class IntercepSource : public BasicSource
{
        Q_OBJECT
    public:
        explicit IntercepSource(QObject *parent = 0);
        ~IntercepSource();
        QString name();
        void setData(QByteArray, quintptr);
        void clear(quintptr source = INVALID_SOURCE);
        int preferredTabType();
        bool isReadableText();

    private:
        Q_DISABLE_COPY(IntercepSource)
        QList<QByteArray> dataList;

};

#endif // INTERCEPSOURCE_H
