/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MD4_H
#define MD4_H

#include "transformabstract.h"

class Md4 : public TransformAbstract
{
    Q_OBJECT
    
    public:
        explicit Md4() {}
        ~Md4() {}
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;

        QString help() const;

};

#endif // MD4_H
