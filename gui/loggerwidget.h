/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef LOGGERWIDGET_H
#define LOGGERWIDGET_H

#include <QWidget>
#include <QMutex>
#include <QTextStream>
#include <QShowEvent>

namespace Ui {
class LoggerWidget;
}

class LoggerWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit LoggerWidget(QWidget *parent = 0);
        ~LoggerWidget();
        void showEvent ( QShowEvent * event );
        bool hasUncheckedError() const;
    signals:
        void errorRaised();
        void cleanStatus();
    public slots:
        void logError(const QString &message, const QString &source = QString());
        void logWarning(const QString &message,const QString &source = QString());
        void logStatus(const QString &message,const QString &source = QString());
    private:
        enum Levels {Normal = 0, Warning, Error};
        void addMessage(const QString &message, const QString &source, const Levels level);
        Ui::LoggerWidget *ui;
        QMutex lock;
        QTextStream *cerr;
        QTextStream *cout;
        bool uncheckedError;
};

#endif // LOGGERWIDGET_H
