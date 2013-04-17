/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef COMPOSEDTRANSFORM_H
#define COMPOSEDTRANSFORM_H

#include "transformabstract.h"
#include "transformchain.h"

class LIBTRANSFORMSHARED_EXPORT ComposedTransform : public TransformAbstract
{
        Q_OBJECT
    public:
        ComposedTransform(TransformChain transformChain);
        ~ComposedTransform();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QWidget * getGui(QWidget * parent);
        QString help() const;
        static const QString id;
   private:
        TransformChain chain;
        bool twoWaysFlag;
        QList<TransformAbstract::Way> defaultWays;
};

#endif // COMPOSEDTRANSFORM_H
