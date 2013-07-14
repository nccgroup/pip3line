/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "loggerwidget.h"
#include "ui_loggerwidget.h"
#include <QMutexLocker>
#include <QTime>
#include <QDebug>

using namespace Pip3lineConst;

LoggerWidget::LoggerWidget(QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::LoggerWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::LoggerWidget X{");
    }
    ui->setupUi(this);

    uncheckedError = false;
}

LoggerWidget::~LoggerWidget()
{
    delete ui;
}

void LoggerWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    uncheckedError = false;
    emit cleanStatus();
}

bool LoggerWidget::hasUncheckedError() const
{
    return uncheckedError;
}

void LoggerWidget::logError(const QString &message, const QString &source)
{
    addMessage(message,source,LERROR);
}

void LoggerWidget::logWarning(const QString &message, const QString &source)
{
    addMessage(message,source,LWARNING);
}

void LoggerWidget::logStatus(const QString &message, const QString &source)
{
    addMessage(message,source,LSTATUS);
}

void LoggerWidget::logMessage(const QString &message, const QString &source, Pip3lineConst::LOGLEVEL level)
{
    addMessage(message,source,level);
}


void LoggerWidget::addMessage(const QString &message, const QString &source, LOGLEVEL level)
{
    QMutexLocker locking(&lock);
    QString fmess;
    QColor color;


    fmess = QString("[%1]").arg(QTime::currentTime().toString());

    if (!source.isEmpty())
        fmess.append(source).append(": ").append(message);
    else
        fmess.append(message);

    switch (level) {
        case LERROR:
            color = Qt::red;
            qCritical() << fmess;
            if (!(uncheckedError && isVisible())) {
                uncheckedError = true;
                emit errorRaised();
            }
            break;
        case LWARNING:
            color = Qt::blue;
            qWarning() << fmess;
            break;
        default:
            color = Qt::black;
            qWarning() << fmess;
            break;

    }

    ui->messagesTextEdit->setTextColor(color);
    ui->messagesTextEdit->append(fmess);
}
