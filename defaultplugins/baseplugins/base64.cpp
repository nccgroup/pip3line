/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "base64.h"
#include "confgui/base64widget.h"
#include <QTextStream>

const QString Base64::id = "Base64";
const QStringList Base64::VARIATIONS = QStringList() << "RFC 2045" << "Safe Url" << ".Net (*Resource.axd)" << "Custom";
const QByteArray Base64::BASE64CHAR = QByteArray("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+/");

Base64::Base64()
{
    choosenVariation = STANDARD;
    char62 = '+';
    char63 = '/';
    paddingChar = '=';
    paddingType = DEFAULTPADDING;
}

Base64::~Base64()
{
}


QString Base64::name() const {
    return id;
}

QString Base64::description() const {
    return tr("base64 encoder/decoder (following RFC 2045)");
}

bool Base64::isTwoWays() {
    return true;
}

QHash<QString, QString> Base64::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();

    properties.insert(XMLVARIANT,QString::number((int)choosenVariation));
    if (choosenVariation == CUSTOM) {
        properties.insert(XMLPADDINGCHAR,QString::number((int)paddingChar));
        properties.insert(XMLCHAR62,QString::number((int)char62));
        properties.insert(XMLCHAR63,QString::number((int)char63));
        properties.insert(XMLPADDINGTYPE,QString::number((int)paddingType));
    }
    return properties;
}

bool Base64::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok = true;
    int val = propertiesList.value(XMLVARIANT).toInt(&ok);
    if (!ok || ( val != STANDARD && val != CUSTOM && val != SAFEURL && val != DOTNET)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLVARIANT),id);
    } else {
        setChoosenVariation((Variation) val);
    }

    if (choosenVariation == CUSTOM) {

        val = propertiesList.value(XMLPADDINGCHAR).toInt(&ok);
        if (!ok || val < 0x01 || val > 0xFF) {
            res = false;
            emit error(tr("Invalid value for %1").arg(XMLPADDINGCHAR),id);
        } else {
            res = setPaddingChar((char)val) && res;
        }

        val = propertiesList.value(XMLCHAR62).toInt(&ok);
        if (!ok || val < 0x01 || val > 0xFF) {
            res = false;
            emit error(tr("Invalid value for %1").arg(XMLCHAR62),id);
        } else {
            res = setChar62((char)val) && res;
        }

        val = propertiesList.value(XMLCHAR63).toInt(&ok);
        if (!ok || val < 0x01 || val > 0xFF) {
            res = false;
            emit error(tr("Invalid value for %1").arg(XMLCHAR63),id);
        } else {
            res = setChar63((char)val) && res;
        }

        val = propertiesList.value(XMLPADDINGTYPE).toInt(&ok);
        if (!ok || (val != DEFAULTPADDING && val != DOTNETPADDING && val != NOPADDING && val != CUSTOMPADDING)) {
            res = false;
            emit error(tr("Invalid value for %1").arg(XMLPADDINGTYPE),id);
        } else {
            setPaddingType((PaddingType)val);
        }
    }
    return res;

}

QWidget *Base64::requestGui(QWidget *parent)
{
    return new Base64Widget(this, parent);
}

QString Base64::help() const
{
    QString help;
    help.append("<p>Base64 decoder/encoder</p><p>Available variants: <ul><li>RFC 2045/4648</li> <li>Safe Url version</li><li>.Net version used by *Resource.axd scripts</li></ul>The default variant used is RFC 2045/4648</p>");
    return help;
}

Base64::Variation Base64::getChoosenVariation()
{
    return choosenVariation;
}

Base64::PaddingType Base64::getPaddingType()
{
    return paddingType;
}

char Base64::getChar62()
{
    return char62;
}

char Base64::getChar63()
{
    return char63;
}

char Base64::getPaddingChar()
{
    return paddingChar;
}

void Base64::setChoosenVariation(Base64::Variation val)
{
    if (choosenVariation != val) {
        choosenVariation = val;
        if (choosenVariation == Base64::STANDARD) {
            char62 = '+';
            char63 = '/';
            paddingChar = '=';
            paddingType = DEFAULTPADDING;
        } else if (choosenVariation == Base64::SAFEURL) {
            char62 = '-';
            char63 = '_';
            paddingType = NOPADDING;
        } else if (choosenVariation == Base64::DOTNET) {
            char62 = '-';
            char63 = '_';
            paddingType = DOTNETPADDING;
        }
        emit confUpdated();
    }
}

