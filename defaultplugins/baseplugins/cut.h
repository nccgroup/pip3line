/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef CUT_H
#define CUT_H

#include "transformabstract.h"

class Cut : public TransformAbstract
{
    Q_OBJECT

    public:
        explicit Cut();
        ~Cut();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QString help() const;

        int getFromPos();
        int getLength();
        bool doCutEverything();
        bool setFromPos(int val);
        bool setLength(int val);
        void setCutEverything(bool val);
    private:
        int from;
        int length;
        bool everything;
};

#endif // CUT_H
