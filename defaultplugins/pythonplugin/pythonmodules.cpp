/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include <Python.h>

#include "pythonmodules.h"
#include "../../version.h"
#include <transformmgmt.h>
#include <QDebug>
#include <pip3linecallback.h>

#ifdef BUILD_PYTHON_3
const QString PythonModules::BASE_SCRIPTS_DIR = "python3";
const QString PythonModules::PYTHON_TYPE = "Python 3";
wchar_t PythonModules::PROG_NAME[] = L"pip3line";
#else
const QString PythonModules::BASE_SCRIPTS_DIR = "python27";
const QString PythonModules::PYTHON_TYPE = "Python 2.7";
char PythonModules::PROG_NAME[] = APPNAME;

#endif
const QString PythonModules::PYTHON_EXTENSION = ".py";

PythonModules::PythonModules(QString name, Pip3lineCallback *callback) :
    ModulesManagement(name, PYTHON_EXTENSION, BASE_SCRIPTS_DIR, callback)
{

    pyGetValFunc = NULL;
    pyStringIO = NULL;
    pyTruncateFunc = NULL;
    pySeekFunc = NULL;

    Py_SetProgramName(PROG_NAME);

    // initialize the Python interpreter without signal handler
    Py_InitializeEx(0);
    // initialize thread support
    PyEval_InitThreads();
    // saving thread state
    pymainstate = PyEval_SaveThread();

   connect(this, SIGNAL(pathsUpdated()), SLOT(updatePath()));
   qDebug() << "Created " << this;

}

PythonModules::~PythonModules()
{
    qDebug() << "Destroying " << this;

    unloadModules();

    PyEval_RestoreThread(pymainstate);


    cleaningPyObjs();

    Py_Finalize();
}

bool PythonModules::initialize()
{

    settingUpStderr();
    disablingSIGINT();

    // retrieving current sys.path from the Python interpreter
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

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
                wchar_t *wstring = PyUnicode_AsWideCharString(pathObj, &size); // new object, need to be cleaned
                if (wstring != NULL) {
                    list.append(QString::fromWCharArray(wstring,size));
                    PyMem_Free(wstring);
#else
            if (PyString_Check(pathObj)) {
                char * buf = NULL;
                if (PyString_AsStringAndSize(pathObj, &buf, &size) != -1) { // do not touch buf after the call
                    list.append(QString::fromUtf8(QByteArray(buf, size)));
#endif
                } else {
                    callback->logError(tr("[getCurrentSysPath] Could not convert the pathObj to string"));
                    return false;
                }

            } else {
                callback->logError(tr("[getCurrentSysPath] the object is not a string"));
                return false;
            }

        }
    } else {
        callback->logError(tr("[getCurrentSysPath] no sys.path property was found. something is really wrong T_T"));
        return false;
    }

    PyGILState_Release(lgstate);

    // initializing the parent with the list
    bool ret = ModulesManagement::initialize(list);

    if (ret) updatePath();

    return ret;
}

PyObject *PythonModules::loadModule(QString modulePath, bool reload, bool *firstLoad)
{
    PyObject *pModule = NULL;
    if (modulePath.isEmpty()) { // should have been checked earlier already, but who knows ...
        callback->logError(tr("Empty module path name, nothing to load..."));
        return pModule;
    }

    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

    if (!modulesPath.contains(modulePath)) {
        qDebug() << "Instanciating the module for the first time " << modulePath;
        QString moduleName = getModuleNameFromFile(modulePath);
            pModule = PyImport_ImportModule(moduleName.toUtf8().data());
            if (!checkPyError()) {
                callback->logError(tr("Module \"%1\" could not be loaded:\n %2").arg(modulePath).arg(errorMessage));
                pModule = NULL;
            } else {
                modulesPath.insert(modulePath,pModule);
                if (firstLoad != NULL)
                    *firstLoad = true;
            }

    } else if (reload) {
        qDebug() << "Reloading module" << modulePath;
        PyObject *oldModule = modulesPath.take(modulePath); // the module object is either going to be replaced or cleared
        pModule = PyImport_ReloadModule(oldModule); // new ref ??
        if (pModule != oldModule) {
            Py_XDECREF(oldModule); // clearing the old module object if the new ref is different
        }

        if (!checkPyError()) {
            callback->logError(tr("Error(s) while reloading the module %1, removing it from the the registered modules.\n%2").arg(modulePath).arg(errorMessage));
            pModule = NULL;
        } else {
            modulesPath.insert(modulePath,pModule);
        }

    } else {
        qDebug() << "no reload, taking the module as it is already" << modulePath;
        pModule = modulesPath.value(modulePath);
    }

    PyGILState_Release(lgstate);
    return pModule;
}