void Base64::setPaddingType(Base64::PaddingType val)
{
    if (paddingType != val) {
        paddingType = val;
        emit confUpdated();
    }
}

bool Base64::setChar62(char val)
{
    if (char62 != val) {
        if (val != char63 && val != paddingChar) {
            char62 = val;
            emit confUpdated();
        } else {
            emit error(tr("All three characters must be differents"),id);
            return false;
        }
        QByteArray charset = BASE64CHAR.mid(0,BASE64CHAR.size() -2);
        if (charset.contains(val)) {
            emit error(tr("This character is already included in the Base64 charset, cannot use it twice"),id);
            return false;
        }
    }
    return true;
}

bool Base64::setChar63(char val)
{
    if (char63 != val) {
        if (val != char62 && val != paddingChar) {
            char63 = val;
            emit confUpdated();
        } else {
            emit error(tr("All three characters must be differents"),id);
            return false;
        }
        QByteArray charset = BASE64CHAR.mid(0,BASE64CHAR.size() -2);
        if (charset.contains(val)) {
            emit error(tr("This character is already included in the Base64 charset, cannot use it twice"),id);
            return false;
        }
    }
    return true;
}

bool Base64::setPaddingChar(char val)
{
    if (paddingChar != val) {
        if (val != char62 && val != char63) {
            paddingChar = val;
            emit confUpdated();
        } else {
            emit error(tr("All three characters must be differents"),id);
            return false;
        }
        QByteArray charset = BASE64CHAR.mid(0,BASE64CHAR.size() -2);
        if (charset.contains(val)) {
            emit error(tr("This character is already included in the Base64 charset, cannot use it for padding"),id);
            return false;
        }
    }
    return true;
}


void Base64::transform(const QByteArray &input, QByteArray &output) {
    output.clear();
    if (input.isEmpty())
        return;

    if (wayValue == TransformAbstract::INBOUND) {
        output = input.toBase64();
        output.replace('+',char62);
        output.replace('/',char63);
        int count = output.count(DEFAULTPADDINGCHAR);
        output.replace(DEFAULTPADDINGCHAR,paddingChar);
        if (paddingType == DOTNETPADDING) {
            output.chop(count);
            output.append(QString::number(count).toUtf8());
        } else if (paddingType == NOPADDING) {
            output.chop(count);
        }

    } else {
        output = input;

        if (paddingType == DOTNETPADDING) {
            bool ok;
            int count = QString(output.right(1)).toInt(&ok);
            if (ok) {
                output.chop(1);
                for (int i =0 ; i < count; i++) {
                    output.append(DEFAULTPADDINGCHAR);
                }
            }
        } else if(paddingType == NOPADDING) {
            int num = output.size() % 4;
            for (int i = 0; i < num; i++) {
                output.append(DEFAULTPADDINGCHAR);
            }
        } else {
            output.replace(paddingChar,DEFAULTPADDINGCHAR);


            int rest = output.size() % 4;
            if (rest != 0) {

               emit error(tr("Invalid padding: missing %1 or %2 too many").arg(4 - rest).arg(rest),id);
            }
        }
        int count = 0;
        while (output.at(output.size() -1 - count) == DEFAULTPADDINGCHAR) {
            count++;
        }
        if (count < output.count(DEFAULTPADDINGCHAR)) {
            emit error(tr("There seem to be one or more padding characters in the middle of the stream. They will be considered invalid"),id);
        }

        output.replace(char62,"+");
        output.replace(char63,"/");
        QByteArray invalid;
        for (int i = 0; i < output.size() - count; i++) {
            if (!BASE64CHAR.contains(output[i])) {
                if (!invalid.contains(output.at(i)))
                    invalid.append(output.at(i));
            }
        }
        if (!invalid.isEmpty()) {
            invalid.replace('=',paddingChar);
            emit error(tr("Input string contains invalid Base64 character(s) : [%1].").arg(QString::fromUtf8(toPrintableString(invalid))),id);
        }
        output = QByteArray::fromBase64(output);
    }
}

