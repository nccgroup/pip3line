/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "loggerwidget.h"
#include "ui_loggerwidget.h"
#include <QMutexLocker>


LoggerWidget::LoggerWidget(QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::LoggerWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::LoggerWidget X{");
    }
    ui->setupUi(this);
    cerr = new(std::nothrow) QTextStream(stderr);
    if (cerr == NULL)
        qFatal("Cannot allocate memory for logger cerr X{");
    cout = new(std::nothrow) QTextStream(stdout);
    if (cout == NULL)
        qFatal("Cannot allocate memory for logger cout X{");
    uncheckedError = false;
}

LoggerWidget::~LoggerWidget()
{
    delete ui;
    delete cerr;
    delete cout;
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
    addMessage(message,source,Error);
}

void LoggerWidget::logWarning(const QString &message, const QString &source)
{
    addMessage(message,source,Warning);
}

void LoggerWidget::logStatus(const QString &message, const QString &source)
{
    addMessage(message,source,Normal);
}


void LoggerWidget::addMessage(const QString &message, const QString &source, const Levels level)
{
    QMutexLocker locking(&lock);
    QString fmess;
    QColor color;



    if (!source.isEmpty())
        fmess.append(source).append(": ").append(message);
    else
        fmess.append(message);

    switch (level) {
        case Error:
            color = Qt::red;
            *cerr << fmess << endl;
            if (!(uncheckedError && isVisible())) {
                uncheckedError = true;
                emit errorRaised();
            }
            break;
        case Warning:
            color = Qt::blue;
            *cerr << fmess << endl;
            break;
        default:
            color = Qt::black;
            *cout << fmess << endl;
            break;

    }

    ui->messagesTextEdit->setTextColor(color);
    ui->messagesTextEdit->append(fmess);
}
