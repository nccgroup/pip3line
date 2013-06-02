/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef DISTORMTRANSF_H
#define DISTORMTRANSF_H

#include <transformabstract.h>

class DistormTransf : public TransformAbstract
{
        Q_OBJECT
    public:
        static const QString id;
        enum ASMType { A16Bits = 2, A32Bits = 4, A64Bits = 8};
        explicit DistormTransf();
        ~DistormTransf();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QWidget * requestGui(QWidget * parent);
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);

        void setDecodeType(ASMType val);
        ASMType getDecodeType() const;
        void setOffset(quint64 val);
        quint64 getOffset() const;
        void setMaxInstruction(uint val);
        uint getMaxInstruction() const;

        QString help() const;
    private:
        static const QString XMLASMTYPE;
        static const QString XMLMAXINSTRUCTION;
        quint64 codeOffset;
        ASMType asmType;
        uint maxInstruction;
};

#endif // DISTORM_H
