/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "xorwidget.h"
#include "ui_xorwidget.h"

XorWidget::XorWidget(Xor *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::XorWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::XorWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    connect(ui->keyPlainTextEdit,SIGNAL(textChanged()),this,SLOT(onKeyChange()));
    connect(ui->fromHexcheckBox,SIGNAL(toggled(bool)),this,SLOT(onFromHexChange(bool)));
    ui->keyPlainTextEdit->appendPlainText(transform->getKey());
    ui->fromHexcheckBox->setChecked(transform->isFromHex());
}

XorWidget::~XorWidget()
{
    delete ui;
}

void XorWidget::onFromHexChange(bool val)
{
    transform->setFromHex(val);
}

void XorWidget::onKeyChange()
{
    transform->setKey(ui->keyPlainTextEdit->toPlainText().toUtf8());
}
