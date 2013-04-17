/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MODULESMANAGEMENTWIDGET_H
#define MODULESMANAGEMENTWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include "libtransform_global.h"
class ModulesManagement;
#include "modulesmanagement.h"

namespace Ui {
class ModulesManagementWidget;
}

class LIBTRANSFORMSHARED_EXPORT ModulesManagementWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit ModulesManagementWidget(ModulesManagement * modulesMgmt, QWidget *parent = 0);
        ~ModulesManagementWidget();

    private Q_SLOTS:
        void onManualClicked();
        void loadModules();
        void loadPaths();
        void unload(QString name = QString());
        void onModuleSelected(QListWidgetItem* item);
    private:
        QString moduleTitle;
        ModulesManagement * modulesMgmt;
        Ui::ModulesManagementWidget *ui;
};

#endif // MODULESMANAGEMENTWIDGET_H
