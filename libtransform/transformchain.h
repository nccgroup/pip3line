/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TRANSFORMCHAIN_H
#define TRANSFORMCHAIN_H

#include "libtransform_global.h"
#include "transformabstract.h"
#include "commonstrings.h"
#include <QList>

class LIBTRANSFORMSHARED_EXPORT TransformChain : public QList<TransformAbstract *>
{
    public:
        explicit TransformChain();
        TransformChain(TransformChain const &other);
        TransformChain& operator = (TransformChain const &other);
        ~TransformChain();
        void setName(const QString &name);
        QString getName() const;
        void setDescription(const QString &desc);
        QString getDescription() const;
        void setHelp(const QString &help);
        QString getHelp() const;
        void setFormat(OutputFormat val);
        OutputFormat getFormat() const;
    private:

        QString name;
        QString description;
        QString help;
        OutputFormat preferredFormat;
};

#endif // TRANSFORMCHAIN_H
