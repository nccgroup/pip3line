/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "pythonplugin.h"
#include "pythontransform.h"
#include "../../version.h"
#include <QDebug>
#include <transformmgmt.h>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#ifdef BUILD_PYTHON_3
const QString PythonPlugin::BASE_SCRIPTS_DIR = "python3";
const QString PythonPlugin::PYTHON_TYPE = "Python 3";
wchar_t PythonPlugin::PROG_NAME[] = L"pip3line";
#else
const QString PythonPlugin::BASE_SCRIPTS_DIR = "python27";
const QString PythonPlugin::PYTHON_TYPE = "Python 2.7";
char PythonPlugin::PROG_NAME[] = APPNAME;
#endif
const QString PythonPlugin::PYTHON_EXTENSION = ".py";

PythonPlugin::PythonPlugin()
{
    callback = NULL;
    gui = NULL;
    pyGetValFunc = NULL;
    pyStringIO = NULL;
    pyTruncateFunc = NULL;
    pySeekFunc = NULL;
    modules = NULL;
    Py_SetProgramName(PROG_NAME);
    Py_Initialize();
    qDebug() << "Created " << this;
}

PythonPlugin::~PythonPlugin()
{
    qDebug() << "Destroying " << this;
    cleaningPyObjs();
    Py_Finalize();
    if (modules != NULL)
        delete modules;
}

void PythonPlugin::setCallBack(Pip3lineCallback *ncallback)
{
    callback = ncallback;

    settingUpStderr();
    modules = new(std::nothrow) ModulesManagement(PythonTransform::id, PYTHON_EXTENSION, BASE_SCRIPTS_DIR, getCurrentSysPath(), callback);
    if (modules == NULL) {
       qFatal("Cannot allocate memory for ModulesManagement (PythonPlugin) X{");
       return;
    }
    updatePath();
    connect(modules, SIGNAL(pathsUpdated()), this, SLOT(updatePath()));
}

QString PythonPlugin::pluginName() const
{

    return QString("%1 plugin").arg(PYTHON_TYPE);
}

QString PythonPlugin::compiledWithQTversion() const
{
    return QT_VERSION_STR;
}

TransformAbstract *PythonPlugin::getTransform(QString name)
{
    PythonTransform *ta = NULL;

    if (name == PythonTransform::id) {
        ta = new(std::nothrow) PythonTransform(modules);
        if (ta == NULL) {
           qFatal("Cannot allocate memory for Python (PythonPlugin 1) X{");
        }
    }
    else if (modules->isRegistered(name)) {
        ta = new(std::nothrow) PythonTransform(modules, name);
        if (ta == NULL) {
           qFatal("Cannot allocate memory for Python (PythonPlugin 2) X{");
        }
    }

    if (ta != NULL)
        connect(ta, SIGNAL(pythonError()), this, SLOT(retrievePythonErrors()));

    return ta;
}

const QStringList PythonPlugin::getTransformList(QString typeName)
{
    QStringList ret;
    if (typeName == PYTHON_TYPE) {
        ret.append(PythonTransform::id);
        ret.append(modules->getRegisteredModule());
    }
    return ret;
}

QWidget *PythonPlugin::getConfGui(QWidget *parent)
{
    if (gui == NULL) {
        gui = modules->getGui(parent);
    }
    return gui;
}

const QStringList PythonPlugin::getTypesList()
{
    return QStringList() << PYTHON_TYPE;
}

int PythonPlugin::getLibTransformVersion() const
{
    return LIB_TRANSFORM_VERSION;
}

QString PythonPlugin::pluginVersion() const
{
    return VERSION_STRING;
}

void PythonPlugin::updatePath()
{
    QStringList list = modules->getPathsList();
    QString pathString;
#if defined Q_OS_WIN
    char separator  =';';
#else
    char separator  = ':';
#endif


    for (int i = 0; i < list.size(); i++) {
        pathString.append(list.at(i)).append(separator);
    }

#ifdef BUILD_PYTHON_3
    // Qt garantee that QString.size() is always enough to convert to wchar_t
    int size = pathString.size();
    wchar_t *pathWString = new(std::nothrow) wchar_t[size + 1];
    if (!pathWString)
        qFatal("Cannot allocate memory for pathWString in python 3 plugin");

    int ret = pathString.toWCharArray(pathWString);
    pathWString[size] = L'\x00';
    if (ret != size) {
        callback->logError(tr("conversion to wchar returned %1. expecting %2").arg(ret).arg(size));
    }

    PySys_SetPath(pathWString);

    delete [] pathWString;
#else
    PySys_SetPath(pathString.toUtf8().data());
#endif
}

