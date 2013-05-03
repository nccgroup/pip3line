/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "hexwidget.h"
#include "ui_hexwidget.h"
#include <transformabstract.h>

HexWidget::HexWidget(QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::HexWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::HexWidget X{");
    }
    ui->setupUi(this);
    connect(ui->hexLineEdit, SIGNAL(textEdited(QString)),this, SLOT(onHexChanged()));
    setChar('\00');
}

HexWidget::~HexWidget()
{
    delete ui;
}

char HexWidget::getChar()
{
    QByteArray input = QByteArray::fromHex(ui->hexLineEdit->text().toUtf8());
    if (!input.isEmpty()) {
        return input.at(0);
    }
    return '\00';
}

void HexWidget::setChar(char c)
{
    QString data;
    QString hex = QString::fromUtf8(QByteArray(1,c).toHex());

    if (TransformAbstract::isPrintable((qint32)c)) {
        data = QString::fromUtf8(&c, 1);
    } else {
        switch (c) {
            case '\n':
                data = "\\n";
                break;
            case '\r':
                data = "\\r";
                break;
            default:
                data = tr("\\%1").arg(hex);
        }
    }

    ui->label->setText(data);
    ui->hexLineEdit->setText(hex);
}

void HexWidget::onHexChanged()
{
    QByteArray input = QByteArray::fromHex(ui->hexLineEdit->text().toUtf8());
    QString data;
    char c = '\00';
    if (!input.isEmpty()) {
        c = input.at(0);
        if (TransformAbstract::isPrintable((qint32)c)) {
            data = QString::fromUtf8(&c, 1);
        } else {
            switch (c) {
                case '\n':
                    data = "\\n";
                    break;
                case '\r':
                    data = "\\r";
                    break;
                default:
                    data = tr("\\%1").arg(ui->hexLineEdit->text());
            }
        }
    }
    ui->label->setText(data);
    emit charChanged(c);
}
