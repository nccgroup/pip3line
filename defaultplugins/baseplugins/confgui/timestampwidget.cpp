/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "timestampwidget.h"
#include "ui_timestampwidget.h"

TimestampWidget::TimestampWidget(TimeStamp *ntransform, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimestampWidget)
{
    transform = ntransform;
    ui->setupUi(this);

    ui->formatLineEdit->setText(transform->getDateFormat());
    connect(ui->formatLineEdit, SIGNAL(textChanged(QString)), this, SLOT(formatChanged(QString)));
}

TimestampWidget::~TimestampWidget()
{
    delete ui;
}

void TimestampWidget::formatChanged(QString format)
{
    transform->setDateFormat(format);
}
