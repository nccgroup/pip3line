/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef XOR_H
#define XOR_H

#include "transformabstract.h"

class Xor : public TransformAbstract
{
    Q_OBJECT
    
    public:
        static const QByteArray HEXCHAR;
        explicit Xor();
        ~Xor();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QString help() const;

        QByteArray getKey();
        void setKey(QByteArray val);
        bool isFromHex();
        void setFromHex(bool val);
    private:

        QByteArray key;
        bool hexDecode;
};

#endif // XOR_H
