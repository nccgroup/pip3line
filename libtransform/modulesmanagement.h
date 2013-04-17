/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MODULESMANAGEMENT_H
#define MODULESMANAGEMENT_H

#include <QList>
#include <QStringList>
#include <QObject>
#include <QHash>
#include <QWidget>
#include "pip3linecallback.h"
#include "libtransform_global.h"
class ModulesManagementWidget;
#include "modulesmanagementwidget.h"

class LIBTRANSFORMSHARED_EXPORT ModulesManagement : public QObject
{
        Q_OBJECT
    public:
        enum ModuleType {TRANSIENT = 0, AUTO = 1, PERSISTENT};
        struct ModuleProperties {
                QString fileName;
                ModuleType type;
        };
        ModulesManagement(const QString &langName, const QString &extension, const QString & baseDir, const QStringList &initialPaths, Pip3lineCallback *callback);
        ~ModulesManagement();
        QStringList getPathsList();
        QStringList getModulesList();
        QStringList getRegisteredModule();

        QString addModule(QString fileName, ModulesManagement::ModuleType type = TRANSIENT);

        bool setType(const QString &name, ModulesManagement::ModuleType type);
        void removeModule(const QString &name);
        void removePersistentFile(const QString &fileName);

        ModuleType getModuleType(const QString &name);
        QString getModuleFileName(const QString &name);

        bool modulesContains(const QString &name);
        bool isRegistered(const QString &name);

        QString getExtension() const;
        QString getLangName() const;

        QWidget *getGui(QWidget * parent);

    Q_SIGNALS:
        void pathsUpdated();
        void modulesUpdated();
    private Q_SLOTS:
        void onGuiDelete();
    private:
        static const QString SETTINGS_USER_MODULES_LIST;

        void savePersistentModules();

        QHash<QString, int> modulesPaths;
        QHash<QString,ModuleProperties> modulesList;
        QString moduleExtension;
        Pip3lineCallback *callback;
        QString baseModulesDirName;
        ModulesManagementWidget *gui;
        QString langName;
};

#endif // MODULESMANAGEMENT_H
