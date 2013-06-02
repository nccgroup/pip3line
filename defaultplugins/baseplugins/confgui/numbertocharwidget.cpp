/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "numbertocharwidget.h"

#include "ui_numbertocharwidget.h"

NumberToCharWidget::NumberToCharWidget(NumberToChar *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::NumberToCharWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::NumberToCharWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->separatorLineEdit->setText(QByteArray(1,transform->getSeparator()));

    connect(ui->separatorLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onSeparatorChanged(QString)));
}

NumberToCharWidget::~NumberToCharWidget()
{
    delete ui;
}

void NumberToCharWidget::onSeparatorChanged(QString val)
{
    if (val.size() == 1) { // one and only character
        QByteArray c = val.toUtf8();
        if (c.size() > 1) { // ignoring multi-bytes characters
            ui->separatorLineEdit->clear();
            return;
        }
        char ch = c.at(0);
        if (!transform->setSeparator(ch)) {
            ui->separatorLineEdit->clear();
        }
    }
}


