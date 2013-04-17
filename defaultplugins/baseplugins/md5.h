/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MD5_H
#define MD5_H

#include "transformabstract.h"

class Md5 : public TransformAbstract
{
    Q_OBJECT
    
public:
    explicit Md5() {}
    ~Md5() {}
    QString name() const;
    QString description() const;
    void transform(const QByteArray &input, QByteArray &output);
    bool isTwoWays();
    static const QString id;
    QString help() const;
};

#endif // MD5_H
