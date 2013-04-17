/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "pythonplugin27.h"
#include "python27.h"
#include "../../version.h"
#include <QDebug>
#include <transformmgmt.h>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

const QString PythonPlugin27::BASE_SCRIPTS_DIR = "python27";
const QString PythonPlugin27::PYTHON27_TYPE = "Python 2.7";
const QString PythonPlugin27::PYTHON_EXTENSION = ".py";
char PythonPlugin27::PROG_NAME[] = APPNAME;

PythonPlugin27::PythonPlugin27()
{
    callback = 0;
    gui = 0;
    pyGetValFunc = NULL;
    pyStringIO = NULL;
    pyTruncateFunc = NULL;
    modules = 0;
    Py_SetProgramName(PROG_NAME);
    Py_Initialize();
    qDebug() << "Created " << this;
}

PythonPlugin27::~PythonPlugin27()
{
    qDebug() << "Destroying " << this;
    cleaningPyObjs();
    Py_Finalize();
    if (modules != 0)
        delete modules;
}

void PythonPlugin27::setCallBack(Pip3lineCallback *ncallback)
{
    callback = ncallback;

    settingUpStderr();
    modules = new ModulesManagement(PYTHON27_TYPE, PYTHON_EXTENSION, BASE_SCRIPTS_DIR, getCurrentSysPath(), callback);
    updatePath();
    connect(modules, SIGNAL(pathsUpdated()), this, SLOT(updatePath()));
}

QString PythonPlugin27::pluginName() const
{
    return "Python 2.7 plugin";
}

QString PythonPlugin27::compiledWithQTversion() const
{
    return QT_VERSION_STR;
}

TransformAbstract *PythonPlugin27::getTransform(QString name)
{
    Python27 *ta = 0;

    if (name == Python27::id)
        ta = new Python27(modules);
    else if (modules->isRegistered(name)) {
        ta = new Python27(modules, name);
    }

    if (ta != 0)
        connect(ta, SIGNAL(pythonError()), this, SLOT(retrievePythonErrors()));

    return ta;
}

const QStringList PythonPlugin27::getTransformList(QString typeName)
{
    QStringList ret;
    if (typeName == PYTHON27_TYPE) {
        ret.append(Python27::id);
        ret.append(modules->getRegisteredModule());
    }
    return ret;
}

QWidget *PythonPlugin27::getConfGui(QWidget * parent)
{
    if (gui == 0) {
        gui = modules->getGui(parent);
    }
    return gui;
}

const QStringList PythonPlugin27::getTypesList()
{
    return QStringList() << PYTHON27_TYPE;
}

int PythonPlugin27::getLibTransformVersion() const
{
    return LIB_TRANSFORM_VERSION;
}

QString PythonPlugin27::pluginVersion() const
{
    return VERSION_STRING;
}

void PythonPlugin27::updatePath()
{
    QStringList list = modules->getPathsList();
    QByteArray final;
#if defined Q_OS_WIN
    char separator  =';';
#else
    char separator  = ':';
#endif
    for (int i = 0; i < list.size(); i++) {
        final.append(list.at(i).toUtf8()).append(separator);
    }
    PySys_SetPath(final.data());
}

QStringList PythonPlugin27::getCurrentSysPath()
{
    QStringList list;
    char * buf = 0;
    Py_ssize_t bufLen = 0;
    PyObject* sysPath = PySys_GetObject((char*)"path"); // borrowed

    if (PyList_Check(sysPath) != 0) {
        Py_ssize_t size = PyList_Size(sysPath);
        for (Py_ssize_t i = 0; i < size; i++) {
            PyObject * pathObj = PyList_GetItem(sysPath,i); // borrowed
            if (PyString_Check(pathObj)) {
                int ret = PyString_AsStringAndSize(pathObj, &buf, &bufLen); //do not touch the buffer
                if (ret != -1) {
                    QByteArray temp(buf, bufLen);
                    list.append(QString::fromUtf8(temp));
                }
            }
        }
    }

    return list;
}

