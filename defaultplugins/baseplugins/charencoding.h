/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef CHARENCODING_H
#define CHARENCODING_H

#include "transformabstract.h"

class CharEncoding : public TransformAbstract
{
    Q_OBJECT
    
    public:
        explicit CharEncoding();
        ~CharEncoding();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QString help() const;

        QByteArray getCodecName();
        bool setCodecName(QByteArray val);
    private:
        QByteArray codecName;
};

#endif // CHARENCODING_H
