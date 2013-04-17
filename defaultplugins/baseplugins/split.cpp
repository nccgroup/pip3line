/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "split.h"
#include "confgui/splitwidget.h"
#include <QTextStream>

const QString Split::id = "Split";

Split::Split()
{
    separator = ':';
    group = 0;
    allGroup = false;
}

Split::~Split()
{
}

QString Split::name() const
{
    return id;
}

QString Split::description() const
{
    return tr("Split the input");
}

void Split::transform(const QByteArray &input, QByteArray &output)
{
    output.clear();
    QTextStream soutput(&output);

    QList<QByteArray> list = input.split(separator);

    if (list.size() > 1 && allGroup) {
        for (int i = 0; i < list.size() - 1 ; i++) {
            if (!list.at(i).isEmpty())
                soutput << list.at(i) << endl;
        }
            soutput << list.at(list.size() - 1);
    } else if (list.size() > 1) {
        if (group > list.size() - 1) {
            emit error(tr("Only %1 group(s) found, cannot return group number %2").arg(list.size()).arg(group),id);
        } else {
            soutput << list.at(group);
        }
    } else {
        emit warning("No separator found",id);
    }
}

bool Split::isTwoWays()
{
    return false;
}

QHash<QString, QString> Split::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLSEPARATOR,QString::number((int)separator));
    properties.insert(XMLGROUP,QString::number((int)group));
    properties.insert(XMLEVERYTHING,QString::number((int)allGroup));

    return properties;
}

bool Split::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLSEPARATOR).toInt(&ok);
    if (!ok || val < 0x00 || val > 0xFF) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLSEPARATOR),id);
    } else {
        setSeparator((char)val);
    }

    val = propertiesList.value(XMLGROUP).toInt(&ok);
    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLGROUP),id);
    } else {
        res = setSelectedGroup(val) && res;
    }

    val = propertiesList.value(XMLEVERYTHING).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLEVERYTHING),id);
    } else {
        setTakeAllGroup(val == 1);
    }

    return res;
}

QWidget *Split::requestGui(QWidget *parent)
{
    return new SplitWidget(this, parent);
}

QString Split::help() const
{
    QString help;
    help.append("<p>Split the input (UTF-8)</p><p>Convience function, nothing fancy.</p>");
    return help;
}

char Split::getSeparator()
{
    return separator;
}

int Split::getSelectedGroup()
{
    return group;
}

bool Split::doWeTakeAllGroup()
{
    return allGroup;
}

void Split::setSeparator(char val)
{
    separator = val;
    emit confUpdated();
}

bool Split::setSelectedGroup(int val)
{
    if (val < 0 || val > MAXGROUPVALUE) {
        emit error(tr("Invalid selected group value %1").arg(val),id);
        return false;
    }

    group = val;
    allGroup = false;
    emit confUpdated();
    return true;
}

void Split::setTakeAllGroup(bool val)
{
    allGroup = val;
    emit confUpdated();
}


