/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "rubyplugin.h"
#include <QDebug>
#include "../../version.h"

const QString RubyPlugin::RUBY_TYPE = "Ruby";

RubyPlugin::RubyPlugin()
{
    callback = 0;
    gui = 0;
    ruby_init();
    ruby_init_loadpath();
    ruby_script("pip3line");
    qDebug() << "Created " << this;
}

RubyPlugin::~RubyPlugin()
{
    qDebug() << "Destroying " << this;
}

void RubyPlugin::setCallBack(Pip3lineCallback *ncallback)
{
    callback = ncallback;
}

QString RubyPlugin::pluginName() const
{
    return "Ruby plugin";
}

QString RubyPlugin::compiledWithQTversion() const
{
    return QT_VERSION_STR;
}

TransformAbstract *RubyPlugin::getTransform(QString name)
{
    TransformAbstract *ta = 0;

    return ta;
}

const QStringList RubyPlugin::getTransformList(QString typeName)
{
    QStringList ret;
    if (typeName == RUBY_TYPE) {

    }
    return ret;
}

QWidget *RubyPlugin::getConfGui(QWidget *parent)
{
    return gui;
}

const QStringList RubyPlugin::getTypesList()
{
    return QStringList() << RUBY_TYPE;
}

int RubyPlugin::getLibTransformVersion() const
{
    return LIB_TRANSFORM_VERSION;
}

QString RubyPlugin::pluginVersion() const
{
    return VERSION_STRING;
}

QT_BEGIN_NAMESPACE
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(rubyplugin, RubyPlugin)
#endif
QT_END_NAMESPACE
