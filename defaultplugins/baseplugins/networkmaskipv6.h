/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef NETWORKMASKIPV6_H
#define NETWORKMASKIPV6_H

#include <transformabstract.h>

class NetworkMaskIPv6 : public TransformAbstract
{
        Q_OBJECT
    public:
        NetworkMaskIPv6();
        ~NetworkMaskIPv6() {}
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString inboundString() const;
        QString outboundString() const;
        QString help() const;
};

#endif // NETWORKMASKIPV6_H
