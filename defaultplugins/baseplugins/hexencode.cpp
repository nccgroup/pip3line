/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "hexencode.h"
#include "confgui/hexencodewidget.h"

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
    if (!ok || (val != NORMAL && val != ESCAPED && val != CSTYLE && val != CSV)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLTYPE),id);
    } else {
        setType((Type)val);
    }

    return res;
}

QWidget *HexEncode::requestGui(QWidget *parent)
{
    return new HexEncodeWidget(this, parent);
}

QString HexEncode::help() const
{
    QString help;
    help.append("<p>Hexadecimal encoder</p><p>It is possible to choose a specific output format.<br>Currently available: <ul><li>Basic hexadecimal format</li><li>Escaped Hexadecimal format (i.e. \"\\xAD\")</li><li>C-Style array (i.e. \"{ 0xAD, 0x01 }\" )</li><li>CSV format, i.e. \"AD, 01\"</li></ul></p>");
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
