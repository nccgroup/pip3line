/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef IDENTITY_H
#define IDENTITY_H

#include <transformabstract.h>

// Example Transform class which will return the input as the output
// nothing really exiting, more detailed comments in the cpp file.

class Identity : public TransformAbstract
{
    public:
        Identity();
        ~Identity();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
};

#endif // IDENTITY_H
