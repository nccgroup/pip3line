/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef QUICKVIEWDIALOG_H
#define QUICKVIEWDIALOG_H

#include <QDialog>
#include <QList>
#include <QReadWriteLock>
#include "quickviewitem.h"
#include "guihelper.h"

namespace Ui {
class QuickViewDialog;
}

class QuickViewDialog : public QDialog
{
        Q_OBJECT
        
    public:
        explicit QuickViewDialog(GuiHelper * guiHelper, QWidget *parent = 0);
        ~QuickViewDialog();
    public slots:
        void receivingData(const QByteArray &data);
    private slots:
        void NewItem();
        void itemDeleted();
        void onReset();

    private:
        void addItem(const QString & conf);
        Ui::QuickViewDialog *ui;
        QList<QuickViewItem *> itemList;
        GuiHelper * guiHelper;
        QByteArray currentData;
        QReadWriteLock dataLock;
        QReadWriteLock listLock;
};

#endif // QUICKVIEWDIALOG_H
