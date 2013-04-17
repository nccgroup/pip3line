/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MYSQLCONCATV2_H
#define MYSQLCONCATV2_H

#include "transformabstract.h"

class MySqlConcatv2 : public TransformAbstract
{
    Q_OBJECT
    public:
        MySqlConcatv2() {}
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString help() const;
};

#endif // MYSQLCONCATV2_H
