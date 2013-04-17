/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef OPENSSLPLUGIN_H
#define OPENSSLPLUGIN_H

#include "opensslplugin_global.h"
#include <transformfactoryplugininterface.h>
#include <QMutex>
#include <pip3linecallback.h>

class OPENSSLPLUGINSHARED_EXPORT OpensslPlugin : public QObject, public TransformFactoryPluginInterface
{
        Q_OBJECT
#if QT_VERSION >= 0x050000
        Q_PLUGIN_METADATA(IID pluginIID FILE "opensslplugin.json")
#endif
        Q_INTERFACES(TransformFactoryPluginInterface)
    public:
        static const QStringList blacklistHash;
        OpensslPlugin();
        ~OpensslPlugin();
        QString pluginName() const;
        QString compiledWithQTversion() const;
        TransformAbstract * getTransform(QString name);
        const QStringList getTransformList(QString type);
        const QStringList getTypesList();
        QWidget * getConfGui(QWidget * parent);
        int getLibTransformVersion() const;
        QString pluginVersion() const;
        void setCallBack(Pip3lineCallback *) {}
    private slots:
        void onGuiDelete();
    private:
        static QMutex hashListLocker;
        QWidget *gui;
};

#endif // OPENSSLPLUGIN_H
