/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PYTHON3_H
#define PYTHON3_H

#include <scripttransformabstract.h>

#ifdef Q_OS_LINUX
#include <python3.2/Python.h>
#elif defined Q_OS_WIN
#include <Python.h>
#endif

class Python3 : public ScriptTransformAbstract
{
        Q_OBJECT
    public:
        static const QString id;
        Python3(ModulesManagement * mmanagement, const QString &name = QString());
        ~Python3();

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
        bool loadModule();
        static const QString MAIN_FUNCTION_NAME;
        static const QString ISTWOWAY_FUNCTION_NAME;
        bool checkPyObject(PyObject * obj);
        PyObject * pModule;
        bool twoWays;
};

#endif // PYTHON3_H
