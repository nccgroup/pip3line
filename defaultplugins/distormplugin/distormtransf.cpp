/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "distormtransf.h"
#include <distorm.h>
#include "distormwidget.h"
#include <QDebug>

const QString DistormTransf::id = "Distorm";
const QString DistormTransf::XMLASMTYPE = "AsmType";
const QString DistormTransf::XMLMAXINSTRUCTION = "MaxInstruction";

DistormTransf::DistormTransf()
{
    codeOffset = 0;
    asmType = A32Bits;
    maxInstruction = 200;
}

DistormTransf::~DistormTransf()
{
}

QString DistormTransf::name() const
{
    return id;
}

QString DistormTransf::description() const
{
    return tr("Distorm disassembler");
}

void DistormTransf::transform(const QByteArray &input, QByteArray &output)
{
    QByteArray offset;
    _DecodeType dt = Decode32Bits;
    int maxOffsetSize = sizeof(_OffsetType);
    switch (asmType) {
        case A16Bits:
            dt = Decode16Bits;
            maxOffsetSize = 4;
            break;
        case A32Bits:
            dt = Decode32Bits;
            maxOffsetSize = 8;
            break;
        case A64Bits:
            dt = Decode64Bits;
            maxOffsetSize = 16;
            break;
    }


    unsigned int decodedInstruction = 0;

    _DecodedInst * resultInstr = new _DecodedInst[maxInstruction];
    _DecodeResult result = distorm_decode64((_OffsetType)codeOffset, (const unsigned char*)input.constData(), input.size(), dt, resultInstr, maxInstruction, &decodedInstruction);
    if (result == DECRES_SUCCESS || result == DECRES_MEMORYERR) {
        if (result == DECRES_MEMORYERR) {
            emit error(tr("Result instruction array not big enough (%1)").arg(maxInstruction), id);
        }

        for (uint i = 0; i < decodedInstruction; i++) {
            _DecodedInst instruction = resultInstr[i];

            output.append("0x");
            offset = QByteArray::number((qulonglong)instruction.offset,16);

            int offsetSize = offset.size();
            for (int j = 0; j < maxOffsetSize - offsetSize; j++) {
                offset.prepend('0');
            }
            output.append(offset);
            output.append(' ');
            output.append((char *)instruction.mnemonic.p,instruction.mnemonic.length);
            output.append(' ');
            output.append((char *)instruction.operands.p,instruction.operands.length);
            output.append('\n');
        }
    } else {
        emit error(tr("There was an error during the disassembling"), id);
    }
}

bool DistormTransf::isTwoWays()
{
    return false;
}

QWidget *DistormTransf::requestGui(QWidget *parent)
{
    return new(std::nothrow) DistormWidget(this, parent);
}

QHash<QString, QString> DistormTransf::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLOFFSET,QString::number(codeOffset));
    properties.insert(XMLASMTYPE, QString::number(asmType));
    properties.insert(XMLMAXINSTRUCTION,QString::number(maxInstruction));
    return properties;
}

bool DistormTransf::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);

    bool ok = true;
    quint64 val = 0;

    val = propertiesList.value(XMLOFFSET).toULongLong(&ok);
    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLOFFSET),id);
    } else {
        setOffset(val);
    }

    int val1 = propertiesList.value(XMLASMTYPE).toInt(&ok);
    if (!ok || (val1 != A16Bits && val1 != A32Bits && val1 != A64Bits) ) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLASMTYPE),id);
    } else {
        setDecodeType((ASMType) val1);
    }

    uint val2 = propertiesList.value(XMLMAXINSTRUCTION).toUInt(&ok);
    if (!ok ) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLMAXINSTRUCTION),id);
    } else {
        setMaxInstruction(val2);
    }

    return res;
}

QString DistormTransf::help() const
{
    QString help;
    help.append("<p>Distorm disassembler</p><p>Disassemble a chunk of bytes using the Distorm library</p>");
    quint32 version = distorm_version();
    quint32 major = version >> 16;
    quint32 minor = ((version & 0x00FF00) >> 8);
    help.append(tr("<p>Distorm version in use: %1.%2</p>").arg(major).arg(minor));
    return help;
}

void DistormTransf::setDecodeType(DistormTransf::ASMType val)
{
    if (asmType != val) {
        asmType = val;
        emit confUpdated();
    }
}

DistormTransf::ASMType DistormTransf::getDecodeType() const
{
    return asmType;
}

void DistormTransf::setOffset(quint64 val)
{
    if (codeOffset != val) {
        codeOffset = val;
        emit confUpdated();
    }
}

quint64 DistormTransf::getOffset() const
{
    return codeOffset;
}

void DistormTransf::setMaxInstruction(uint val)
{
    if (maxInstruction != val) {
        maxInstruction = val;
        emit confUpdated();
    }
}

uint DistormTransf::getMaxInstruction() const
{
    return maxInstruction;
}

