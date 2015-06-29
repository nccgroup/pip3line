/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PYTHONMODULES_H
#define PYTHONMODULES_H

#include "modulesmanagement.h"
#include <QHash>



class Pip3lineCallback;
typedef struct _ts PyThreadState;
typedef struct _object PyObject;

class PythonModules : public ModulesManagement
{
        Q_OBJECT
    public:
        static const QString PYTHON_TYPE;
        explicit PythonModules(QString name, Pip3lineCallback *callback);
        ~PythonModules();
        bool initialize();
        bool checkPyError();
        QString getLastError();
        PyObject * loadModule(QString modulePath, bool reload = false, bool *firstLoad = NULL);
        bool unloadModules(QString modulePath);
        void unloadModules();
        QString getInfos();
    private Q_SLOTS:
        void updatePath();
        QString retrievePythonErrors();
    private:
        static const QString BASE_SCRIPTS_DIR;

        static const QString PYTHON_EXTENSION;
#ifdef BUILD_PYTHON_3
        static wchar_t PROG_NAME[];
#else
        static char PROG_NAME[];
#endif
        PyObject *pyGetValFunc;
        PyObject *pyTruncateFunc;
        PyObject *pySeekFunc;
        PyObject *pyStringIO;
        QStringList defaultsysPath;
        PyThreadState *pymainstate;

        QHash<QString, PyObject*> modulesPath;

        void cleaningPyObjs();
        QString getRuntimeVersion();
        bool checkPyObject(PyObject *obj);
        void settingUpStderr();
        void disablingSIGINT();
        QString errorMessage;
};

#endif // PYTHONMODULES_H
