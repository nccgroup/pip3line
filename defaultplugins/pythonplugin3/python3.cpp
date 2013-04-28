/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "python3.h"
#include <QDebug>

const QString Python3::id = "Python script 3";

const QString Python3::MAIN_FUNCTION_NAME = "pip3line_transform";
const QString Python3::ISTWOWAY_FUNCTION_NAME = "pip3line_is_two_ways";

Python3::Python3(ModulesManagement * mmanagement, const QString &name) :
    ScriptTransformAbstract(mmanagement, name)
{
    pModule = NULL;
    twoWays = false;
    loadModule();
}

Python3::~Python3()
{
    qDebug() << "Destroying " << this;
    Py_XDECREF(pModule);
}

QString Python3::description() const
{
    return tr("Python 3 script");
}

void Python3::transform(const QByteArray &input, QByteArray &output)
{
    if (moduleName.isEmpty()) {
        Q_EMIT error(tr("No script configured"),id);
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
                Q_EMIT error("Error while creating the Python argument tuple", id);
                Py_XDECREF(pFunc);
                return;
            }

            PyObject* inputPy = PyByteArray_FromStringAndSize(input.data(),input.size());
            if (!checkPyObject( inputPy)) {
                Q_EMIT error("Error while creating the Python byte array", id);
                Py_XDECREF(pFunc);
                Py_XDECREF(pArgs);
                return;
            }

            if (PyTuple_SetItem(pArgs, 0, inputPy) != 0) {
                Py_XDECREF(pFunc);
                Py_XDECREF(pArgs);
                return;
            }

            PyObject* returnValue = PyObject_CallObject(pFunc, pArgs);

            if (!checkPyObject( returnValue)) {
                Q_EMIT error("Python error while executing the function (check logs for more information)", id);
            } else {
                if (PyByteArray_Check(returnValue)) {
                    Py_ssize_t templength = PyByteArray_Size(returnValue);
                    char * buffer = PyByteArray_AsString(returnValue); // never to be deleted
                    output.append(QByteArray(buffer,templength));
                } else {
                    Q_EMIT error("The Python object returned is not a bytearray", id);
                }
                Py_XDECREF(returnValue);
            }

            Py_XDECREF(pArgs);
          //  Py_DECREF(inputPy); // stolen reference, don't touch that
            Py_XDECREF(pFunc);

        } else {
            Q_EMIT error(tr("Python error while calling the function %1() (check stderr for more details)").arg(MAIN_FUNCTION_NAME), id);
        }
    }
}

bool Python3::isTwoWays()
{
    return twoWays;
}

QString Python3::help() const
{
    QString help;
    help.append(tr("<p>Python 3 script</p><p>The list of Python transforms is loaded automatically from differents default locations.</p><p>In addition modules are reloaded (refreshed) every time the input is refreshed, so you don't need to restart Pip3line to commit scripts changes</p><p>Using Python %1</p><p>Current Python 3 module name: \"%2\"</p>").arg(QString(Py_GetVersion())).arg(moduleName));
    return help;
}

bool Python3::setModuleFile(const QString &fileName)
{
    if (type == ModulesManagement::AUTO) {
        Q_EMIT error(tr("Not allowed to change the module for auto loaded modules"),id);
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
        Q_EMIT confUpdated();
        return true;
    }

    return false;
}

QString Python3::inboundString() const
{
    return "Inbound";
}

QString Python3::outboundString() const
{
    return "Outbound";
}

bool Python3::loadModule()
{
    bool oldtwoWays = twoWays;
    twoWays = false;
    if (!checkPyObject(pModule)) {
        PyObject *pName;
        pName = PyUnicode_FromString(moduleName.toUtf8().data());
        pModule = PyImport_Import(pName);
        Py_XDECREF(pName);
        if (!checkPyObject(pModule)) {
            Q_EMIT error(tr("Module \"%1\" could not be loaded. check stderr for more details").arg(moduleName),id);
            pModule = NULL;
            return false;
        }
    }

    PyObject *oldModule = pModule;
    pModule = PyImport_ReloadModule(oldModule);
    Py_XDECREF(oldModule); // cleaning the old module because the reference is different now
    if (checkPyObject(pModule)) {
        PyObject * pFunc = PyObject_GetAttrString(pModule, ISTWOWAY_FUNCTION_NAME.toUtf8().data());

        if (pFunc != NULL && PyCallable_Check(pFunc)) {
            PyObject* returnValue = PyObject_CallObject(pFunc, NULL);

            if (checkPyObject(returnValue) && PyBool_Check(returnValue)) {
                twoWays = returnValue == Py_True;
            }

            Py_XDECREF(returnValue);
            Py_XDECREF(pFunc);
        }else {
            PyErr_Clear();
        }

        if (oldtwoWays != twoWays)
            Q_EMIT confUpdated();

        return true;
    }
    else {
        pModule = NULL;
        return false;
    }
}

bool Python3::checkPyObject(PyObject *obj)
{
    if (PyErr_Occurred()) {
        Q_EMIT pythonError();
        return false;
    }
    return obj != NULL;
}
