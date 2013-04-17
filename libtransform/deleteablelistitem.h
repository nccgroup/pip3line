/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef DELETEABLELISTITEM_H
#define DELETEABLELISTITEM_H

#include "libtransform_global.h"
#include <QWidget>
#include <QPixmap>

namespace Ui {
class DeleteableListItem;
}

class LIBTRANSFORMSHARED_EXPORT DeleteableListItem : public QWidget
{
        Q_OBJECT
        
    public:
        explicit DeleteableListItem(const QString &text, const QPixmap &im = QPixmap(), QWidget *parent = 0);
        ~DeleteableListItem();
        QString getName();
        void setEnableDelete(bool val);
    signals:
        void itemDeleted(QString name);
    private slots:
        void onDelete();
    private:
        Ui::DeleteableListItem *ui;
};

#endif // DELETEABLELISTITEM_H
