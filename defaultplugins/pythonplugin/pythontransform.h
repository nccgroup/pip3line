/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PYTHONTRANSFORM_H
#define PYTHONTRANSFORM_H

#include <scripttransformabstract.h>

#ifdef Q_OS_LINUX
#ifdef BUILD_PYTHON_3
#include <python3.2/Python.h>
#else
#include <python2.7/Python.h>
#endif // BUILD_PYTHON_X
#elif defined Q_OS_WIN
#include <Python.h>
#endif

class PythonTransform : public ScriptTransformAbstract
{
        Q_OBJECT
    public:
        static const QString id;
        explicit PythonTransform(ModulesManagement * mmanagement, const QString &name = QString());
        ~PythonTransform();

        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QString help() const;
        bool setModuleFile(const QString &fileName);
        QString inboundString() const;
        QString outboundString() const;
    Q_SIGNALS:
        void pythonError();
    private:

        static const char * MAIN_FUNCTION_NAME;
        static const char * ISTWOWAY_ATTR_NAME;
        bool loadModule();
        bool checkPyObject(PyObject * obj);
        PyObject * pModule;
        bool twoWays;
};

#endif // PYTHONTRANSFORM_H
