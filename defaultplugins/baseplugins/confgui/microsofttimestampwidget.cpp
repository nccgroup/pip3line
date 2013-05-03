/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "microsofttimestampwidget.h"
#include "ui_microsofttimestampwidget.h"

MicrosoftTimestampWidget::MicrosoftTimestampWidget(MicrosoftTimestamp *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::MicrosoftTimestampWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::MicrosoftTimestampWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->formatLineEdit->setText(transform->getDateFormat());
    connect(ui->formatLineEdit, SIGNAL(textChanged(QString)), this, SLOT(formatChanged(QString)));
}

MicrosoftTimestampWidget::~MicrosoftTimestampWidget()
{
    delete ui;
}

void MicrosoftTimestampWidget::formatChanged(QString format)
{
    transform->setDateFormat(format);
}
