/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "scripttransformabstract.h"
#include "moduletransformwidget.h"
#include <QHash>
#include <QHashIterator>
#include <QDebug>

const QString ScriptTransformAbstract::PROP_SCRIPT = "script";
const QString ScriptTransformAbstract::PROP_MODULE_NAME = "module";
const QString ScriptTransformAbstract::PROP_MODULE_PARAMS = "parameters";

ScriptTransformAbstract::ScriptTransformAbstract(ModulesManagement *mmanagement, const QString &modulename)
{
    type = ModulesManagement::TRANSIENT;
    moduleManagement = mmanagement;

    if (!modulename.isEmpty()) {
        moduleName = modulename;
        type = moduleManagement->getModuleType(modulename);
        moduleFileName = moduleManagement->getModuleFileName(modulename);
    }
}

ScriptTransformAbstract::~ScriptTransformAbstract()
{
}

QString ScriptTransformAbstract::name() const
{
    if (type != ModulesManagement::TRANSIENT && !moduleName.isEmpty())
        return moduleName;
    else
        return moduleManagement->getLangName();
}

QHash<QString, QString> ScriptTransformAbstract::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(PROP_MODULE_NAME, moduleName);

    QByteArray serialized;
    QHashIterator<QByteArray, QByteArray> i(parameters);
    while (i.hasNext()) {
        i.next();
        serialized.append(i.key().toBase64()).append(':').append(i.value().toBase64());
        if (i.hasNext())
            serialized.append('|');
    }
    properties.insert(PROP_MODULE_PARAMS, QString::fromUtf8(serialized));
    properties.insert(PROP_SCRIPT, QString::fromUtf8(moduleFileName.toUtf8().toBase64()));

    return properties;
}

bool ScriptTransformAbstract::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    parameters.clear();

    moduleName = propertiesList.value(PROP_MODULE_NAME);
    if (moduleName.isEmpty()) {
        emit error(tr("[ScriptTransformAbstract::setConfiguration]Module name is empty"),moduleManagement->getLangName());
        res = false;
    } else if (moduleManagement->modulesContains(moduleName)) {
        type = moduleManagement->getModuleType(moduleName);
        moduleFileName = moduleManagement->getModuleFileName(moduleName);
    } else {
        moduleFileName = QString::fromUtf8(QByteArray::fromBase64(propertiesList.value(PROP_SCRIPT).toUtf8()));
        if (moduleFileName.isEmpty()) {
            emit error(tr("[ScriptTransformAbstract::setConfiguration]Module file name is empty"),moduleManagement->getLangName());
            res = false;
        } else {
            moduleName = moduleManagement->addModule(moduleFileName);
            if (!moduleName.isEmpty()) {
                type = moduleManagement->getModuleType(moduleName);
            } else {
                emit error(tr("[ScriptTransformAbstract::setConfiguration]Module name is empty"),moduleManagement->getLangName());
                res = false;
            }
        }
    }

    if (!moduleName.isEmpty()) {
        QString parametersList = propertiesList.value(PROP_MODULE_PARAMS, QString());
        if (!parametersList.isEmpty()) {
            QStringList list = parametersList.split("|", QString::SkipEmptyParts);
            for (int i = 0 ; i < list.size(); i++) {
                QStringList param = list.at(i).split(":");
                if (param.size() != 2) {
                    emit error(tr("Invalid parameter: %1").arg(list.at(i)),moduleManagement->getLangName());
                } else {
                    parameters.insert(QByteArray::fromBase64(param.at(0).toUtf8()),QByteArray::fromBase64(param.at(1).toUtf8()));
                }
            }
        }
    }

    return res;
}

QWidget *ScriptTransformAbstract::requestGui(QWidget *parent)
{
    ModuleTransformWidget * widget = new(std::nothrow) ModuleTransformWidget(this, parent);
    if (widget == NULL) {
        qFatal("Cannot allocate memory for ModuleTransformWidget X{");
    }
    widget->reloadParameters();
    return widget;
}

QString ScriptTransformAbstract::getModuleName() const
{
    return moduleName;
}

QString ScriptTransformAbstract::getModuleFileName() const
{
    return moduleFileName;
}

void ScriptTransformAbstract::setType(ModulesManagement::ModuleType ntype)
{
    if (type == ModulesManagement::AUTO) {
        emit error(tr("Not allowed to change the type for auto loaded modules"),moduleManagement->getLangName());
        return;
    }
    qDebug() << "[ScriptTransformAbstract::setType]";
    if (type != ntype) {
        qDebug() << "[ScriptTransformAbstract::setType] type is different " << ntype;
        if (moduleManagement->setType(moduleName, ntype)) {
            type = moduleManagement->getModuleType(moduleName);
        }
    }
}

ModulesManagement::ModuleType ScriptTransformAbstract::getType() const
{
    return type;
}

QString ScriptTransformAbstract::getScriptDescr() const
{
    return moduleManagement->getLangName();
}

QHash<QByteArray, QByteArray> ScriptTransformAbstract::getParameters() const
{
    return parameters;
}

void ScriptTransformAbstract::setParameters(QHash<QByteArray, QByteArray> newParams)
{
    parameters = newParams;
    emit confUpdated();
}

