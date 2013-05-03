/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef RUBYPLUGIN_H
#define RUBYPLUGIN_H

#include "rubyplugin_global.h"

#ifdef Q_OS_LINUX
#include <ruby-1.9.1/ruby.h>
#elif defined Q_OS_WIN
#include <Ruby.h>
#endif

#include <transformfactoryplugininterface.h>
#include <QList>
#include <pip3linecallback.h>
#include <modulesmanagement.h>

class RUBYPLUGINSHARED_EXPORT RubyPlugin : public QObject, public TransformFactoryPluginInterface
{
        Q_OBJECT
#if QT_VERSION >= 0x050000
        Q_PLUGIN_METADATA(IID pluginIID FILE "rubyplugin.json")
#endif
        Q_INTERFACES(TransformFactoryPluginInterface)
    public:
        RubyPlugin();
        ~RubyPlugin();
        void setCallBack(Pip3lineCallback * callback);
        QString pluginName() const;
        QString compiledWithQTversion() const;
        TransformAbstract * getTransform(QString name);
        const QStringList getTransformList(QString typeName);
        QWidget * getConfGui(QWidget * parent);
        const QStringList getTypesList();
        int getLibTransformVersion() const;
        QString pluginVersion() const;
    private:
        static const QString RUBY_TYPE;
        Pip3lineCallback *callback;
        QWidget * gui;
};

#endif // RUBYPLUGIN_H
