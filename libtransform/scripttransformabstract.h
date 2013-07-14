/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SCRIPTTRANSFORMABSTRACT_H
#define SCRIPTTRANSFORMABSTRACT_H

#include "transformabstract.h"
#include <QString>
#include <QByteArray>
#include <QHash>
#include "modulesmanagement.h"
#include "libtransform_global.h"

class LIBTRANSFORMSHARED_EXPORT ScriptTransformAbstract : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit ScriptTransformAbstract(ModulesManagement * mmanagement, const QString &modulename = QString());
        virtual ~ScriptTransformAbstract();
        QString name() const;
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QString getModuleName() const;
        virtual bool setModuleFile(const QString &fileName) = 0;
        QString getModuleFileName() const;
        void setType(ModulesManagement::ModuleType type);
        ModulesManagement::ModuleType getType() const;
        QString getScriptDescr() const;
        QHash<QByteArray, QByteArray> getParameters() const;
        void setParameters(QHash<QByteArray, QByteArray> newParams);
    protected:
        Q_DISABLE_COPY(ScriptTransformAbstract)
        QWidget * requestGui(QWidget * parent);
        static const QString PROP_SCRIPT;
        static const QString PROP_MODULE_NAME;
        static const QString PROP_MODULE_PARAMS;
        QString moduleName;
        QString moduleFileName;
        ModulesManagement * moduleManagement;
        ModulesManagement::ModuleType type;
        QHash<QByteArray, QByteArray> parameters;
};

#endif // SCRIPTTRANSFORMABSTRACT_H