bool PythonModules::unloadModules(QString modulePath)
{
    bool ret = false;
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();
    if (modulesPath.contains(modulePath)) {
        Py_XDECREF(modulesPath.take(modulePath));
        qDebug() << "Module" << modulePath << "removed";
        ret = true;
    }

    PyGILState_Release(lgstate);

    return ret;
}

void PythonModules::unloadModules()
{
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();
    PyObject * module = NULL;
    Q_FOREACH (module, modulesPath) {
        Py_XDECREF(module);
    }
    modulesPath.clear();

    PyGILState_Release(lgstate);
}

void PythonModules::updatePath()
{
    QStringList list = modulesPaths.keys();
    QString pathString;
#if defined Q_OS_WIN
    char separator  =';';
#else
    char separator  = ':';
#endif


    for (int i = 0; i < list.size(); i++) {
        pathString.append(list.at(i)).append(separator);
    }

    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

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

    PyGILState_Release(lgstate);
}

void PythonModules::settingUpStderr()
{
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

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
        PyGILState_Release(lgstate);
        return;
    }
    // get StringIO constructor
    obFuncStringIO = PyObject_GetAttrString(modStringIO, "StringIO");
    if (!checkPyObject(obFuncStringIO)){
        callback->logError("[stderr init] can't find io.StringIO");
        Py_XDECREF(modStringIO);
        Py_XDECREF(obFuncStringIO);
        PyGILState_Release(lgstate);
        return;
    }
    // Construct cStringIO object
    pyStringIO = PyObject_CallObject(obFuncStringIO, NULL);
    if (!checkPyObject(pyStringIO)) {
        callback->logError("[stderr init] StringIO() failed");
        Py_XDECREF(modStringIO);
        Py_XDECREF(obFuncStringIO);
        cleaningPyObjs();
        PyGILState_Release(lgstate);
        return;
    }
    Py_XDECREF(obFuncStringIO);
    Py_XDECREF(modStringIO);

    // create getvalue() callable from StringIO
    pyGetValFunc = PyObject_GetAttrString(pyStringIO, "getvalue");
    if (!checkPyObject(pyGetValFunc)){
        callback->logError("[stderr init] can't find getvalue() function");
        cleaningPyObjs();
        PyGILState_Release(lgstate);
        return;
    }

    // create truncate() callable from StringIO
    pyTruncateFunc = PyObject_GetAttrString(pyStringIO, "truncate");
    if (!checkPyObject(pyTruncateFunc)){
        callback->logError("[stderr init] can't find truncate() function");
        cleaningPyObjs();
        PyGILState_Release(lgstate);
        return;
    }

    // create seek() callable from StringIO
    pySeekFunc = PyObject_GetAttrString(pyStringIO, "seek");
    if (!checkPyObject(pySeekFunc)){
        callback->logError("[stderr init] can't find seek() function");
        cleaningPyObjs();
        PyGILState_Release(lgstate);
        return;
    }

    // try assigning this object to sys.stderr
    int ret = PySys_SetObject(stderrString, pyStringIO);
    if (ret != 0) {
        callback->logError("[stderr init] failed to assign io.StringIO object to stderr");
        PyErr_Print();
        cleaningPyObjs();
    }

    PyGILState_Release(lgstate);
}

