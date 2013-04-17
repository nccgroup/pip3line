/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "numbertochar.h"

const QString NumberToChar::id = "short to Char";

NumberToChar::NumberToChar()
{

}

NumberToChar::~NumberToChar()
{
}

QString NumberToChar::name() const
{
    return id;
}

QString NumberToChar::description() const
{
    return tr("Convert list of signed short to an array of char.");
}

void NumberToChar::transform(const QByteArray &input, QByteArray &output)
{
    int val;
    bool ok;
    if (wayValue == TransformAbstract::INBOUND) {
        QByteArray num;
        for (int i = 0; i < input.size(); i++) {
            if (input.at(i) ==  '-' || (input.at(i) > 47 && input.at(i) < 58)) {
                num.append(input.at(i));
            }
            else {
                val = num.toInt(&ok);
                if (ok) {
                    output.append((char)val);
                }
                num.clear();
            }
        }
    } else {
        for (int i = 0; i < input.size(); i++) {
            output.append(QByteArray::number((int)input.at(i))).append('_');
        }
    }
}

bool NumberToChar::isTwoWays()
{
    return true;
}

QString NumberToChar::help() const
{
    QString help;
    help.append("<p>Convert list of signed short to an array of char.</p><p> Input delimiters can be anything apart from the sign character '-' and numbers chararacters.</p><p>Any other characters will be silently ignored</p>");
    return help;
}

