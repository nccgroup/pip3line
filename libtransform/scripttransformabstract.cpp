/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "scripttransformabstract.h"
#include "moduletransformwidget.h"
#include <QHash>

const QString ScriptTransformAbstract::PROP_SCRIPT = "script";
const QString ScriptTransformAbstract::PROP_MODULE_NAME = "module";

ScriptTransformAbstract::ScriptTransformAbstract(ModulesManagement *mmanagement, const QString &name)
{
    moduleManagement = mmanagement;

    if (!name.isEmpty()) {
        moduleName = name;
        type = moduleManagement->getModuleType(name);
        moduleFileName = moduleManagement->getModuleFileName(name);
    }
}

ScriptTransformAbstract::~ScriptTransformAbstract()
{
}

QString ScriptTransformAbstract::name() const
{
    if (type != ModulesManagement::TRANSIENT)
        return moduleName;
    else
        return moduleManagement->getLangName();
}

QHash<QString, QString> ScriptTransformAbstract::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(PROP_MODULE_NAME, moduleName);
    properties.insert(PROP_SCRIPT, QString::fromUtf8(moduleFileName.toUtf8().toBase64()));

    return properties;
}

bool ScriptTransformAbstract::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);

    moduleName = propertiesList.value(PROP_MODULE_NAME);
    if (moduleName.isEmpty()) {
        emit error(tr("Module name is empty"),moduleManagement->getLangName());
        res = false;
    } else if (moduleManagement->modulesContains(moduleName)) {
        type = moduleManagement->getModuleType(moduleName);
        moduleFileName = moduleManagement->getModuleFileName(moduleName);
    } else {
        moduleFileName = QString::fromUtf8(QByteArray::fromBase64(propertiesList.value(PROP_SCRIPT).toUtf8()));
        if (moduleFileName.isEmpty()) {
            emit error(tr("Module file name is empty"),moduleManagement->getLangName());
            res = false;
        } else {
            moduleName = moduleManagement->addModule(moduleFileName);
            if (!moduleName.isEmpty()) {
                type = moduleManagement->getModuleType(moduleFileName);
            } else {
                res = false;
            }
        }
    }

    return res;
}

QWidget *ScriptTransformAbstract::requestGui(QWidget *parent)
{
    return new ModuleTransformWidget(this, parent);
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
    if (type != ntype) {
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

