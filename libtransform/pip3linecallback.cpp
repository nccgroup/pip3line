/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "pip3linecallback.h"
#include "transformmgmt.h"
#include "transformabstract.h"
#include <QDebug>

Pip3lineCallback::Pip3lineCallback(TransformMgmt *tfactory, const QString &confFile, const QString &plugin, QObject *parent) :
    QObject(parent),
    settings(confFile, QSettings::IniFormat)
{
    transformFactory = tfactory;
    pluginName = plugin;
}

void Pip3lineCallback::saveParameter(const QString &name, const QVariant &value)
{
    if (name.isEmpty()) {
        logError(tr("Empty name passed to saveParameter, ignoring"));
        return;
    }
    qDebug() << "Saving setting " << name << " : " << value;
    settings.beginGroup(pluginName);
    settings.setValue(name,value);
    settings.endGroup();
}

QVariant Pip3lineCallback::getParameter(const QString &name)
{
    if (name.isEmpty()) {
        logError(tr("Empty name passed to getParameter, ignoring"));
        return QVariant();
    }
    settings.beginGroup(pluginName);
    QVariant ret = settings.value(name,QVariant());
    settings.endGroup();
    return ret;
}

QStringList Pip3lineCallback::pip3lineDirs()
{
    return transformFactory->getPluginsDirectory();
}

TransformAbstract *Pip3lineCallback::getTransformFromXML(QXmlStreamReader *xmlConf)
{
    return transformFactory->loadComposedTransformFromXML(xmlConf);
}

void Pip3lineCallback::logError(const QString &message)
{
    emit error(message, pluginName);
}

void Pip3lineCallback::logWarning(const QString &message)
{
    emit warning(message, pluginName);
}

void Pip3lineCallback::logStatus(const QString &message)
{
    emit status(message, pluginName);
}

void Pip3lineCallback::notifyNewTransform()
{
    emit newTransform();
}
