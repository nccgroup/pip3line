/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "charencodingwidget.h"
#include "ui_charencodingwidget.h"
#include <QTextCodec>

CharEncodingWidget::CharEncodingWidget(CharEncoding *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::CharEncodingWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::CharEncodingWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    QList<QByteArray> codecs =  QTextCodec::availableCodecs();
    qSort(codecs);
    for (int i = 0; i < codecs.size(); i++) {
        ui->codecComboBox->addItem(QString(codecs.at(i)),QVariant(codecs.at(i)));
    }
    ui->codecComboBox->setCurrentIndex(ui->codecComboBox->findData((QVariant) transform->getCodecName()));
    connect(ui->codecComboBox,SIGNAL(currentIndexChanged(int)), this, SLOT(onCodecChange()));
}

CharEncodingWidget::~CharEncodingWidget()
{
    delete ui;
}

void CharEncodingWidget::onCodecChange()
{
    transform->setCodecName(ui->codecComboBox->itemData(ui->codecComboBox->currentIndex()).toByteArray());
}

void CharEncodingWidget::on_UTF16PushButton_clicked()
{
    transform->setCodecName("UTF-16");
    ui->codecComboBox->setCurrentIndex(ui->codecComboBox->findData((QVariant) transform->getCodecName()));
}

void CharEncodingWidget::on_latin1PushButton_clicked()
{
    transform->setCodecName("ISO-8859-1");
    ui->codecComboBox->setCurrentIndex(ui->codecComboBox->findData((QVariant) transform->getCodecName()));
}
