/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PYTHONPLUGIN3_H
#define PYTHONPLUGIN3_H

#include "pythonplugin3_global.h"

#ifdef Q_OS_LINUX
#include <python3.2/Python.h>
#elif defined Q_OS_WIN
#include <Python.h>
#endif

#include <transformfactoryplugininterface.h>
#include <QList>
#include <pip3linecallback.h>
#include <modulesmanagement.h>

class PYTHONPLUGIN3SHARED_EXPORT Pythonplugin3 : public QObject, public TransformFactoryPluginInterface
{
        Q_OBJECT
#if QT_VERSION >= 0x050000
        Q_PLUGIN_METADATA(IID pluginIID FILE "pythonplugin3.json")
#endif
        Q_INTERFACES(TransformFactoryPluginInterface)
    public:
        Pythonplugin3();
        ~Pythonplugin3();
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
        wchar_t *getWC(const char *c, int size);
        QStringList getCurrentSysPath();
        void settingUpStderr();
        bool checkPyObject(PyObject * obj);
        static const QString BASE_SCRIPTS_DIR;
        static const QString PYTHON3_TYPE;
        static const QString PYTHON_EXTENSION;
        static wchar_t PROG_NAME[];

        QWidget * gui;
        Pip3lineCallback *callback;
        PyObject *pyGetValFunc;
        PyObject *pyTruncateFunc;
        PyObject *pySeekFunc;
        PyObject *pyStringIO;
        ModulesManagement * modules;
        QStringList defaultsysPath;

};

#endif // PYTHONPLUGIN3_H
