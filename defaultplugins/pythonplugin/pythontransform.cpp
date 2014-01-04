/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "pythontransform.h"
#include <QHashIterator>
#include <QDebug>
#include <QCoreApplication>
#include <QThread>

#ifdef BUILD_PYTHON_3
const QString PythonTransform::id = "Python 3 script";
#else
const QString PythonTransform::id = "Python 2.7 script";
#endif
const char * PythonTransform::MAIN_FUNCTION_NAME = "pip3line_transform";
const char * PythonTransform::ISTWOWAY_ATTR_NAME = "Pip3line_is_two_ways";
const char * PythonTransform::INBOUND_ATTR_NAME = "Pip3line_INBOUND";
const char * PythonTransform::PARAMS_ATTR_NAME = "Pip3line_params";
const char * PythonTransform::PARAMS_NAMES_ATTR_NAME = "Pip3line_params_names";

PythonTransform::PythonTransform(ModulesManagement * mmanagement, const QString &name) :
    ScriptTransformAbstract(mmanagement, name)
{
    pModule = NULL;
    twoWays = false;

    if (!moduleName.isEmpty()) {
        loadModule();
    }
}

PythonTransform::~PythonTransform()
{
    qDebug() << "Destroying " << this;
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();
    Py_XDECREF(pModule);
    PyGILState_Release(lgstate);
}

QString PythonTransform::description() const
{
    return id;
}

void PythonTransform::transform(const QByteArray &input, QByteArray &output)
{
    if (moduleName.isEmpty()) {
        Q_EMIT error(tr("No script configured"),id);
        return;
    }

    if (input.isEmpty())
        return;

    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();
    if (loadModule()) {
        PyObject * pyInbound = Py_False; // never to be deleted

        if (twoWays)
            pyInbound = (wayValue == INBOUND ? Py_True : Py_False );
        Py_INCREF(pyInbound);

        if (PyModule_AddObject(pModule, INBOUND_ATTR_NAME, pyInbound) == -1) { // steal references

            Q_EMIT pythonError();
            logError(tr("Could not set the direction value properly"),id);
            Py_XDECREF(pyInbound);
        }

        // setting parameters in the python environment
        if (!parameters.isEmpty()) {
            PyObject *paramsdict = PyDict_New();
            if (!checkPyError()) {
                logError("Error while creating the Python parameter dict", id);
                Py_XDECREF(paramsdict);
                PyGILState_Release(lgstate);
                return;
            }
            // adding parameters to the python list
            QHashIterator<QByteArray, QByteArray> i(parameters);
            while (i.hasNext()) {
                i.next();
                PyObject* paramKey = PyUnicode_FromStringAndSize(i.key(),i.key().size());
                if (!checkPyError()) {
                    logError("Error while creating Python parameter key", id);
                    Py_XDECREF(paramsdict);
                    PyGILState_Release(lgstate);
                    return;
                }

                PyObject* paramValue = PyUnicode_FromStringAndSize(i.value(),i.value().size());
                if (!checkPyError()) {
                    logError("Error while creating Python parameter value", id);
                    Py_XDECREF(paramsdict);
                    Py_XDECREF(paramKey);
                    PyGILState_Release(lgstate);
                    return;
                }

                if (PyDict_SetItem(paramsdict,paramKey,paramValue) == -1) { // stealing reference
                    checkPyError();
                    logError("Error while setting Python parameter pair", id);
                    Py_XDECREF(paramsdict);
                    Py_XDECREF(paramKey);
                    Py_XDECREF(paramValue);
                    PyGILState_Release(lgstate);
                    return;
                }

                // Cleaning the values (references not stolen)
                Py_XDECREF(paramKey);
                Py_XDECREF(paramValue);
            }

            if (PyModule_AddObject(pModule,PARAMS_ATTR_NAME , paramsdict) == -1) { // stolen paramsdict reference
                checkPyError();
                logError(tr("Could not set the Pip3line_params value properly"),id);
            }


        } else {
            Py_INCREF(Py_None);
            if (PyModule_AddObject(pModule,PARAMS_ATTR_NAME , Py_None) == -1) { // stealing reference
                checkPyError();
                logError(tr("Could not set the Pip3line_params None value properly"),id);
                Py_DECREF(Py_None);
            }
        }

        PyObject * pFunc = PyObject_GetAttrString(pModule, MAIN_FUNCTION_NAME);

        if (checkPyError() && PyCallable_Check(pFunc)) {
            PyObject* pArgs = PyTuple_New(1);

            if (!checkPyError()) {
                Q_EMIT error("Error while creating the Python argument tuple", id);
                Py_XDECREF(pFunc);
                Py_XDECREF(pArgs);
                PyGILState_Release(lgstate);
                return;
            }

            PyObject* inputPy = PyByteArray_FromStringAndSize(input.data(),input.size());
            if (!checkPyError()) {
                Q_EMIT error("Error while creating the Python byte array", id);
                Py_XDECREF(pFunc);
                Py_XDECREF(pArgs);
                Py_XDECREF(inputPy);
                PyGILState_Release(lgstate);
                return;
            }

            if (PyTuple_SetItem(pArgs, 0, inputPy) != 0) {// stealing the reference of inputPy
                Q_EMIT error("Error while creating the Python byte array", id);
                Py_XDECREF(inputPy);
                Py_XDECREF(pFunc);
                Py_XDECREF(pArgs);
                PyGILState_Release(lgstate);
                return;
            }
            PyObject* returnValue = PyObject_CallObject(pFunc, pArgs); // new ref or NULL

            if (!checkPyError()) {
                Q_EMIT error("Python error while executing the function (check logs for more information)", id);
            } else {
                if (PyByteArray_Check(returnValue)) {

                    Py_ssize_t templength = PyByteArray_Size(returnValue);
                    if (templength > BLOCK_MAX_SIZE) {
                        templength = BLOCK_MAX_SIZE;
                        Q_EMIT warning("Data block returned is too large, truncating.",id);
                    }

                    char * buffer = PyByteArray_AsString(returnValue); // never to be deleted
                    output.append(QByteArray(buffer,templength));
                } else {
                    Q_EMIT error("The Python object returned is not a bytearray", id);
                }
            }

            Py_XDECREF(returnValue);
            Py_XDECREF(pArgs);
          //  Py_DECREF(inputPy); // stolen reference, don't touch that
            Py_XDECREF(pFunc);

        } else {
            Q_EMIT error(tr("Python error while calling the function %1() (check stderr for more details)").arg(MAIN_FUNCTION_NAME), id);
        }
    }

    PyGILState_Release(lgstate);
}

