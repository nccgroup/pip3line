/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "xmlquerywidget.h"
#include "ui_xmlquerywidget.h"

XmlQueryWidget::XmlQueryWidget(XmlQuery *ntransform, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::XmlQueryWidget)
{
    transform = ntransform;
    ui->setupUi(this);
    ui->queryPlainTextEdit->appendPlainText(transform->getQueryString());
    connect(ui->submitPushButton, SIGNAL(clicked()), this, SLOT(onQuerySubmit()));
}

XmlQueryWidget::~XmlQueryWidget()
{
    delete ui;
}

void XmlQueryWidget::onQuerySubmit()
{
    if (!ui->queryPlainTextEdit->toPlainText().isEmpty()) {
        transform->setQueryString(ui->queryPlainTextEdit->toPlainText());
    }
}
