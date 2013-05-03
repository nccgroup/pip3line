/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "splitwidget.h"
#include "ui_splitwidget.h"

SplitWidget::SplitWidget(Split *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::SplitWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::SplitWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    QString data;
    char c = transform->getSeparator();
    QString hex = QString::fromUtf8(QByteArray(1,c).toHex());

    if (c >= ' ' && c <= '~') {
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
    ui->separatorLineEdit->setText(hex);

    ui->groupSpinBox->setValue(transform->getSelectedGroup());
    ui->groupSpinBox->setMaximum(Split::MAXGROUPVALUE);
    ui->groupSpinBox->setMinimum(0);
    ui->allCheckBox->setChecked(transform->doWeTakeAllGroup());

    connect(ui->separatorLineEdit,SIGNAL(textChanged(QString)), this, SLOT(onSeparatorChange(QString)));
    connect(ui->groupSpinBox,SIGNAL(valueChanged(int)), this, SLOT(onGroupChange(int)));
    connect(ui->allCheckBox,SIGNAL(toggled(bool)), this, SLOT(onAllGroupChange(bool)));
}

SplitWidget::~SplitWidget()
{
    delete ui;
}

void SplitWidget::onGroupChange(int val)
{
    if (!transform->setSelectedGroup(val)) {
        ui->groupSpinBox->setValue(transform->getSelectedGroup());
    } else {
        ui->allCheckBox->blockSignals(true);
        ui->allCheckBox->setChecked(false);
        ui->allCheckBox->blockSignals(false);
    }
}

void SplitWidget::onAllGroupChange(bool val)
{
    transform->setTakeAllGroup(val);
}

void SplitWidget::onSeparatorChange(QString val)
{
    QByteArray input = QByteArray::fromHex(val.toUtf8());
    QString data;
    char c = '\00';
    if (!input.isEmpty()) {
        c = input.at(0);
        if (c >= ' ' && c <= '~') {
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
                    data = tr("\\%1").arg(ui->separatorLineEdit->text());
            }
        }
        ui->label->setText(data);
        transform->setSeparator(c);
    }

}