void PythonModules::disablingSIGINT()
{
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

    PyObject *modSignals = NULL;
    // Import signal module

    modSignals = PyImport_ImportModule("signal");

    if (!checkPyObject(modSignals)){
        callback->logError("[disablingSIGINT] Importing signal failed, Python SIGINT handler is NOT disabled");
        Py_XDECREF(modSignals);
        PyGILState_Release(lgstate);
        return;
    }

    // get signal.SIGINT
    PyObject *pySIGINT = PyObject_GetAttrString(modSignals, "SIGINT");
    if (!checkPyObject(pySIGINT)){
        callback->logError("[disablingSIGINT] Failed to retrieve signal.SIGINT");
        Py_XDECREF(modSignals);
        PyGILState_Release(lgstate);
        return;
    }

    // get signal.SIG_IGN (the handler that ignore a signal)
    PyObject *pySIG_IGN = PyObject_GetAttrString(modSignals, "SIG_IGN");
    if (!checkPyObject(pySIG_IGN)){
        callback->logError("[disablingSIGINT] Failed to retrieve signal.SIG_IGN");
        Py_XDECREF(modSignals);
        Py_XDECREF(pySIGINT);
        PyGILState_Release(lgstate);
        return;
    }

    // create signal() callable from signal
    PyObject * pySignalFunc = PyObject_GetAttrString(modSignals, "signal");
    if (!checkPyObject(pySignalFunc)){
        checkPyError();
        {
            callback->logError(tr("%1").arg(errorMessage));
        }
        callback->logError("[disablingSIGINT] Failed to retrieve signal.signal()");
        Py_XDECREF(modSignals);
        Py_XDECREF(pySIGINT);
        Py_XDECREF(pySIG_IGN);
        PyGILState_Release(lgstate);
        return;
    }

    PyObject * pArgs = PyTuple_New(2); // new ref for arguments
    if (!checkPyObject(pArgs)){
        callback->logError("[disablingSIGINT] Failed to create argument tuple");
        Py_XDECREF(modSignals);
        Py_XDECREF(pySIGINT);
        Py_XDECREF(pySIG_IGN);
        Py_XDECREF(pySignalFunc);
        PyGILState_Release(lgstate);
        return;
    }


    if (PyTuple_SetItem(pArgs, 0, pySIGINT) != 0) { // don't need to clean pySIGINT at this point (stolen)
        callback->logError("[disablingSIGINT] Error while assigning the SIGINT value to the arg tuple");
        Py_XDECREF(modSignals);
        Py_XDECREF(pySIGINT);
        Py_XDECREF(pySIG_IGN);
        Py_XDECREF(pySignalFunc);
        Py_XDECREF(pArgs);
        PyGILState_Release(lgstate);
        return;
    }

    if (PyTuple_SetItem(pArgs, 1, pySIG_IGN) != 0) { // don't need to clean pySIG_IGN at this point (stolen)
        callback->logError("[disablingSIGINT] Error while assigning the SIG_IGN value to the arg tuple");
        Py_XDECREF(modSignals);
        Py_XDECREF(pySIG_IGN);
        Py_XDECREF(pySignalFunc);
        Py_XDECREF(pArgs);
        PyGILState_Release(lgstate);
        return;
    }

    PyObject * obResult = PyObject_CallObject(pySignalFunc, pArgs); // new ref or NULL
    if (!checkPyError()){
        callback->logError("[disablingSIGINT] signal.signal(SIGINT, SIG_IGN) failed");
        callback->logError(tr("%1").arg(errorMessage));
    }

    qDebug() << "Python SIGINT handler disabled";
    Py_XDECREF(obResult);
    Py_XDECREF(pArgs);
    Py_XDECREF(modSignals);

    PyGILState_Release(lgstate);
}

bool PythonModules::checkPyError()
{
    if (PyErr_Occurred()) {
        errorMessage = retrievePythonErrors();
        return false;
    }
    errorMessage.clear();
    return true;
}

QString PythonModules::getLastError()
{
    return errorMessage;
}

