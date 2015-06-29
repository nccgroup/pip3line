/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/


#ifndef SHA3_256_H
#define SHA3_256_H

#include "transformabstract.h"

class Sha3_256 : public TransformAbstract
{
    Q_OBJECT
    public:
        explicit Sha3_256();
        ~Sha3_256();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString help() const;
};

#endif // SHA3_256_H
