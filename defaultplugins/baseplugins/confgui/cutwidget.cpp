/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "cutwidget.h"
#include "ui_cutwidget.h"

CutWidget::CutWidget(Cut *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::CutWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::CutWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->fromSpinBox->setValue(transform->getFromPos());
    ui->lengthSpinBox->setValue(transform->getLength());
    ui->everythingCheckBox->setChecked(transform->doCutEverything());
    connect(ui->fromSpinBox,SIGNAL(valueChanged(int)),this,SLOT(onFromChange(int)));
    connect(ui->lengthSpinBox,SIGNAL(valueChanged(int)),this,SLOT(onLengthChange(int)));
    connect(ui->everythingCheckBox,SIGNAL(toggled(bool)),this,SLOT(onEverythingChange(bool)));
}

CutWidget::~CutWidget()
{
    delete ui;
}


void CutWidget::onFromChange(int value)
{
    if (!transform->setFromPos(value)) {
        ui->lengthSpinBox->blockSignals(true);
        ui->lengthSpinBox->setValue(transform->getLength());
        ui->lengthSpinBox->blockSignals(false);
    }
}

void CutWidget::onLengthChange(int value)
{
    if (!transform->setLength(value)) {
        ui->lengthSpinBox->blockSignals(true);
        ui->lengthSpinBox->setValue(transform->getLength());
        ui->lengthSpinBox->blockSignals(false);
    } else {
        ui->everythingCheckBox->blockSignals(true);
        ui->everythingCheckBox->setChecked(false);
        ui->everythingCheckBox->blockSignals(false);
    }
}

void CutWidget::onEverythingChange(bool value)
{
    transform->setCutEverything(value);
}
