/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "messagedialog.h"
#include "ui_messagedialog.h"
#include <QMutexLocker>

MessageDialog::MessageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageDialog)
{
    ui->setupUi(this);
    warningCount = 0;
    errorCount = 0;

}

MessageDialog::~MessageDialog()
{
    delete ui;
}

long MessageDialog::errors()
{
    return errorCount;
}

long MessageDialog::warnings()
{
    return warningCount;
}

bool MessageDialog::hasMessages()
{
    return errorCount != 0 || warningCount != 0;
}

void MessageDialog::setJustShowMessages(bool flag)
{
    if (flag) {
        ui->buttonBox->clear();
        ui->buttonBox->addButton(QDialogButtonBox::Ok);
    } else {
        ui->buttonBox->clear();
        ui->buttonBox->addButton(QDialogButtonBox::Ignore);
        ui->buttonBox->addButton(QDialogButtonBox::Cancel);
    }
}

void MessageDialog::logWarning(const QString message, const QString source) {
    QMutexLocker locking(&lock);
    QString fmess;
    if (!source.isEmpty())
        fmess.append(source).append(":").append(message);
    else
        fmess.append(source).append(message);
    addMessage(fmess,Qt::blue);
    warningCount++;
}

void MessageDialog::logError(const QString message, const QString source) {
    QMutexLocker locking(&lock);
    QString fmess;
    if (!source.isEmpty())
        fmess.append(source).append(":").append(message);
    else
        fmess.append(source).append(message);
    addMessage(fmess,Qt::red);
    errorCount++;
}

void MessageDialog::clearMessages()
{
    QMutexLocker locking(&lock);
    ui->messagesDisplay->clear();
    warningCount = 0;
    errorCount = 0;
}

void MessageDialog::addMessage(const QString &message, QColor color)
{
    ui->messagesDisplay->setTextColor(color);
    ui->messagesDisplay->append(message);
}
