/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BASEPLUGINS_H
#define BASEPLUGINS_H

#include "baseplugins_global.h"
#include <transformfactoryplugininterface.h>
#include <pip3linecallback.h>

class BASEPLUGINSSHARED_EXPORT BasePlugins : public QObject, public TransformFactoryPluginInterface {
        Q_OBJECT
#if QT_VERSION >= 0x050000
        Q_PLUGIN_METADATA(IID pluginIID FILE "baseplugin.json")
#endif
        Q_INTERFACES(TransformFactoryPluginInterface)
    public:
        BasePlugins();
        ~BasePlugins();
        QString pluginName() const;
        QString compiledWithQTversion() const;
        TransformAbstract * getTransform(QString name);
        const QStringList getTransformList(QString typeName);
        const QStringList getTypesList();
        QWidget * getConfGui(QWidget * parent);
        int getLibTransformVersion() const;
        QString pluginVersion() const;
        void setCallBack(Pip3lineCallback *);
    private:
        TransformAbstract * getTransformFromFile(QString resFile);
        static const QString Base64Url;
        static const QString BinaryNum;
        static const QString OctalNum;
        static const QString HexaNum;
        Pip3lineCallback * callback;
};

#endif // BASEPLUGINS_H
