/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "newbytedialog.h"
#include "ui_newbytedialog.h"

NewByteDialog::NewByteDialog(QWidget *parent, bool isSimple) :
    QDialog(parent),
    ui(new Ui::NewByteDialog)
{
    ui->setupUi(this);
    if (isSimple) {
        ui->countLabel->setVisible(false);
        ui->countSpinBox->setVisible(false);
    }
}

NewByteDialog::~NewByteDialog()
{
    delete ui;
}

int NewByteDialog::byteCount()
{
    return ui->countSpinBox->value();
}

char NewByteDialog::getChar()
{
    return ui->hexWidget->getChar();
}

void NewByteDialog::closeEvent(QCloseEvent *event)
{
    reject();
    QDialog::closeEvent(event);
}
