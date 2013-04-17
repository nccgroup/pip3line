/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "xor.h"
#include "confgui/xorwidget.h"

const QString Xor::id = "Xor";

Xor::Xor()
{
    hexDecode = false;
}

Xor::~Xor()
{
}


QString Xor::name() const {
    return id;
}

QString Xor::description() const {
    return tr("Basic xor cipher");
}

bool Xor::isTwoWays() {
    return false;
}

QHash<QString, QString> Xor::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLKEY,QString(key.toBase64()));
    properties.insert(XMLFROMHEX,QString::number((int)hexDecode));

    return properties;
}

bool Xor::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLFROMHEX).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLFROMHEX),id);
    }
    setFromHex(val == 1);

    setKey(QByteArray::fromBase64(propertiesList.value(XMLKEY).toUtf8()));
    return res;
}

QWidget *Xor::requestGui(QWidget *parent)
{
    return new XorWidget(this, parent);
}

QString Xor::help() const
{
    QString help;
    help.append("<p>Basic xor cipher</p><p>Xor the given key with the input.</p>");
    return help;
}

void Xor::transform(const QByteArray &input, QByteArray &output){

    output.clear();

    QByteArray finalKey;
    if (hexDecode)
        finalKey = fromHex(key);
    else
        finalKey = key;

    if (finalKey.size() < 1) {
        emit error(tr("Key size is zero, nothing done."),id);
        return;
    }

    if (finalKey.size() < input.size()) {
        emit warning(tr("Key length (%1) is inferior to the data length (%2). Reusing the key multiple time.").arg(finalKey.size()).arg(input.size()),id);
    }
    for (int i = 0 ; i < input.size(); i++) {
        output.append(input.at(i) ^ finalKey.at(i % finalKey.size()));
    }
}

QByteArray Xor::getKey()
{
    return key;
}

void Xor::setKey(QByteArray val)
{
    key = val;
    emit confUpdated();
}

bool Xor::isFromHex()
{
    return hexDecode;
}

void Xor::setFromHex(bool val)
{
    hexDecode = val;
    emit confUpdated();
}
