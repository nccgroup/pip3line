/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "customdialogdockwidget.h"
#include "ui_customdialogdockwidget.h"

CustomDialogDockWidget::CustomDialogDockWidget(const QString &title, QWidget *parent, QWidget *item) :
    QDockWidget(parent)
{
    ui = new(std::nothrow) Ui::CustomDialogDockWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::CustomDialogDockWidget X{");
    }
    ui->setupUi(this);
    this->setWindowTitle(title);
    if (item != NULL)
        ui->verticalLayout->addWidget(item);
}

CustomDialogDockWidget::~CustomDialogDockWidget()
{
    delete ui;
}

void CustomDialogDockWidget::addWidget(QWidget * item)
{
    ui->verticalLayout->addWidget(item);
}
