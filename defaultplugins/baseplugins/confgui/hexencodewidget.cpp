/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "hexencodewidget.h"
#include "ui_hexencodewidget.h"

HexEncodeWidget::HexEncodeWidget(HexEncode *ntransform, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HexEncodeWidget)
{
    transform = ntransform;
    ui->setupUi(this);
    switch(transform->getType()) {
        case HexEncode::NORMAL:
            ui->normalRadioButton->setChecked(true);
            break;
        case HexEncode::ESCAPED:
            ui->escapedRadioButton->setChecked(true);
            break;
        case HexEncode::CSTYLE:
            ui->cstyleRadioButton->setChecked(true);
            break;
        case HexEncode::CSV:
            ui->csvRadioButton->setChecked(true);
            break;
        default:
            ui->normalRadioButton->setChecked(true);
    }

    connect(ui->normalRadioButton, SIGNAL(clicked()), this, SLOT(onTypeChange()));
    connect(ui->escapedRadioButton, SIGNAL(clicked()), this, SLOT(onTypeChange()));
    connect(ui->cstyleRadioButton, SIGNAL(clicked()), this, SLOT(onTypeChange()));
    connect(ui->csvRadioButton, SIGNAL(clicked()), this, SLOT(onTypeChange()));
}

HexEncodeWidget::~HexEncodeWidget()
{
    delete ui;
}

void HexEncodeWidget::onTypeChange()
{
    if (ui->normalRadioButton->isChecked())
        transform->setType(HexEncode::NORMAL);
    else if (ui->escapedRadioButton->isChecked())
        transform->setType(HexEncode::ESCAPED);
    else if (ui->cstyleRadioButton->isChecked())
        transform->setType(HexEncode::CSTYLE);
    else
        transform->setType(HexEncode::CSV);
}
