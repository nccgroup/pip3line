/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "regexphelpdialog.h"
#include "ui_regexphelpdialog.h"

RegExpHelpDialog::RegExpHelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegExpHelpDialog)
{
    ui->setupUi(this);
}

RegExpHelpDialog::~RegExpHelpDialog()
{
    delete ui;
}