void PythonPlugin27::settingUpStderr()
{
    PyObject *modStringIO = NULL;
    PyObject *obFuncStringIO = NULL;
    char stderrString[] = "stderr";

    // Import cStringIO module
    modStringIO = PyImport_ImportModule("cStringIO");
    if (!checkPyObject(modStringIO)){
        callback->logError("[stderr init]Importing cStringIO failed");
        Py_XDECREF(modStringIO);
        return;
    }
    // get StringIO constructor
    obFuncStringIO = PyObject_GetAttrString(modStringIO, "StringIO");
    if (!checkPyObject(obFuncStringIO)){
        callback->logError("[stderr init] can't find cStringIO.StringIO");
        Py_XDECREF(modStringIO);
        Py_XDECREF(obFuncStringIO);
        return;
    }
    // Construct cStringIO object
    pyStringIO = PyObject_CallObject(obFuncStringIO, NULL);
    if (!checkPyObject(pyStringIO)) {
        callback->logError("[stderr init] cStringIO.StringIO() failed");
        Py_XDECREF(modStringIO);
        Py_XDECREF(obFuncStringIO);
        cleaningPyObjs();
        return;
    }
    Py_XDECREF(modStringIO);
    Py_XDECREF(obFuncStringIO);

    // get getvalue() method in StringIO instance
    pyGetValFunc = PyObject_GetAttrString(pyStringIO, "getvalue");
    if (!checkPyObject(pyGetValFunc)){
        callback->logError("[stderr init] can't find getvalue() function");
        cleaningPyObjs();
        return;
    }

    // get truncate() method in StringIO instance
    pyTruncateFunc = PyObject_GetAttrString(pyStringIO, "truncate");
    if (!checkPyObject(pyTruncateFunc)){
        callback->logError("[stderr init] can't find truncate() function");
        cleaningPyObjs();
        return;
    }
    // try assigning this object to sys.stderr
    int ret = PySys_SetObject(stderrString, pyStringIO);
    if (ret != 0) {
        callback->logError("[stderr init] failed to assign StrinIO object to stderr");
        PyErr_Print();
        cleaningPyObjs();
    }
    return;
}

void PythonPlugin27::retrievePythonErrors()
{
    PyErr_Print(); // Dump the error message(s) in the buffer

    if (pyGetValFunc == NULL || pyTruncateFunc == NULL || pyStringIO == NULL) {
        callback->logError(tr("The error catching mecanism was not properly initialized, ignoring Python error request."));
        return;
    }

    // call getvalue() method in StringIO instance
    PyObject *obResult = NULL;
    QByteArray final;

    obResult = PyObject_CallObject(pyGetValFunc, NULL);
    if (!checkPyObject(obResult)){
        callback->logError("[stderr read] getvalue() failed");
        Py_XDECREF(obResult);
        return;
    }

    // did getvalue return a string?
    if (!PyString_Check(obResult)){
        callback->logError("[stderr read] getvalue() did not return error string");
        Py_XDECREF(obResult);
        return;
    }

    // retrieve error message string from this object

    final.append(PyString_AsString(obResult));
    if (final.endsWith('\n')) {
        final.chop(1);
    }

    callback->logError(QString::fromUtf8(final));
    Py_XDECREF(obResult);

    // Cleaning the StringIO object
    PyObject* pArgs = PyTuple_New(1);

    if (!checkPyObject( pArgs)) {
        callback->logError("[stderr cleaning] Error while creating the Python argument tuple");
        Py_XDECREF(pArgs);
        return;
    }

    PyObject* inputPy = PyInt_FromLong(0);
    if (!checkPyObject( inputPy)) {
        callback->logError("[stderr cleaning] Error while creating the Python int value (0)");
        Py_XDECREF(pArgs);
        Py_XDECREF(inputPy);
        return;
    }

    if (PyTuple_SetItem(pArgs, 0, inputPy) != 0) { // don't need to clean inputPy at this point (stolen)
        Py_XDECREF(pArgs);
        return;
    }

    obResult = PyObject_CallObject(pyTruncateFunc, pArgs);
    if (!checkPyObject(obResult)){
        callback->logError("[stderr cleaning] truncate() failed");
    }
    Py_XDECREF(obResult);
    Py_XDECREF(pArgs);
}

void PythonPlugin27::cleaningPyObjs()
{
    Py_XDECREF(pyTruncateFunc);
    pyTruncateFunc = NULL;
    Py_XDECREF(pyGetValFunc);
    pyGetValFunc = NULL;
    Py_XDECREF(pyStringIO);
    pyStringIO = NULL;
}

bool PythonPlugin27::checkPyObject(PyObject *obj)
{
    if (PyErr_Occurred()) {
        PyErr_Print();
        return false;
    }
    return obj != NULL;
}

QT_BEGIN_NAMESPACE
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(pythonplugin27, PythonPlugin27)
#endif
QT_END_NAMESPACE
