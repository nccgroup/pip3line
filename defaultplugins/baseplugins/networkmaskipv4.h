/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef NETWORKMASK_H
#define NETWORKMASK_H

#include <transformabstract.h>

class NetworkMaskIPv4 : public TransformAbstract
{
        Q_OBJECT
    public:
        NetworkMaskIPv4();
        ~NetworkMaskIPv4() {}
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString inboundString() const;
        QString outboundString() const;
        QString help() const;
};

#endif // NETWORKMASK_H
