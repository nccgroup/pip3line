/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "pluginconfwidget.h"
#include "ui_pluginconfwidget.h"
#include <QDebug>

PluginConfWidget::PluginConfWidget(TransformFactoryPluginInterface *nplugin, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::PluginConfWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::PluginConfWidget X{");
    }
    plugin = nplugin;
    ui->setupUi(this);
    ui->pluginNamelabel->setText(plugin->pluginName());

    QWidget * gui = plugin->getConfGui(this);

    if (gui != NULL) {
        ui->mainLayout->addWidget(gui);
    }

    QStringList typesList = plugin->getTypesList();

    QString types;
    for (int i = 0; i < typesList.size(); i++) {
        types.append(typesList.at(i)).append(": ");
        types.append(QString::number(plugin->getTransformList(typesList.at(i)).size()));
        types.append("\n");
    }

    ui->pluginTransformLabel->setText(types);
}

PluginConfWidget::~PluginConfWidget()
{
    delete ui;
}