QString PythonModules::retrievePythonErrors()
{
    QString message;
    PyObject* pArgs = NULL;
    PyObject* inputPy = NULL;
#ifdef BUILD_PYTHON_3
    wchar_t *wstring = NULL;
#else
    char * buf = NULL;
#endif
    Py_ssize_t size = 0;
    QString final = "[Script error]";
    PyObject *obResult = NULL;
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

    PyErr_Print(); // Dump the error message(s) in the buffer
    if (pyGetValFunc == NULL || pyTruncateFunc == NULL || pySeekFunc ==NULL || pyStringIO == NULL) {
        callback->logError(tr("The error catching mecanism was not properly initialized, ignoring Python error request."));
        goto leaving;
    }

    // call getvalue() method in StringIO instance



    obResult = PyObject_CallObject(pyGetValFunc, NULL); // new ref or NULL
    if (!checkPyObject(obResult)){
        callback->logError("[stderr read] getvalue() failed");
        Py_XDECREF(obResult);
        goto leaving;
    }

    // did getvalue return a string?
#ifdef BUILD_PYTHON_3
    if (!PyUnicode_Check(obResult)){
#else
    if (!PyString_Check(obResult)){
#endif
        callback->logError("[stderr read] getvalue() did not return error string");
        Py_XDECREF(obResult);
        goto leaving;
    }

    // retrieve error message string from this object
#ifdef BUILD_PYTHON_3
    size = PyUnicode_GetSize(obResult);
#else
    size = PyString_Size(obResult);
#endif
    if (size < 1) {
        callback->logError(tr("[stderr read] invalid size returned %1").arg(size));
        Py_XDECREF(obResult);
        goto leaving;
    }

#ifdef BUILD_PYTHON_3
    // needs to be cleaned
    wstring = PyUnicode_AsWideCharString(obResult, &size);

    if (wstring != NULL) {
        final.append(QString::fromWCharArray(wstring, size));
        PyMem_Free(wstring);
#else
    buf = PyString_AsString(obResult); // don't touch that
    if (buf != NULL) {
        final.append(QByteArray(buf,size));
#endif
    } else {
       callback->logError("[stderr read] null string returned");
    }

    if (final.endsWith('\n')) {
        final.chop(1);
    }
    message = final;

    Py_XDECREF(obResult);
    obResult = NULL;

    // Cleaning the StringIO object
    pArgs = PyTuple_New(1); // new ref

    if (!checkPyObject( pArgs)) {
        callback->logError("[stderr cleaning] Error while creating the Python argument tuple");
        Py_XDECREF(pArgs);
        goto leaving;
    }

    inputPy = PyLong_FromLong(0); // new ref
    if (!checkPyObject( inputPy)) {
        callback->logError("[stderr cleaning] Error while creating the Python int value (0)");
        Py_XDECREF(pArgs);
        Py_XDECREF(inputPy);
        goto leaving;
    }

    if (PyTuple_SetItem(pArgs, 0, inputPy) != 0) { // don't need to clean inputPy at this point (stolen)
        callback->logError("[stderr cleaning] Error while assigning the long value to the arg tuple");
        Py_XDECREF(pArgs);
        Py_XDECREF(inputPy);
        goto leaving;
    }

    obResult = PyObject_CallObject(pyTruncateFunc, pArgs); // new ref or NULL
    if (!checkPyObject(obResult)){
        callback->logError("[stderr cleaning] truncate() failed");
    } else {
        Py_XDECREF(obResult);
    }
    obResult = NULL;
    Py_XDECREF(pArgs);

    // seek(0)

    pArgs = PyTuple_New(1); // new ref

    if (!checkPyObject( pArgs)) {
        callback->logError("[stderr cleaning] Error while creating the Python argument tuple");
        Py_XDECREF(pArgs);
        goto leaving;
    }

    inputPy = PyLong_FromLong(0); // new ref
    if (!checkPyObject( inputPy)) {
        callback->logError("[stderr cleaning] Error while creating the Python int value (0)");
        Py_XDECREF(pArgs);
        Py_XDECREF(inputPy);
        goto leaving;
    }

    if (PyTuple_SetItem(pArgs, 0, inputPy) != 0) { // don't need to clean inputPy at this point (stolen)
        Py_XDECREF(pArgs);
        Py_XDECREF(inputPy);
        goto leaving;
    }

    obResult = PyObject_CallObject(pySeekFunc, pArgs); // new ref or NULL
    if (!checkPyObject(obResult)){
        callback->logError("[stderr cleaning] seek() failed");
    } else {
        Py_XDECREF(obResult);
    }

    Py_XDECREF(pArgs);
leaving:
    PyGILState_Release(lgstate);

    return message;
}

void PythonModules::cleaningPyObjs()
{
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();
    Py_XDECREF(pyTruncateFunc);
    pyTruncateFunc = NULL;
    Py_XDECREF(pyGetValFunc);
    pyGetValFunc = NULL;
    Py_XDECREF(pySeekFunc);
    pySeekFunc = NULL;
    Py_XDECREF(pyStringIO);
    pyStringIO = NULL;
    PyGILState_Release(lgstate);
}

bool PythonModules::checkPyObject(PyObject *obj)
{
    return !(PyErr_Occurred() || obj == NULL);
}
