/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include <QDialog>
#include <QColor>
#include <QMutex>

namespace Ui {
class MessageDialog;
}

class MessageDialog : public QDialog
{
    Q_OBJECT
    
    public:
        explicit MessageDialog(QWidget *parent = 0);
        ~MessageDialog();
        long errors();
        long warnings();
        bool hasMessages();
        void setJustShowMessages(bool flag);

    public slots:
        void logWarning(const QString message, const QString source = QString());
        void logError(const QString message, const QString source = QString());
        void clearMessages();

    private:
        void addMessage(const QString &message, QColor color);
        QMutex lock;
        Ui::MessageDialog *ui;
        long warningCount;
        long errorCount;
};

#endif // MESSAGEDIALOG_H