void PythonPlugin::retrievePythonErrors()
{
    qDebug() << "Processing " << PYTHON_TYPE << " error";
    PyErr_Print(); // Dump the error message(s) in the buffer
    if (pyGetValFunc == NULL || pyTruncateFunc == NULL || pySeekFunc ==NULL || pyStringIO == NULL) {
        callback->logError(tr("The error catching mecanism was not properly initialized, ignoring Python error request."));
        return;
    }

    // call getvalue() method in StringIO instance
    PyObject *obResult = NULL;
    QString final = "[Script error]";

    obResult = PyObject_CallObject(pyGetValFunc, NULL);
    if (!checkPyObject(obResult)){
        callback->logError("[stderr read] getvalue() failed");
        Py_XDECREF(obResult);
        return;
    }

    // did getvalue return a string?
#ifdef BUILD_PYTHON_3
    if (!PyUnicode_Check(obResult)){
#else
    if (!PyString_Check(obResult)){
#endif
        callback->logError("[stderr read] getvalue() did not return error string");
        Py_XDECREF(obResult);
        return;
    }

    // retrieve error message string from this object
#ifdef BUILD_PYTHON_3
    Py_ssize_t size = PyUnicode_GetSize(obResult);
#else
    Py_ssize_t size = PyString_Size(obResult);
#endif
    if (size < 1) {
        callback->logError(tr("[stderr read] invalid size returned %1").arg(size));
        Py_XDECREF(obResult);
        return;
    }

#ifdef BUILD_PYTHON_3
    // needs to be cleaned
    wchar_t *wstring = PyUnicode_AsWideCharString(obResult, &size);

    if (wstring != NULL) {
        final.append(QString::fromWCharArray(wstring, size));
        PyMem_Free(wstring);
#else
    char * buf = PyString_AsString(obResult); // don't touch that
    if (buf != NULL) {
        final.append(QByteArray(buf,size));
#endif
    } else {
       callback->logError("[stderr read] null string returned");
    }

    if (final.endsWith('\n')) {
        final.chop(1);
    }
    callback->logError(final);

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

void PythonPlugin::cleaningPyObjs()
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

QStringList PythonPlugin::getCurrentSysPath()
{
    QStringList list;
    Py_ssize_t listSize = 0;
    Py_ssize_t size = 0;
    PyObject* sysPath = PySys_GetObject((char*)"path"); // borrowed

    if (PyList_Check(sysPath) != 0) {
        listSize = PyList_Size(sysPath);
        for (Py_ssize_t i = 0; i < listSize; i++) {
            PyObject * pathObj = PyList_GetItem(sysPath,i); // borrowed
#ifdef BUILD_PYTHON_3
            if (PyUnicode_Check(pathObj)) {
                // this one needs to be cleaned
                wchar_t *wstring = PyUnicode_AsWideCharString(pathObj, &size);
                if (wstring != NULL) {
                    list.append(QString::fromWCharArray(wstring,size));
                    PyMem_Free(wstring);
#else
            if (PyString_Check(pathObj)) {
                char * buf; // don't touch that
                if (PyString_AsStringAndSize(pathObj, &buf, &size) != -1) {
                    list.append(QString::fromUtf8(QByteArray(buf, size)));
#endif
                } else {
                    callback->logError(tr("[getCurrentSysPath] Could not retrieve the current sys.path"));
                }

            } else {
                callback->logError(tr("[getCurrentSysPath] the object is not a string"));
            }

        }
    }

    return list;
}

void PythonPlugin::settingUpStderr()
{
    PyObject *modStringIO = NULL;
    PyObject *obFuncStringIO = NULL;
    char stderrString[] = "stderr";

    // Import cStringIO module
#ifdef BUILD_PYTHON_3
    modStringIO = PyImport_ImportModule("io");
#else
    modStringIO = PyImport_ImportModule("cStringIO");
#endif
    if (!checkPyObject(modStringIO)){
#ifdef BUILD_PYTHON_3
        callback->logError("[stderr init]Importing io failed");
#else
        callback->logError("[stderr init]Importing cStringIO failed");
#endif
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
        callback->logError("[stderr init] StringIO() failed");
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

bool PythonPlugin::checkPyObject(PyObject *obj)
{
    if (PyErr_Occurred()) {
        PyErr_Print();
        return false;
    }
    return obj != NULL;
}

QT_BEGIN_NAMESPACE
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(pythonplugin, PythonPlugin)
#endif
QT_END_NAMESPACE