bool PythonTransform::isTwoWays()
{
    return twoWays;
}

QString PythonTransform::help() const
{
    QString help;
    help.append(tr("<p>%1</p>").arg(id));
    help.append(tr("<p>The list of Python transforms is loaded automatically from differents default locations.</p><p>In addition modules are reloaded (refreshed) every time the input is refreshed, so you don't need to restart Pip3line to commit scripts changes</p><p>Using Python %1</p><p>Current Python module name: \"%2\"</p>").arg(QString(Py_GetVersion())).arg(moduleName));
    return help;
}

bool PythonTransform::setModuleFile(const QString &fileName)
{
    if (type == ModulesManagement::AUTO) {
        Q_EMIT error(tr("Not allowed to change the module for auto loaded modules"),id);
        return false;
    }
    if (fileName == moduleFileName) {
        qDebug() << "nothing to be done here, filename already set";
        return true; // nothing to be done here
    }

    QString val = moduleManagement->addModule(fileName, type);
    if (!val.isEmpty()) {
        PyGILState_STATE lgstate;
        lgstate = PyGILState_Ensure();
        Py_XDECREF(pModule);
        pModule = NULL;
        moduleName = val;
        moduleFileName = fileName;

        if  (!loadModule()) {
           Q_EMIT error(tr("Error while loading module"),id);
        }

        qDebug() << "Module appeared to have been loaded successfully";
        PyGILState_Release(lgstate);
        return true;
    }
    qDebug() << "Module appeared to have failed";
    return false;
}

QString PythonTransform::inboundString() const
{
    return "Inbound";
}

QString PythonTransform::outboundString() const
{
    return "Outbound";
}

