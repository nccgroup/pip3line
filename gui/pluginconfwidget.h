/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PLUGINCONFWIDGET_H
#define PLUGINCONFWIDGET_H

#include <QWidget>
#include <transformfactoryplugininterface.h>
#include <QDialog>

namespace Ui {
class PluginConfWidget;
}

class PluginConfWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit PluginConfWidget(TransformFactoryPluginInterface * plugin, QWidget *parent = 0);
        ~PluginConfWidget();
    private:
        Ui::PluginConfWidget *ui;
        TransformFactoryPluginInterface * plugin;
};

#endif // PLUGINCONFWIDGET_H
