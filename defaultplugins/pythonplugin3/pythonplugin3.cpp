/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "pythonplugin3.h"
#include "python3.h"
#include "../../version.h"
#include <QDebug>
#include <transformmgmt.h>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

const QString Pythonplugin3::BASE_SCRIPTS_DIR = "python3";
const QString Pythonplugin3::PYTHON3_TYPE = "Python 3";
const QString Pythonplugin3::PYTHON_EXTENSION = ".py";
wchar_t Pythonplugin3::PROG_NAME[] = L"pip3line";

Pythonplugin3::Pythonplugin3()
{
    callback = 0;
    gui = 0;
    pyGetValFunc = NULL;
    pyStringIO = NULL;
    pyTruncateFunc = NULL;
    pySeekFunc = NULL;
    modules = 0;
    Py_SetProgramName(PROG_NAME);
    Py_Initialize();
    qDebug() << "Created " << this;
}

Pythonplugin3::~Pythonplugin3()
{
    qDebug() << "Destroying " << this;
    cleaningPyObjs();
    Py_Finalize();
    if (modules != 0)
        delete modules;
}

void Pythonplugin3::setCallBack(Pip3lineCallback *ncallback)
{
    callback = ncallback;

    settingUpStderr();
    modules = new ModulesManagement(PYTHON3_TYPE, PYTHON_EXTENSION, BASE_SCRIPTS_DIR, getCurrentSysPath(), callback);
    updatePath();
    connect(modules, SIGNAL(pathsUpdated()), this, SLOT(updatePath()));
}

QString Pythonplugin3::pluginName() const
{
    return "Python 3 plugin";
}

QString Pythonplugin3::compiledWithQTversion() const
{
    return QT_VERSION_STR;
}

TransformAbstract *Pythonplugin3::getTransform(QString name)
{
    Python3 *ta = 0;

    if (name == Python3::id)
        ta = new Python3(modules);
    else if (modules->isRegistered(name)) {
        ta = new Python3(modules, name);
    }

    if (ta != 0)
        connect(ta, SIGNAL(pythonError()), this, SLOT(retrievePythonErrors()));

    return ta;
}

const QStringList Pythonplugin3::getTransformList(QString typeName)
{
    QStringList ret;
    if (typeName == PYTHON3_TYPE) {
        ret.append(Python3::id);
        ret.append(modules->getRegisteredModule());
    }
    return ret;
}

QWidget *Pythonplugin3::getConfGui(QWidget *parent)
{
    if (gui == 0) {
        gui = modules->getGui(parent);
    }
    return gui;
}

const QStringList Pythonplugin3::getTypesList()
{
    return QStringList() << PYTHON3_TYPE;
}

int Pythonplugin3::getLibTransformVersion() const
{
    return LIB_TRANSFORM_VERSION;
}

QString Pythonplugin3::pluginVersion() const
{
    return VERSION_STRING;
}

void Pythonplugin3::updatePath()
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
    wchar_t *wstring = getWC(final.data(), final.size());
    PySys_SetPath(wstring);
    delete wstring;
}

void Pythonplugin3::retrievePythonErrors()
{
    PyErr_Print(); // Dump the error message(s) in the buffer
    if (pyGetValFunc == NULL || pyTruncateFunc == NULL || pySeekFunc ==NULL || pyStringIO == NULL) {
        callback->logError(tr("The error catching mecanism was not properly initialized, ignoring Python error request."));
        return;
    }

    // call getvalue() method in StringIO instance
    PyObject *obResult = NULL;
    QString final;

    obResult = PyObject_CallObject(pyGetValFunc, NULL);
    if (!checkPyObject(obResult)){
        callback->logError("[stderr read] getvalue() failed");
        Py_XDECREF(obResult);
        return;
    }

    // did getvalue return a string?
    if (!PyUnicode_Check(obResult)){
        callback->logError("[stderr read] getvalue() did not return error string");
        Py_XDECREF(obResult);
        return;
    }

    // retrieve error message string from this object

    Py_ssize_t size = 0;
    wchar_t *wstring = PyUnicode_AsWideCharString(obResult,&size);

    if (wstring != NULL) {
        final.append(QString::fromWCharArray(wstring, size));
        PyMem_Free(wstring);
        if (final.endsWith('\n')) {
            final.chop(1);
        }
        callback->logError(final);
    }

    Py_XDECREF(obResult);

    // Cleaning the StringIO object
    PyObject* pArgs = PyTuple_New(1);

    if (!checkPyObject( pArgs)) {
        callback->logError("[stderr cleaning] Error while creating the Python argument tuple");
        Py_XDECREF(pArgs);
        return;
    }

    PyObject* inputPy = PyLong_FromLong(0);
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

    // seek(0)

    pArgs = PyTuple_New(1);

    if (!checkPyObject( pArgs)) {
        callback->logError("[stderr cleaning] Error while creating the Python argument tuple");
        Py_XDECREF(pArgs);
        return;
    }

    inputPy = PyLong_FromLong(0);
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

    obResult = PyObject_CallObject(pySeekFunc, pArgs);
    if (!checkPyObject(obResult)){
        callback->logError("[stderr cleaning] seek() failed");
    }
    Py_XDECREF(obResult);
    Py_XDECREF(pArgs);
}

