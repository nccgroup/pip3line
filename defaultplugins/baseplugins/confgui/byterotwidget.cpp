/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "byterotwidget.h"
#include "ui_byterotwidget.h"
#include "../byterot.h"

ByteRotWidget::ByteRotWidget(ByteRot * transf, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::ByteRotWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::ByteRotWidget X{");
    }

    ui->setupUi(this);
    transform = transf;
    ui->spinBox->setValue(transform->getRotation());

    connect(ui->spinBox, SIGNAL(valueChanged(int)), SLOT(onRotationChanged(int)));
}

ByteRotWidget::~ByteRotWidget()
{
    delete ui;
}

void ByteRotWidget::onRotationChanged(int val)
{
    transform->setRotation(val);
}
