/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "paddingwidget.h"
#include "ui_paddingwidget.h"
#include <QDebug>

PaddingWidget::PaddingWidget(Padding *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::PaddingWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::PaddingWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->blockSizeSpinBox->setMinimum(Padding::MINBLOCKSIZE);
    ui->blockSizeSpinBox->setMaximum(Padding::MAXBLOCKSIZE);
    ui->typeComboBox->setCurrentIndex((int)transform->getVariant());
    ui->blockSizeSpinBox->setValue(transform->getBlocksize());
    ui->paddingCharLineEdit->setText(QByteArray().append(transform->getPadChar()).toHex());

    connect(ui->typeComboBox,SIGNAL(currentIndexChanged(int)), this,SLOT(onTypeChange(int)));
    connect(ui->blockSizeSpinBox,SIGNAL(valueChanged(int)), this, SLOT(onBlockSizeChange(int)));
    connect(ui->paddingCharLineEdit,SIGNAL(textChanged(QString)), this, SLOT(onPaddingCharChange(QString)));
}

PaddingWidget::~PaddingWidget()
{
    delete ui;
}

void PaddingWidget::onTypeChange(int index) {
    transform->setVariant((Padding::PaddingVariant)index);
}

void PaddingWidget::onPaddingCharChange(QString value) {
    QByteArray hexCharVal = value.toUtf8();
    if (hexCharVal.size() < 2) {
        return;
    }
    QByteArray val = QByteArray::fromHex(hexCharVal);

    if (val.size() == 0) {
        ui->paddingCharLineEdit->clear();
    } else {
        transform->setPadChar(val.at(0));
        ui->typeComboBox->blockSignals(true);
        ui->typeComboBox->setCurrentIndex((int)Padding::CUSTOM);
        ui->typeComboBox->blockSignals(false);

    }
}

void PaddingWidget::onBlockSizeChange(int size)
{
    transform->setBlockSize(size);
}
