/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "zlibwidget.h"
#include "ui_zlibwidget.h"


ZlibWidget::ZlibWidget(Zlib *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::ZlibWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::ZlibWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->compressionSpinBox->setValue(transform->getCompression());
    ui->removeHeaderCheckBox->setChecked(transform->doRemoveHeader());
}

ZlibWidget::~ZlibWidget()
{
    delete ui;
}

void ZlibWidget::on_compressionSpinBox_valueChanged(int value)
{
    if (!transform->setCompression(value)) {
        ui->compressionSpinBox->setValue(transform->getCompression());
    }
}

void ZlibWidget::on_removeHeaderCheckBox_toggled(bool checked)
{
    transform->setRemoveHeader(checked);
}
