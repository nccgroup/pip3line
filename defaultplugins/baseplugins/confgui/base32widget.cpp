/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "base32widget.h"
#include "ui_base32widget.h"

Base32Widget::Base32Widget(Base32 *ntransform, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Base32Widget)
{
    transform = ntransform;
    ui->setupUi(this);
    ui->variantComboBox->addItems(Base32::VARIATIONS);
    ui->paddingCheckBox->setChecked(transform->isPaddingIncluded());
    ui->paddingLineEdit->setText(QString(transform->getPaddingChar()));
    ui->variantComboBox->setCurrentIndex(transform->getVariant());
    ui->charsetDisplay->setText(QString(transform->getCharTable()));

    connect(ui->variantComboBox,SIGNAL(currentIndexChanged(int)), this, SLOT(onVariantchange(int)));
    connect(ui->paddingCheckBox,SIGNAL(toggled(bool)), this, SLOT(onIncludePaddingToggled(bool)));
    connect(ui->paddingLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onPaddingChange(QString)));
}

Base32Widget::~Base32Widget()
{
    delete ui;
}

void Base32Widget::onVariantchange(int newVariant)
{
    transform->setVariant((Base32::CharSetVariant)newVariant);
    ui->charsetDisplay->setText(QString(transform->getCharTable()));
}

void Base32Widget::onPaddingChange(QString val)
{
    if (!val.isEmpty()) {
        QByteArray c = val.toUtf8();
        if (c.size() > 1) {
            ui->paddingLineEdit->clear();
            return;
        }
        char ch = c.at(0);

        if (!transform->setPaddingChar(ch)) {
            ui->paddingLineEdit->clear();
        } else {
            ui->paddingCheckBox->setChecked(true);
        }
    } else {
        ui->paddingCheckBox->setChecked(false);
    }
}

void Base32Widget::onIncludePaddingToggled(bool val)
{
    transform->setIncludePadding(val);
}
