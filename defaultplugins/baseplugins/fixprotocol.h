/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef FIXPROTOCOL_H
#define FIXPROTOCOL_H

#include <transformabstract.h>
#include <QXmlQuery>

class FixProtocol : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit FixProtocol();
        ~FixProtocol();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString help() const;
    private:
        QByteArray translateField(QByteArray val);
        QXmlQuery query;
};

#endif // FIXPROTOCOL_H
