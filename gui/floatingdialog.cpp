/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "floatingdialog.h"
#include "ui_floatingdialog.h"
#include <QDebug>

FloatingDialog::FloatingDialog(QWidget * widget, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FloatingDialog)
{
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

void FloatingDialog::closeEvent(QCloseEvent *event)
{
    reject();
    QDialog::closeEvent(event);
}
