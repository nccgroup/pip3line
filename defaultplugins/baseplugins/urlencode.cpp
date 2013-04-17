/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "urlencode.h"
#include "confgui/urlencodewidget.h"

#include <QTextStream>

const QString UrlEncode::id = "Url Encode";
const QByteArray UrlEncode::TEXTCHAR = QByteArray("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-._~");

UrlEncode::UrlEncode()
{
    percentSign = '%';
}

UrlEncode::~UrlEncode()
{
}

QString UrlEncode::name() const{
    return id;
}

QString UrlEncode::description() const{
    return tr("URL encode the input");
}

bool UrlEncode::isTwoWays() {
    return true;
}

QHash<QString, QString> UrlEncode::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLPERCENTCHAR,QString::number((int)percentSign));
    properties.insert(XMLINCLUDE,QString(include.toBase64()));
    properties.insert(XMLEXCLUDE,QString(exclude.toBase64()));

    return properties;
}

bool UrlEncode::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLPERCENTCHAR).toInt(&ok);
    if (!ok || val < 0x00 || val > 0xFF) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLPERCENTCHAR),id);
    } else {
        setPercentSign((char)val);
    }

    include = QByteArray::fromBase64(propertiesList.value(XMLINCLUDE).toUtf8());
    exclude = QByteArray::fromBase64(propertiesList.value(XMLEXCLUDE).toUtf8());

    return res;
}

QWidget *UrlEncode::requestGui(QWidget *parent)
{
    return new UrlEncodeWidget(this, parent);
}

QString UrlEncode::help() const
{
    QString help;
    help.append("<p>URL encode the input (UTF-8)</p><p>Different tweaking options are availables.</p>");
    return help;
}

void UrlEncode::transform(const QByteArray &input, QByteArray &output) {
    if (wayValue == INBOUND){
        output = input.toPercentEncoding(exclude, include,percentSign);
    }
    else {
        output = QByteArray::fromPercentEncoding(input);
    }
}

char UrlEncode::getPercentSign()
{
    return percentSign;
}

QByteArray UrlEncode::getExclude()
{
    return exclude;
}

QByteArray UrlEncode::getInclude()
{
    return include;
}

void UrlEncode::setPercentSign(char val)
{
    percentSign = val;
    emit confUpdated();
}

void UrlEncode::setExclude(QByteArray vals)
{
    exclude = vals;
    emit confUpdated();
}

void UrlEncode::setInclude(QByteArray vals)
{
    include = vals;
    emit confUpdated();
}




