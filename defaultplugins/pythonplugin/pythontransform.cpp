/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "pythontransform.h"
#include <QHashIterator>
#include <QDebug>

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
    if (!moduleName.isEmpty())
        loadModule();
}

PythonTransform::~PythonTransform()
{
    qDebug() << "Destroying " << this;
    Py_XDECREF(pModule);
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

    if (loadModule()) {
        PyObject * pyInbound = Py_False; // never to be deleted
        if (twoWays)
            pyInbound = (wayValue == INBOUND ? Py_True : Py_False );

        if (PyModule_AddObject(pModule, INBOUND_ATTR_NAME, pyInbound) == -1) {
            Q_EMIT pythonError();
            logError(tr("Could not set the direction value properly"),id);
        }

        if (!parameters.isEmpty()) {
            PyObject *paramsdict = PyDict_New();
            if (!checkPyObject( paramsdict)) {
                logError("Error while creating the Python parameter dict", id);
                return;
            }
            QHashIterator<QByteArray, QByteArray> i(parameters);
            while (i.hasNext()) {
                i.next();
                PyObject* paramKey = PyUnicode_FromStringAndSize(i.key(),i.key().size());
                if (!checkPyObject( paramsdict)) {
                    logError("Error while creating Python parameter key", id);
                    Py_XDECREF(paramsdict);
                    return;
                }

                PyObject* paramValue = PyUnicode_FromStringAndSize(i.value(),i.value().size());
                if (!checkPyObject( paramValue)) {
                    logError("Error while creating Python parameter value", id);
                    Py_XDECREF(paramsdict);
                    Py_XDECREF(paramKey);
                    return;
                }

                if (PyDict_SetItem(paramsdict,paramKey,paramValue) == -1) {
                    Q_EMIT pythonError();
                    logError("Error while setting Python parameter pair", id);
                    Py_XDECREF(paramsdict);
                    Py_XDECREF(paramKey);
                    Py_XDECREF(paramValue);
                    return;
                }

                // Cleaning the values (references not stolen)
                Py_XDECREF(paramKey);
                Py_XDECREF(paramValue);
            }

            if (PyModule_AddObject(pModule,PARAMS_ATTR_NAME , paramsdict) == -1) { // stolen paramsdict reference
                Q_EMIT pythonError();
                logError(tr("Could not set the Pip3line_params value properly"),id);
            }


        } else {
            if (PyModule_AddObject(pModule,PARAMS_ATTR_NAME , Py_None) == -1) {
                Q_EMIT pythonError();
                logError(tr("Could not set the Pip3line_params None value properly"),id);
            }
        }

        PyObject * pFunc = PyObject_GetAttrString(pModule, MAIN_FUNCTION_NAME);

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

            if (PyTuple_SetItem(pArgs, 0, inputPy) != 0) {// stealing the reference of inputPy
                Q_EMIT error("Error while creating the Python byte array", id);
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
                    if (templength > BLOCK_MAX_SIZE) {
                        templength = BLOCK_MAX_SIZE;
                        Q_EMIT warning("Data block returned is too large, truncating.",id);
                    }
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
    qDebug()  << "Setting module filename";
    if (fileName == moduleFileName) {
        qDebug() << "nothing to be done here, filename already set";
        return true; // nothing to be done here
    }

    QString val = moduleManagement->addModule(fileName, type);
    if (!val.isEmpty()) {
        Py_XDECREF(pModule);
        pModule = NULL;
        moduleName = val;
        moduleFileName = fileName;
        if  (!loadModule()) {
           Q_EMIT error(tr("Error while loading module"),id);
        }
        qDebug() << "Module appeared to have been laoded successfully";
        //Q_EMIT confUpdated();
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

bool PythonTransform::loadModule()
{
    bool oldtwoWays = twoWays;

    qDebug() << "loading module";
    if (pModule == NULL) {
        if (moduleName.isEmpty()) {
            Q_EMIT error(tr("Empty module name, nothing to load..."),id);
            return false;
        }
        qDebug() << "loading module for the first time " << moduleName;
        PyObject *pName = PyUnicode_FromString(moduleName.toUtf8().data());
        pModule = PyImport_Import(pName);
        Py_XDECREF(pName);
        if (!checkPyObject(pModule)) {
            qDebug() << "First time load failed";
            Q_EMIT error(tr("Module \"%1\" could not be loaded. check stderr for more details").arg(moduleName),id);
            pModule = NULL;
            return false;
        }
    } else {
        PyObject *oldModule = pModule;
        pModule = PyImport_ReloadModule(oldModule);
        Py_XDECREF(oldModule); // cleaning the old module because the reference is different now
    }
    twoWays = false; // setting default
    if (checkPyObject(pModule)) {

        // checking if the two ways attribute is there
        PyObject * twoWayAttr = PyUnicode_FromString(ISTWOWAY_ATTR_NAME); // New ref
        if (checkPyObject(twoWayAttr)) {
            PyObject * pyTwoWay = NULL;
            int ret = PyObject_HasAttr(pModule,twoWayAttr);
            if (ret == 1) {
                pyTwoWay = PyObject_GetAttr(pModule,twoWayAttr); // New ref
                twoWays = checkPyObject(pyTwoWay) && pyTwoWay == Py_True;
            } else {
                qDebug() << id << " does not have attribute";
            }

            Py_XDECREF(pyTwoWay);
            Py_XDECREF(twoWayAttr);

        } else {
            PyErr_Clear();
        }

        bool parametersChanged = false;
        // checking if some default parameters names were defined
        PyObject * paramsNamesAttr = PyUnicode_FromString(PARAMS_NAMES_ATTR_NAME); // New ref
        if (checkPyObject(paramsNamesAttr)) {
            PyObject * pyNamesList = NULL;
            int ret = PyObject_HasAttr(pModule,paramsNamesAttr);
            if (ret == 1) {
                pyNamesList = PyObject_GetAttr(pModule,paramsNamesAttr); // New ref
                if (checkPyObject(pyNamesList) && PyList_Check(pyNamesList)) {
                    Py_ssize_t listSize = PyList_Size(pyNamesList);
                    if (listSize > 0) {
                        for (int i = 0; i < listSize; i++) {
                            PyObject *pyName = PyList_GetItem(pyNamesList, i);
                            if (checkPyObject(pyName) &&
 #ifdef BUILD_PYTHON_3
                                    PyUnicode_Check(pyName)) {

                                PyObject * nameutf8 = PyUnicode_AsUTF8String(pyName);
                                if (nameutf8 == NULL) {
                                    logError("Error while encoding a parameter to UTF-8",id);
                                    continue;
                                }
                                QByteArray val(PyBytes_AsString(nameutf8), PyBytes_Size(nameutf8));
#else
                                    PyString_Check(pyName)) {
                                QByteArray val(PyString_AsString(pyName), PyString_Size(pyName));
#endif
                                if (!parameters.contains(val)) {
                                    parameters.insert(val, QByteArray());
                                    parametersChanged = true;
                                }
                            }
                        }
                    }
                }
            } else {
                qDebug() << id << " does not have attribute";
            }

            Py_XDECREF(pyNamesList);
            Py_XDECREF(paramsNamesAttr);


        } else {
            PyErr_Clear();
        }

        if (oldtwoWays != twoWays || parametersChanged)
            Q_EMIT confUpdated();
    }
    else {
        pModule = NULL;
        return false;
    }
    return true;
}

bool PythonTransform::checkPyObject(PyObject *obj)
{
    if (PyErr_Occurred()) {
        Q_EMIT pythonError();
        return false;
    }
    return obj != NULL;
}
