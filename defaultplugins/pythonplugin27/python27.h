/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/


#ifndef PYTHON27_H
#define PYTHON27_H

#include <scripttransformabstract.h>

#ifdef Q_OS_LINUX
#include <python2.7/Python.h>
#elif defined Q_OS_WIN
#include <Python.h>
#endif


class Python27 : public ScriptTransformAbstract
{
        Q_OBJECT
    public:
        static const QString id;
        explicit Python27(ModulesManagement * mmanagement, const QString &name = QString());
        ~Python27();
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QString help() const;
        QString inboundString() const;
        QString outboundString() const;
        bool setModuleFile(const QString &fileName);
    signals:
        void pythonError();
    private:
        static const QString MAIN_FUNCTION_NAME;
        static const QString ISTWOWAY_FUNCTION_NAME;
        bool loadModule();
        bool checkPyObject(PyObject * obj);
        PyObject * pModule;
        bool twoWays;
};

#endif // PYTHON27_H
