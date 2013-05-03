/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "hexencode.h"
#include "confgui/hexencodewidget.h"
#include <QDebug>

const QString HexEncode::id = "Hexadecimal";

HexEncode::HexEncode()
{
    type = NORMAL;
}

HexEncode::~HexEncode()
{
}

QString HexEncode::name() const {
    return id;
}

QString HexEncode::description() const {
    return tr("Char hexadecimal encoding");
}

bool HexEncode::isTwoWays() {
    return true;
}

QHash<QString, QString> HexEncode::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLTYPE,QString::number(type));
    return properties;
}

bool HexEncode::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLTYPE).toInt(&ok);
    if (!ok || (val != NORMAL && val != ESCAPED_MIXED && val != ESCAPED && val != CSTYLE && val != CSV)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLTYPE),id);
    } else {
        setType((Type)val);
    }

    return res;
}

QWidget *HexEncode::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) HexEncodeWidget(this, parent);
    if (widget == NULL) {
        qFatal("Cannot allocate memory for HexEncodeWidget X{");
    }
    return widget;
}

QString HexEncode::help() const
{
    QString help;
    help.append("<p>Hexadecimal encoder</p>");
    help.append("<p>It is possible to choose a specific output format.");
    help.append("<br>Currently available: <ul>");
    help.append("<li>Basic hexadecimal </li>");
    help.append("<li>Escaped Hexadecimal (i.e. \"\\xAD\")</li>");
    help.append("<li>Escaped Hexadecimal Mixed (i.e. \"abc123\\xAD\")*</li>");
    help.append("<li>C-Style array (i.e. \"{ 0xAD, 0x01 }\" )</li>");
    help.append("<li>CSV format, i.e. \"AD, 01\"</li></ul>");
    help.append("* Escaped Hexadecimal Mixed is encoding only non-printable characters, and while decoding just append any incorrect value to the output (instead of discarding it)");
    help.append("</p>");
    return help;
}

void HexEncode::transform(const QByteArray &input, QByteArray &output) {
    output.clear();
    if (input.isEmpty())
        return;
    QByteArray temp;
    int i = 0;
    if (wayValue == TransformAbstract::INBOUND) {
        temp = input.toHex();
        switch (type) {
            case NORMAL:
                output = temp;
                break;
            case ESCAPED_MIXED:
                for (i = 0; i < input.size(); i += 1) {
                    char c = input.at(i);
                    if (c < '\x20' || c > '\x7e')
                        output.append(QByteArray("\\x").append(QByteArray(1,c).toHex()));
                    else
                        output.append(c);
                }
                break;
            case ESCAPED:
                for (i = 0; i < temp.size(); i += 2) {
                    output.append(QString("\\x%1%2").arg(temp.at(i)).arg(temp.at(i+1)));
                }
                break;
            case CSTYLE:
                output.append("{");
                for (i = 0; i < temp.size(); i += 2) {
                    output.append(QString(" 0x%1%2,").arg(temp.at(i)).arg(temp.at(i+1)));
                }
                output.chop(1);
                output.append(" }");
                break;
            case CSV:
                for (i = 0; i < temp.size(); i += 2) {
                    output.append(QString("%1%2,").arg(temp.at(i)).arg(temp.at(i+1)));
                }
                break;
        }
    }
    else {
        QString HEXCHAR("abcdefABCDEF0123456789");
        switch (type) {
            case NORMAL:
                output = fromHex(input);
                break;
            case ESCAPED_MIXED:
                qDebug() << "Escape lenient";
                while (i < input.size()) {
                    if (input.at(i) == '\\' && i < input.size() - 3 &&  input.at(i+1) == 'x') {
                        if (HEXCHAR.contains(input.at(i+2)) && HEXCHAR.contains(input.at(i+3))) {
                            temp.append(QByteArray::fromHex(QByteArray(1,input.at(i+2)).append(input.at(i+3))));
                            i += 4;
                            continue;
                        }
                    }
                    temp.append(input.at(i));
                    i++;
                }
                output = temp;
                break;
            case ESCAPED:
                while (i < input.size()) {
                    if (input.at(i) == '\\' && i < input.size() - 3 &&  input.at(i+1) == 'x') {
                        if (HEXCHAR.contains(input.at(i+2)) && HEXCHAR.contains(input.at(i+3))) {
                            temp.append(input.at(i+2)).append(input.at(i+3));
                            i += 4;
                            continue;
                        }
                    }
                    i++;
                }
                output = fromHex(temp);
                break;
            case CSTYLE:
                while (i < input.size()) {
                    if (input.at(i) == '0' && i < input.size() - 3 &&  input.at(i+1) == 'x') {
                        if (HEXCHAR.contains(input.at(i+2)) && HEXCHAR.contains(input.at(i+3))) {
                            temp.append(input.at(i+2)).append(input.at(i+3));
                            i += 4;
                            continue;
                        }
                    }
                    i++;
                }
                output = fromHex(temp);
                break;
            case CSV:
                QList<QByteArray> list = input.split(',');
                for (i = 0; i < list.size(); i++) {
                    temp.append(list.at(i).trimmed());
                }
                output = fromHex(temp);
                break;
        }
    }
}

HexEncode::Type HexEncode::getType()
{
    return type;
}

void HexEncode::setType(HexEncode::Type ntype)
{
    type = ntype;
    emit confUpdated();
}
