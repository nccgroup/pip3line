/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "rotxwidget.h"
#include "ui_rotxwidget.h"

RotXWidget::RotXWidget(Rotx *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::RotXWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::RotXWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->typeComboBox->setCurrentIndex(transform->getRotVariant());
    connect(ui->typeComboBox,SIGNAL(currentIndexChanged(int)), this,SLOT(onUpdateType(int)));
}

RotXWidget::~RotXWidget()
{
    delete ui;
}

void RotXWidget::onUpdateType(int val)
{
    transform->setRotVariant((Rotx::RotVariant)val);
}
