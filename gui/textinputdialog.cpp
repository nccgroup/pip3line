/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "textinputdialog.h"
#include "ui_textinputdialog.h"

TextInputDialog::TextInputDialog(QWidget *parent) :
    QDialog(parent)
{
    ui = new(std::nothrow) Ui::textInputDialog;
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::textInputDialog X{");
    }
    ui->setupUi(this);
}

TextInputDialog::~TextInputDialog()
{
    delete ui;
}

void TextInputDialog::setTextLabel(QString text)
{
    ui->inputLabel->setText(text);
}

void TextInputDialog::setPixLabel(QPixmap pix)
{
    ui->inputLabel->setPixmap(pix);
}

QString TextInputDialog::getInputText() const
{
    return ui->inputLineEdit->text();
}

void TextInputDialog::setText(QString text)
{
    ui->inputLineEdit->setText(text);
    ui->inputLineEdit->selectAll();
}


