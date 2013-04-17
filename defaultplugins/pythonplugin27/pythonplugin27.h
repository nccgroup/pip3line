/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PYTHONPLUGIN27_H
#define PYTHONPLUGIN27_H

#include "pythonplugin27_global.h"

#ifdef Q_OS_LINUX
#include <python2.7/Python.h>
#elif defined Q_OS_WIN
#include <Python.h>
#endif

#include <transformfactoryplugininterface.h>
#include <QList>
#include <pip3linecallback.h>
#include <modulesmanagement.h>

class PYTHONPLUGIN27SHARED_EXPORT PythonPlugin27 : public QObject, public TransformFactoryPluginInterface
{
        Q_OBJECT
#if QT_VERSION >= 0x050000
        Q_PLUGIN_METADATA(IID pluginIID FILE "python27plugin.json")
#endif
        Q_INTERFACES(TransformFactoryPluginInterface)
    public:

        PythonPlugin27();
        ~PythonPlugin27();
        void setCallBack(Pip3lineCallback * callback);
        QString pluginName() const;
        QString compiledWithQTversion() const;
        TransformAbstract * getTransform(QString name);
        const QStringList getTransformList(QString typeName);
        QWidget * getConfGui(QWidget * parent);
        const QStringList getTypesList();
        int getLibTransformVersion() const;
        QString pluginVersion() const;
    private slots:
        void updatePath();
        void retrievePythonErrors();
    private:
        void cleaningPyObjs();
        QStringList getCurrentSysPath();
        void settingUpStderr();
        bool checkPyObject(PyObject * obj);
        static const QString BASE_SCRIPTS_DIR;
        static const QString PYTHON27_TYPE;
        static const QString PYTHON_EXTENSION;
        static char PROG_NAME[];

        QWidget * gui;
        Pip3lineCallback *callback;
        PyObject *pyGetValFunc;
        PyObject *pyTruncateFunc;
        PyObject *pyStringIO;
        ModulesManagement * modules;
        QStringList defaultsysPath;
};

#endif // PYTHONPLUGIN27_H
