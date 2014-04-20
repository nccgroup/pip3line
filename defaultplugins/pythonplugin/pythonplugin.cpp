/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "pythonplugin.h"
#include "pythontransform.h"
#include "../../version.h"
#include "pythonmodules.h"
#include <QDebug>
#include <transformmgmt.h>
#include <pip3linecallback.h>
#include <modulesmanagement.h>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <pip3linecallback.h>
#include <modulesmanagement.h>


PythonPlugin::PythonPlugin()
{
    callback = NULL;
    gui = NULL;
    modules = NULL;

    qDebug() << "Created " << this;
}

PythonPlugin::~PythonPlugin()
{
    qDebug() << "Destroying " << this;

    delete modules;
}

void PythonPlugin::setCallBack(Pip3lineCallback *ncallback)
{
    callback = ncallback;

    modules = new(std::nothrow) PythonModules(PythonTransform::id, callback);

    if (modules == NULL) {
       qFatal("Cannot allocate memory for ModulesManagement (PythonPlugin) X{");
       return;
    }
    modules->initialize();
}

QString PythonPlugin::pluginName() const
{

    return QString("%1 plugin").arg(PythonModules::PYTHON_TYPE);
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

    return ta;
}

const QStringList PythonPlugin::getTransformList(QString typeName)
{
    QStringList ret;
    if (typeName == PythonModules::PYTHON_TYPE) {
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
    return QStringList() << PythonModules::PYTHON_TYPE;
}

int PythonPlugin::getLibTransformVersion() const
{
    return LIB_TRANSFORM_VERSION;
}

QString PythonPlugin::pluginVersion() const
{
    return VERSION_STRING;
}


QT_BEGIN_NAMESPACE
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(pythonplugin, PythonPlugin)
#endif
QT_END_NAMESPACE
