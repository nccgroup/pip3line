/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "modulesmanagement.h"
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QHashIterator>

const QString ModulesManagement::SETTINGS_USER_MODULES_LIST = "UserModulesList";

ModulesManagement::ModulesManagement(const QString &nlangName, const QString &extension, const QString & baseDir, const QStringList &initialPaths, Pip3lineCallback *ncallback)
{
    gui = 0;
    langName = nlangName;
    callback = ncallback;
    baseModulesDirName = baseDir;
    moduleExtension = extension;

    QStringList dirList = callback->pip3lineDirs();
    for (int i = 0; i < dirList.size(); i++) {
        QDir dir(dirList.at(i));
        if (dir.cd(baseDir)) {
            qDebug() << "Found " << nlangName << " plugin directory: " << dir.absolutePath();
            modulesPaths.insert(dir.absolutePath(), -1);
        }
    }
    // load modules from default locations
    QHashIterator<QString, int> j(modulesPaths);

    while (j.hasNext()) {
        j.next();
        QDir mDir(j.key());
        QStringList filters;
        filters << QString(moduleExtension).prepend("*");
        mDir.setNameFilters(filters);
        foreach (QString fileName, mDir.entryList(QDir::Files)) {
            callback->logStatus(tr("Loading module file %1 from %2").arg(fileName).arg(mDir.absolutePath()));
            addModule(mDir.absolutePath().append(QDir::separator()).append(fileName), AUTO);
        }
    }

    // adding default SysPath
    for (int i = 0; i < initialPaths.size(); i++)
        modulesPaths.insert(initialPaths.at(i),-1);

    // retrieving modules from persistent storage
    QStringList list = callback->getParameter(SETTINGS_USER_MODULES_LIST).toStringList();
    for (int i = 0; i < list.size(); i++) {
        addModule(list.at(i),PERSISTENT);
    }
}

ModulesManagement::~ModulesManagement()
{
    if (gui != 0)
        gui->deleteLater();
}

QStringList ModulesManagement::getPathsList()
{
    return modulesPaths.keys();
}

QStringList ModulesManagement::getModulesList()
{
    return modulesList.keys();
}

QStringList ModulesManagement::getRegisteredModule()
{
    QStringList list;
    QHashIterator<QString, ModuleProperties> i(modulesList);
     while (i.hasNext()) {
         i.next();
         if (i.value().type != TRANSIENT)
            list << i.key();
     }
     return list;
}

bool ModulesManagement::setType(const QString &name, ModulesManagement::ModuleType type)
{

    if (modulesList.contains(name)) {
        if (type == AUTO) {
            callback->logError(tr("setType - Cannot change the type to AUTO for %1 -_-").arg(name));
            return false;
        }
        ModuleType previous = modulesList.value(name).type;
        if (previous == type) // if there is no change, just return
            return true;
        else if (previous == AUTO) {
            callback->logError(tr("setType - Cannot change the type of %1. Module loaded from default location -_-").arg(name));
            return false;
        }
        ModuleProperties prop =  modulesList.value(name);
        prop.type = type;
        modulesList.insert(name, prop);
        savePersistentModules();
    } else {
        callback->logError(tr("setType - No module named \"%1\"").arg(name));
        return false;
    }

    return true;
}

void ModulesManagement::removeModule(const QString &name)
{
    if (modulesList.contains(name)) {
        ModuleType moduleType = modulesList.value(name).type;
        if (moduleType == AUTO) {
            callback->logError(tr("removeModule - Cannot remove module \"%1\", this module was loaded from default a location.").arg(name));
            return;
        }
        modulesList.remove(name);
        if (moduleType == PERSISTENT) {
            savePersistentModules();
            callback->notifyNewTransform();
        }
        emit modulesUpdated();
    } else {
        callback->logWarning(tr("removeModule - No module \"%1\"").arg(name));
    }
}

ModulesManagement::ModuleType ModulesManagement::getModuleType(const QString &name)
{
    if (modulesList.contains(name)) {
        return modulesList.value(name).type;
    } else {
        callback->logError(tr("getModuleType - No module \"%1\"").arg(name));
    }
    return TRANSIENT;
}

QString ModulesManagement::getModuleFileName(const QString &name)
{
    QString fileName;
    if (modulesList.contains(name)) {
        fileName = modulesList.value(name).fileName;
    } else {
        callback->logError(tr("getModuleFileName - No module \"%1\"").arg(name));
    }
    return fileName;
}

bool ModulesManagement::modulesContains(const QString &name)
{
    if (modulesList.contains(name)) {
        return modulesList.value(name).type != TRANSIENT;
    } else {
        return false;
    }
    return modulesList.contains(name);
}

bool ModulesManagement::isRegistered(const QString &name)
{
    if (modulesList.contains(name)) {
        return modulesList.value(name).type != TRANSIENT;
    }
    return false;
}

QString ModulesManagement::getExtension() const
{
    return moduleExtension;
}

QString ModulesManagement::getLangName() const
{
    return langName;
}

QWidget *ModulesManagement::getGui(QWidget *parent)
{
    if (gui == 0)
        gui = new ModulesManagementWidget(this, parent);
    return gui;
}

QString ModulesManagement::addModule(QString fileName, ModulesManagement::ModuleType type)
{
    QString moduleName;
    QFileInfo file(fileName);
    if (file.exists()) {
        moduleName = file.fileName();
        moduleName.chop(moduleExtension.size());
        if (modulesList.contains(moduleName)) {
            callback->logWarning(tr("A module named \"%1\" was already loaded, skipping.").arg(moduleName));
        } else {
            QString path = file.absolutePath();
            if (!modulesPaths.contains(path)) {
                modulesPaths.insert(path, 1);
                emit pathsUpdated();
            } else if (modulesPaths.value(path) > 0){
                modulesPaths.insert(path, modulesPaths.value(path) + 1);
            }
            ModuleProperties prop;
            prop.fileName = fileName;
            prop.type = type;
            modulesList.insert(moduleName,prop);
            if (type == PERSISTENT)
                savePersistentModules();
            emit modulesUpdated();
            callback->notifyNewTransform();
        }
    } else {
        callback->logError(tr("File %1 does not exist").arg(fileName));
    }
    return moduleName;
}

void ModulesManagement::savePersistentModules()
{
    QHashIterator<QString, ModuleProperties> i(modulesList);
    QStringList list;
    while (i.hasNext()) {
        i.next();
        if (i.value().type == PERSISTENT) {
            list << i.value().fileName;
        }
    }

    callback->saveParameter(SETTINGS_USER_MODULES_LIST,list );
}


void ModulesManagement::onGuiDelete()
{
    gui = 0;
}
