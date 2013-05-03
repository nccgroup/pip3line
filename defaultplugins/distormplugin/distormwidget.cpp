/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "distormwidget.h"
#include "ui_distormwidget.h"

const QStringList DistormWidget::asmTypeStrings = QStringList() << "16 Bits" << "32 Bits" << "64 Bits";

DistormWidget::DistormWidget(DistormTransf *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::DistormWidget();
    if (ui == NULL)
        qFatal("Cannot allocate memory for Ui::DistormWidget X{");

    transform = ntransform;
    ui->setupUi(this);
    ui->asmTypeComboBox->addItems(asmTypeStrings);
    switch(transform->getDecodeType()) {
        case DistormTransf::A16Bits:
            ui->asmTypeComboBox->setCurrentIndex(0);
            break;
        case DistormTransf::A32Bits:
            ui->asmTypeComboBox->setCurrentIndex(1);
            break;
        case DistormTransf::A64Bits:
            ui->asmTypeComboBox->setCurrentIndex(2);
            break;
    }

    ui->offsetLineEdit->setText(QString::number(transform->getOffset(),16)); // hex format

    connect(ui->maxInstructionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onMaxInstruc(int)));
    connect(ui->offsetLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onOffset(QString)));
    connect(ui->asmTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onType(int)));


}

DistormWidget::~DistormWidget()
{
    delete ui;
}

void DistormWidget::onMaxInstruc(int val)
{
    transform->setMaxInstruction((uint)val);
}

void DistormWidget::onOffset(QString val)
{
    quint64 realVal = 0;
    bool ok;
    realVal = val.toULongLong(&ok, 16);
    if (!ok) {
        ui->offsetLineEdit->setStyleSheet("QLineEdit { border: 1px ridge red }");
    } else {
        ui->offsetLineEdit->setStyleSheet("");
        transform->setOffset(realVal);
    }
}

void DistormWidget::onType(int val)
{
    if (val == 0) {
        transform->setDecodeType(DistormTransf::A16Bits);
    } else if (val == 1) {
        transform->setDecodeType(DistormTransf::A32Bits);
    } else if (val == 2) {
        transform->setDecodeType(DistormTransf::A64Bits);
    }
}