void Pythonplugin3::cleaningPyObjs()
{
    Py_XDECREF(pyTruncateFunc);
    pyTruncateFunc = NULL;
    Py_XDECREF(pyGetValFunc);
    pyGetValFunc = NULL;
    Py_XDECREF(pySeekFunc);
    pySeekFunc = NULL;
    Py_XDECREF(pyStringIO);
    pyStringIO = NULL;
}

QStringList Pythonplugin3::getCurrentSysPath()
{
    QStringList list;
    PyObject* sysPath = PySys_GetObject((char*)"path"); // borrowed

    if (PyList_Check(sysPath) != 0) {
        Py_ssize_t size = PyList_Size(sysPath);
        for (Py_ssize_t i = 0; i < size; i++) {
            PyObject * pathObj = PyList_GetItem(sysPath,i); // borrowed
            if (PyUnicode_Check(pathObj)) {
                Py_ssize_t size = 0;
                wchar_t *wstring = PyUnicode_AsWideCharString(pathObj,&size);
                if (wstring != NULL) {
                    list.append(QString::fromWCharArray(wstring));
                    PyMem_Free(wstring);
                }
            }
        }
    }

    return list;
}

void Pythonplugin3::settingUpStderr()
{
    PyObject *modStringIO = NULL;
    PyObject *obFuncStringIO = NULL;
    char stderrString[] = "stderr";

    // Import cStringIO module
    modStringIO = PyImport_ImportModule("io");
    if (!checkPyObject(modStringIO)){
        callback->logError("[stderr init]Importing io failed");
        Py_XDECREF(modStringIO);
        return;
    }
    // get StringIO constructor
    obFuncStringIO = PyObject_GetAttrString(modStringIO, "StringIO");
    if (!checkPyObject(obFuncStringIO)){
        callback->logError("[stderr init] can't find io.StringIO");
        Py_XDECREF(modStringIO);
        Py_XDECREF(obFuncStringIO);
        return;
    }
    // Construct cStringIO object
    pyStringIO = PyObject_CallObject(obFuncStringIO, NULL);
    if (!checkPyObject(pyStringIO)) {
        callback->logError("[stderr init] io.StringIO() failed");
        Py_XDECREF(modStringIO);
        Py_XDECREF(obFuncStringIO);
        cleaningPyObjs();
        return;
    }
    Py_XDECREF(obFuncStringIO);
    Py_XDECREF(modStringIO);

    // create getvalue() callable from StringIO
    pyGetValFunc = PyObject_GetAttrString(pyStringIO, "getvalue");
    if (!checkPyObject(pyGetValFunc)){
        callback->logError("[stderr init] can't find getvalue() function");
        cleaningPyObjs();
        return;
    }

    // create truncate() callable from StringIO
    pyTruncateFunc = PyObject_GetAttrString(pyStringIO, "truncate");
    if (!checkPyObject(pyTruncateFunc)){
        callback->logError("[stderr init] can't find truncate() function");
        cleaningPyObjs();
        return;
    }

    // create seek() callable from StringIO
    pySeekFunc = PyObject_GetAttrString(pyStringIO, "seek");
    if (!checkPyObject(pySeekFunc)){
        callback->logError("[stderr init] can't find seek() function");
        cleaningPyObjs();
        return;
    }

    // try assigning this object to sys.stderr
    int ret = PySys_SetObject(stderrString, pyStringIO);
    if (ret != 0) {
        callback->logError("[stderr init] failed to assign io.StringIO object to stderr");
        PyErr_Print();
        cleaningPyObjs();
    }
    return;
}

bool Pythonplugin3::checkPyObject(PyObject *obj)
{
    if (PyErr_Occurred()) {
        PyErr_Print();
        return false;
    }
    return obj != NULL;
}

wchar_t *Pythonplugin3::getWC(const char *c,  int size)
{
    const size_t cSize = size + 1; // +1 for the null byte
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs (wc, c, cSize);
    wc[size] = 0;

    return wc; // wc has to be freed by the caller
}

QT_BEGIN_NAMESPACE
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(pythonplugin3, Pythonplugin3)
#endif
QT_END_NAMESPACE
