/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef DISTORMPLUGIN_H
#define DISTORMPLUGIN_H

#include "distorm_global.h"
#include <transformfactoryplugininterface.h>
#include <pip3linecallback.h>

class DISTORMSHARED_EXPORT DistormPlugin : public QObject, public TransformFactoryPluginInterface
{
        Q_OBJECT
#if QT_VERSION >= 0x050000
        Q_PLUGIN_METADATA(IID pluginIID FILE "distormplugin.json")
#endif
        Q_INTERFACES(TransformFactoryPluginInterface)
    public:
        DistormPlugin();
        ~DistormPlugin();
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
        QWidget * gui;
        Pip3lineCallback *callback;
};

#endif // DISTORMPLUGIN_H
