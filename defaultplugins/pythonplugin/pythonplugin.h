/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PYTHONPLUGIN_H
#define PYTHONPLUGIN_H

#include "pythonplugin_global.h"

#ifdef Q_OS_LINUX
#ifdef BUILD_PYTHON_3
#include <python3.2/Python.h>
#else
#include <python2.7/Python.h>
#endif // BUILD_PYTHON_X
#elif defined Q_OS_WIN
#include <Python.h>
#endif //Q_OS_?

#include <transformfactoryplugininterface.h>
#include <QList>
#include <pip3linecallback.h>
#include <modulesmanagement.h>

class PYTHONPLUGINSHARED_EXPORT PythonPlugin : public QObject, public TransformFactoryPluginInterface
{
        Q_OBJECT
#if QT_VERSION >= 0x050000
        Q_PLUGIN_METADATA(IID pluginIID FILE "pythonplugin.json")
#endif
        Q_INTERFACES(TransformFactoryPluginInterface)
    public:
        explicit PythonPlugin();
        ~PythonPlugin();
        void setCallBack(Pip3lineCallback * callback);
        QString pluginName() const;
        QString compiledWithQTversion() const;
        TransformAbstract * getTransform(QString name);
        const QStringList getTransformList(QString typeName);
        QWidget * getConfGui(QWidget * parent);
        const QStringList getTypesList();
        int getLibTransformVersion() const;
        QString pluginVersion() const;
    private Q_SLOTS:
        void updatePath();
        void retrievePythonErrors();
    private:
        void cleaningPyObjs();
        QStringList getCurrentSysPath();
        void settingUpStderr();
        bool checkPyObject(PyObject * obj);
        static const QString BASE_SCRIPTS_DIR;
        static const QString PYTHON_TYPE;
        static const QString PYTHON_EXTENSION;
#ifdef BUILD_PYTHON_3
        static wchar_t PROG_NAME[];
#else
        static char PROG_NAME[];
#endif

        QWidget * gui;
        Pip3lineCallback *callback;
        PyObject *pyGetValFunc;
        PyObject *pyTruncateFunc;
        PyObject *pySeekFunc;
        PyObject *pyStringIO;
        ModulesManagement * modules;
        QStringList defaultsysPath;

};

#endif // PYTHONPLUGIN_H
