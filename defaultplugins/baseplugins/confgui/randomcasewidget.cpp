/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "randomcasewidget.h"
#include "ui_randomcasewidget.h"

RandomCaseWidget::RandomCaseWidget(RandomCase *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::RandomCaseWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::RandomCaseWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
}

RandomCaseWidget::~RandomCaseWidget()
{
    delete ui;
}

void RandomCaseWidget::onRerandomize()
{
    transform->reRandomize();
}
