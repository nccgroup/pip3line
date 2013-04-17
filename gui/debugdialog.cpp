/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "debugdialog.h"
#include "ui_debugdialog.h"
#include <transformabstract.h>
#include <QHash>
#include <QHashIterator>
#include <QDebug>

DebugDialog::DebugDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DebugDialog)
{
    ui->setupUi(this);
    connect(this, SIGNAL(rejected()), this, SLOT(deleteLater()));
}

DebugDialog::~DebugDialog()
{
    delete ui;
    qDebug() << "Destroyed " << this;
}

void DebugDialog::on_loadPushButton_clicked()
{
    QString addrString = ui->addressLineEdit->text();
    if (addrString.startsWith("0x"))
        addrString = addrString.mid(2);

    bool ok;
    quint64 addr = addrString.toUInt(&ok, 16);

    QObject *obj = (QObject *) addr;
    const QMetaObject *superClassObj = obj->metaObject()->superClass();

    QObject *parentObj = obj->parent();


    if (superClassObj != 0) {
        QString parentName = superClassObj->className();
        ui->nameValLabel->setText(tr("%1 (%2)").arg(obj->metaObject()->className()).arg(parentName));

        if (parentName == "TransformAbstract") {
            TransformAbstract * ta = (TransformAbstract *) addr;

            QString stringConf;
            QHash<QString, QString> conf = ta->getConfiguration();
            QHashIterator<QString, QString> i(conf);
            while (i.hasNext()) {
                i.next();
                stringConf.append(i.key()).append(" => ").append(i.value()).append("\n");
            }
            if (stringConf.endsWith("\n"))
                stringConf.chop(1);
            ui->confLabel->setText(stringConf);

            int size = 128;
            qDebug() << size << obj << ta;
            char buf[128];
            memcpy(buf, ta, size);

            QByteArray data = QByteArray(buf,size);
            ui->dataPlainTextEdit->appendPlainText(QString::fromUtf8(data.toHex()));

        } else {
            ui->confLabel->clear();
            ui->dataPlainTextEdit->clear();
        }
    }

    else
        ui->nameValLabel->setText(tr("%1").arg(obj->metaObject()->className()));
    if (parentObj == 0)
        ui->parentValLabel->setText(tr("None"));
    else
        ui->parentValLabel->setText(tr("0x%1 %2").arg(QString::number((quint64) parentObj, 16)).arg(parentObj->metaObject()->className()));

    obj->dumpObjectInfo();
    obj->dumpObjectTree();

}

void DebugDialog::closeEvent(QCloseEvent *event)
{
    QDialog::closeEvent(event);
    reject();
}
