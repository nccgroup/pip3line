/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "modulesmanagementwidget.h"
#include "ui_modulesmanagementwidget.h"
#include "deleteablelistitem.h"
#include <QFileDialog>

ModulesManagementWidget::ModulesManagementWidget(ModulesManagement *nmodulesMgmt, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModulesManagementWidget)
{
    modulesMgmt = nmodulesMgmt;
    moduleTitle = modulesMgmt->getLangName();
    ui->setupUi(this);

    connect(modulesMgmt,SIGNAL(modulesUpdated()), this, SLOT(loadModules()));
    connect(modulesMgmt,SIGNAL(pathsUpdated()), this, SLOT(loadPaths()));
    connect(ui->modulesListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onModuleSelected(QListWidgetItem*)));
    connect(ui->loadPushButton, SIGNAL(clicked()), this, SLOT(onManualClicked()));

    loadPaths();
    loadModules();
}

ModulesManagementWidget::~ModulesManagementWidget()
{
    delete ui;
}

void ModulesManagementWidget::onManualClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose %1 module file").arg(moduleTitle),"", tr("%1 files (*%2)").arg(moduleTitle).arg(modulesMgmt->getExtension()));
    if (!fileName.isEmpty()) {
        modulesMgmt->addModule(fileName,ModulesManagement::PERSISTENT);
    }
}

void ModulesManagementWidget::loadModules()
{
    ui->modulesListWidget->clear();
    QStringList list = modulesMgmt->getModulesList();
    for (int i = 0; i < list.size(); i++) {
        DeleteableListItem *itemWid = new DeleteableListItem(list.at(i));
        if (modulesMgmt->getModuleType(list.at(i)) == ModulesManagement::AUTO) {
            itemWid->setEnableDelete(false);
        } else {
            itemWid->setEnableDelete(true);
        }

        connect(itemWid, SIGNAL(itemDeleted(QString)), this, SLOT(unload(QString)));
        QListWidgetItem *item = new QListWidgetItem();
        ui->modulesListWidget->addItem(item);
        ui->modulesListWidget->setItemWidget(item, itemWid);
    }
    ui->modulesListWidget->setCurrentRow(0);

    ui->modulesListWidget->sortItems();
}

void ModulesManagementWidget::loadPaths()
{
    ui->pathListWidget->clear();
    ui->pathListWidget->addItems(modulesMgmt->getPathsList());
    ui->pathListWidget->sortItems();
}

void ModulesManagementWidget::unload(QString name)
{
    if (name.isEmpty())
        name = ui->modulesListWidget->currentItem()->text();

    if (!name.isEmpty())
        modulesMgmt->removeModule(name);

}

void ModulesManagementWidget::onModuleSelected(QListWidgetItem *item)
{
    QString name = ((DeleteableListItem *)ui->modulesListWidget->itemWidget(item))->getName();
}
