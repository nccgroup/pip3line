/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "floatingdialog.h"
#include "ui_floatingdialog.h"
#include <QDebug>

FloatingDialog::FloatingDialog(GuiHelper * guiHelper, QWidget * widget, QWidget *parent) :
    AppDialog(guiHelper, parent)
{
    allowReject = true;
    ui = new(std::nothrow) Ui::FloatingDialog();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::FloatingDialog X{");
    }
    ui->setupUi(this);
    setModal(false);
    ui->verticalLayout->addWidget(widget);
    widget->setVisible(true);
    qDebug() << "Created " << this;
}

FloatingDialog::~FloatingDialog()
{
    delete ui;
    qDebug() << "Destroyed " << this;
}

void FloatingDialog::setAllowReject(bool val)
{
    allowReject = val;
}

void FloatingDialog::closeEvent(QCloseEvent *event)
{
    if (allowReject) {
        reject();
        QDialog::closeEvent(event);
    }
}
