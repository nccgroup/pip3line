/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "deleteablelistitem.h"
#include "ui_deleteablelistitem.h"
#include <QDebug>

DeleteableListItem::DeleteableListItem(const QString &text, const QPixmap &im, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeleteableListItem)
{
    ui->setupUi(this);
    ui->textLabel->setText(text);

    if (!im.isNull())
        ui->pixLabel->setPixmap(im);
    connect(ui->deletePushButton, SIGNAL(clicked()), this, SLOT(onDelete()));
}

DeleteableListItem::~DeleteableListItem()
{
   // qDebug() << "D3stroying " << this;
    delete ui;
}

QString DeleteableListItem::getName()
{
    return ui->textLabel->text();
}

void DeleteableListItem::setEnableDelete(bool val)
{
    ui->deletePushButton->setEnabled(val);
}


void DeleteableListItem::onDelete()
{
    emit itemDeleted(ui->textLabel->text());
    deleteLater();
}