bool PythonTransform::reloadModule()
{
    if (pModule == NULL) {
        Q_EMIT error(tr("Error while reloading %1, current module pointer is NULL").arg(moduleName),id);
        return false;
    }

    bool ret = true;
    bool oldtwoWays = twoWays;
    twoWays = false; // setting default
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

    PyObject *oldModule = pModule;
    pModule = PyImport_ReloadModule(oldModule); // new ref
    if (!checkPyError()) {
        Q_EMIT error(tr("Error while reloading the module %1").arg(moduleName),id);
        Py_XDECREF(pModule);
        pModule = NULL;
        ret = false;
    } else {
        // checking if the two ways attribute is there
        PyObject * twoWayAttr = PyUnicode_FromString(ISTWOWAY_ATTR_NAME); // New ref
        if (checkPyError()) {
            if (PyObject_HasAttr(pModule,twoWayAttr) == 1) {  // does the module has the attribute?
                PyObject * pyTwoWay = PyObject_GetAttr(pModule,twoWayAttr); // New ref
                if (checkPyError()) {
                    twoWays =  pyTwoWay == Py_True;
                } else {
                    logError("Error while looking for attribute value ISTWOWAY_ATTR_NAME",moduleName);
                }
                Py_XDECREF(pyTwoWay);
            } else {
                qDebug() << moduleName << " has no attribute" << ISTWOWAY_ATTR_NAME;
            }
        }
        Py_XDECREF(twoWayAttr);

        bool parametersChanged = false;
        // checking if some default parameters names were defined
        PyObject * paramsNamesAttr = PyUnicode_FromString(PARAMS_NAMES_ATTR_NAME); // New ref
        if (checkPyError()) {

            if (PyObject_HasAttr(pModule,paramsNamesAttr) == 1) { // does the module has the attribute?
                PyObject * pyNamesList = PyObject_GetAttr(pModule,paramsNamesAttr); // New ref
                if (checkPyError() && PyList_Check(pyNamesList)) {
                    Py_ssize_t listSize = PyList_Size(pyNamesList);
                    if (listSize > 0) { // if list size is null then nothing to do
                        for (int i = 0; i < listSize; i++) {
                            QByteArray val;
                            PyObject *pyName = PyList_GetItem(pyNamesList, i); // borrowed ref
                            if (checkPyError() &&  pyName != NULL && // error or invalid?
 #ifdef BUILD_PYTHON_3
                                    PyUnicode_Check(pyName)) { // is this a unicode string?

                                PyObject * nameutf8 = PyUnicode_AsUTF8String(pyName); // new ref
                                if (checkPyError() || nameutf8 == NULL) {
                                    logError("Error while encoding a parameter to UTF-8",id);
                                } else {
                                    val = QByteArray(PyBytes_AsString(nameutf8), PyBytes_Size(nameutf8));
                                }
                                Py_XDECREF(nameutf8);
#else
                                    PyString_Check(pyName)) { // is this a string?
                                val = QByteArray(PyString_AsString(pyName), PyString_Size(pyName));
#endif
                                if (val.isEmpty()) { // if the parameter name is empty, we skip
                                    qDebug() << moduleName << "Parameter name is empty";
                                } else if (!parameters.contains(val)) { // we don't want to erase any pre-existing configuration
                                    parameters.insert(val, QByteArray());
                                    parametersChanged = true;
                                }
                            }
                        }
                    }

                }
                Py_XDECREF(pyNamesList);
                pyNamesList = NULL;
            } else {
                qDebug() << moduleName << " does not have attribute" << PARAMS_NAMES_ATTR_NAME;
            }
        }
        Py_XDECREF(paramsNamesAttr);

        if (oldtwoWays != twoWays || parametersChanged) { // notifying only if something has changed
            Q_EMIT confUpdated();
        }
    }

    Py_XDECREF(oldModule); // cleaning the old module because the reference is different now

    PyGILState_Release(lgstate);
    return ret;
}

bool PythonTransform::loadModule()
{

    bool ret = true;

    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();
    if (pModule == NULL) {
        if (moduleName.isEmpty()) { // should have been checked earlier already, but who knows ...
            Q_EMIT error(tr("Empty module name, nothing to load..."),id);
            ret = false;
        } else {
            qDebug() << "Instanciating the module for the first time " << moduleName;
            PyObject *pName = PyUnicode_FromString(moduleName.toUtf8().data()); // new ref
            if (checkPyError() && pName != NULL) {
                pModule = PyImport_Import(pName);
                Py_XDECREF(pName);
                if (!checkPyError()) {
                    Q_EMIT error(tr("Module \"%1\" could not be loaded. check stderr for more details").arg(moduleName),id);
                    Py_XDECREF(pModule);
                    pModule = NULL;
                    ret = false;
                }
            } else {
                Q_EMIT error(tr("Could not create unicode string"),id);
                Py_XDECREF(pName);
                ret = false;
            }
        }
    }

    if (pModule != NULL && autoReload) { // if autoreload is enabled
        ret = reloadModule();
    } else {
        qDebug() << "no reloading";
    }

    PyGILState_Release(lgstate);
    return ret;
}

bool PythonTransform::checkPyError()
{
    if (PyErr_Occurred()) {
        Q_EMIT pythonError();
        return false;
    }
    return true;
}
