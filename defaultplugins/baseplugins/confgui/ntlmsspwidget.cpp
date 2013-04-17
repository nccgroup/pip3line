/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "ntlmsspwidget.h"
#include "ui_ntlmsspwidget.h"

NtlmsspWidget::NtlmsspWidget(Ntlmssp *ntransform, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NtlmsspWidget)
{
    transform = ntransform;
    ui->setupUi(this);
    ui->decodeCheckBox->setChecked(transform->decodeBase64());

    connect(ui->decodeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onDecode(bool)));
}

NtlmsspWidget::~NtlmsspWidget()
{
    delete ui;
}

void NtlmsspWidget::onDecode(bool checked)
{
    transform->setDecodeBase64(checked);
}
