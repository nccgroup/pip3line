/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "python27.h"
#include <QDebug>

const QString Python27::id = "Python script 2.7";
const QString Python27::MAIN_FUNCTION_NAME = "pip3line_transform";
const QString Python27::ISTWOWAY_FUNCTION_NAME = "pip3line_is_two_ways";

Python27::Python27(ModulesManagement *mmanagement, const QString &name) :
    ScriptTransformAbstract(mmanagement, name)
{
    pModule = NULL;
    twoWays = false;
    loadModule();
}

Python27::~Python27()
{
    qDebug() << "Destroying " << this;
    Py_XDECREF(pModule);
}

QString Python27::description() const
{
    return tr("Python 2.7 script");
}

void Python27::transform(const QByteArray &input, QByteArray &output)
{
    if (moduleName.isEmpty()) {
        emit error(tr("No script configured"),id);
        return;
    }

    if (input.isEmpty())
        return;

    if (loadModule()) {
        PyObject * pyInbound = Py_False; // never to be deleted
        if (twoWays)
            pyInbound = (wayValue == INBOUND ? Py_True : Py_False );

        if (PyModule_AddObject(pModule, "Pip3line_INBOUND", pyInbound) == -1) {
            Q_EMIT pythonError();
            logError(tr("Could not set the direction value properly"),id);
        }

        PyObject * pFunc = PyObject_GetAttrString(pModule, MAIN_FUNCTION_NAME.toUtf8().data());

        if (checkPyObject(pFunc) && PyCallable_Check(pFunc)) {
            PyObject* pArgs = PyTuple_New(1);

            if (!checkPyObject( pArgs)) {
                emit error("Error while creating the Python argument tuple", id);
                Py_XDECREF(pFunc);
                return;
            }

            PyObject* inputPy = PyByteArray_FromStringAndSize(input.data(),input.size());
            if (!checkPyObject( inputPy)) {
                emit error("Error while creating the Python byte array", id);
                Py_XDECREF(pFunc);
                Py_XDECREF(pArgs);
                return;
            }

            if (PyTuple_SetItem(pArgs, 0, inputPy) != 0) {
                emit error("Error while creating the Python byte array", id);
                Py_XDECREF(pFunc);
                Py_XDECREF(pArgs);
                return;
            }

            PyObject* returnValue = PyObject_CallObject(pFunc, pArgs);

            if (!checkPyObject( returnValue)) {
                emit error("Python error while executing the function (check logs for more information)", id);
            } else {
                if (PyByteArray_Check(returnValue)) {
                    Py_ssize_t templength = PyByteArray_Size(returnValue);
                    char * buffer = PyByteArray_AsString(returnValue); // never to be deleted
                    output.append(QByteArray(buffer,templength));
                } else {
                    emit error("The Python object returned is not a bytearray", id);
                }
                Py_XDECREF(returnValue);
            }

            Py_XDECREF(pArgs);
          //  Py_DECREF(inputPy); // stolen reference, don't touch that
            Py_XDECREF(pFunc);

        } else {
            emit error(tr("Python error while calling the function %1() (check logs for more details)").arg(MAIN_FUNCTION_NAME), id);
        }

    } else {
        emit error(tr("Module \"%1\" could not be loaded. check logs for more details").arg(moduleName), id);
    }
}

bool Python27::isTwoWays()
{
    return twoWays;
}

QString Python27::inboundString() const
{
    return "Inbound";
}

QString Python27::outboundString() const
{
    return "Outbound";
}

QString Python27::help() const
{
    QString help;
    help.append(tr("<p>Python 2.7 script</p><p>The list of Python transforms is loaded automatically from differents default locations.</p><p>In addition modules are reloaded (refreshed) every time the input is refreshed, so you don't need to restart Pip3line to commit scripts changes</p><p>Using Python %1</p><p>Current Python 2.7 module name: \"%2\"</p>").arg(QString(Py_GetVersion())).arg(moduleName));
    return help;
}


bool Python27::setModuleFile(const QString &fileName)
{
    if (type == ModulesManagement::AUTO) {
        emit error(tr("Not allowed to change the module for auto loaded modules"),id);
        return false;
    }
    if (fileName == moduleFileName)
        return true; // nothing to be done here

    QString val = moduleManagement->addModule(fileName, type);
    if (!val.isEmpty()) {
        Py_XDECREF(pModule);
        pModule = NULL;
        moduleName = val;
        moduleFileName = fileName;
        loadModule();
        emit confUpdated();
        return true;
    }

    return false;
}

bool Python27::checkPyObject(PyObject *obj)
{
    if (PyErr_Occurred()) {
        emit pythonError();
        return false;
    }
    return obj != NULL;
}

bool Python27::loadModule()
{
    twoWays = false;
    if (!checkPyObject(pModule)) {
        PyObject *pName;
        pName = PyString_FromString(moduleName.toUtf8().data());
        pModule = PyImport_Import(pName);
        Py_XDECREF(pName);
        if (!checkPyObject(pModule)) {
            emit error(tr("Module \"%1\" could not be loaded. check stderr for more details").arg(moduleName),id);
            pModule = NULL;
            return false;
        }
    }

    PyObject *oldModule = pModule;
    pModule = PyImport_ReloadModule(oldModule);
    Py_XDECREF(oldModule); // cleaning the old module ref because the reference is different now

    if (checkPyObject(pModule)) {
        PyObject * pFunc = PyObject_GetAttrString(pModule, ISTWOWAY_FUNCTION_NAME.toUtf8().data());

        if (pFunc != NULL && PyCallable_Check(pFunc)) {
            PyObject* returnValue = PyObject_CallObject(pFunc, NULL);

            if (checkPyObject(returnValue) && PyBool_Check(returnValue)) {
                twoWays = returnValue == Py_True;
                qDebug() << "Got the direction " << twoWays;
            } else {
                qDebug() << "don't have the direction";
            }
            Py_XDECREF(returnValue);
            Py_XDECREF(pFunc);

        }else {
            PyErr_Clear();
            qDebug() << "Error while calling the Python function %1(), assuming one way transform";
        }
        qDebug() << "module loaded and configured";
        return true;
    } else {
        pModule = NULL;
        return false;
    }
}
