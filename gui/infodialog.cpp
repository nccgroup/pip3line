/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "infodialog.h"
#include "ui_infodialog.h"

InfoDialog::InfoDialog(TransformAbstract *ntransform, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    transform = ntransform;
    ui->setupUi(this);

    ui->helpTextEdit->setHtml(transform->help());
    if (!transform->credits().isEmpty()) {
        ui->creditsTextEdit->setHtml(transform->credits());
    } else {
        ui->creditsTab->setVisible(false);
    }
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

TransformAbstract *InfoDialog::getTransform()
{
    return transform;
}
