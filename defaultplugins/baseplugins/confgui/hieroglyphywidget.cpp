/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "hieroglyphywidget.h"
#include "ui_hieroglyphywidget.h"

HieroglyphyWidget::HieroglyphyWidget(Hieroglyphy * ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::HieroglyphyWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::HieroglyphyWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->btoaCheckBox->setChecked(transform->isBtoaInUse());
}

HieroglyphyWidget::~HieroglyphyWidget()
{
    delete ui;
}

void HieroglyphyWidget::on_btoaCheckBox_clicked()
{
    transform->setUseBtoa(ui->btoaCheckBox->isChecked());
}
