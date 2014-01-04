/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "cut.h"
#include "confgui/cutwidget.h"

const QString Cut::id = "Cut";

Cut::Cut()
{
    from = 0;
    length = 1;
    everything = false;
}

Cut::~Cut()
{
}

QString Cut::name() const
{
    return id;
}

QString Cut::description() const
{
    return tr("Cut the input");
}

void Cut::transform(const QByteArray &input, QByteArray &output)
{
    output = input.mid(from, (everything ? -1 : length));
}

bool Cut::isTwoWays()
{
    return false;
}

QHash<QString, QString> Cut::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLFROM,QString::number(from));
    properties.insert(XMLLENGTH,QString::number(length));
    properties.insert(XMLEVERYTHING,QString::number(everything ? 1 : 0));
    return properties;
}

bool Cut::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLFROM).toInt(&ok);
    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLFROM),id);
    } else {
        res = setFromPos(val) && res;
    }

    val = propertiesList.value(XMLLENGTH).toInt(&ok);
    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLLENGTH),id);
    } else {
        res = setLength(val) && res;
    }

    val = propertiesList.value(XMLEVERYTHING).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLEVERYTHING),id);
    } else {
        setCutEverything(val == 1);
    }
    return res;
}

QWidget *Cut::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) CutWidget(this, parent);
    if (widget == NULL) {
        qFatal("Cannot allocate memory for CutWidget X{");
    }
    return widget;
}

QString Cut::help() const
{
    QString help;
    help.append("<p>This transformation will cut out part of the input</p><p>Nothing fancy here, this is just a convenience transformation.</p>");
    return help;
}

int Cut::getFromPos()
{
    return from;
}

int Cut::getLength()
{
    return length;
}

bool Cut::doCutEverything()
{
    return everything;
}

bool Cut::setFromPos(int val)
{
    if (val < 0) {
        emit error(tr("Invalid starting position: %1").arg(val),id);
        return false;
    }
    from = val;
    emit confUpdated();
    return true;
}

bool Cut::setLength(int val)
{
    if (val < 1) {
        emit error(tr("Invalid length: %1").arg(val),id);
        return false;
    }
    length = val;
    everything = false;
    emit confUpdated();
    return true;
}

void Cut::setCutEverything(bool val)
{
    everything = val;
    emit confUpdated();
}
