/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef FILEWIDGET_H
#define FILEWIDGET_H

#include <QWidget>
#include <QModelIndex>
#include <QSize>
#include <QBitArray>

namespace Ui {
class FileWidget;
}

class LargeFile;

class FileWidget : public QWidget
{
    Q_OBJECT
    
    public:
        explicit FileWidget(LargeFile *fsource, QWidget *parent = 0);
        ~FileWidget();
        QSize sizeHint() const;

    signals:
        void readonlyChange(bool val);
    private slots:
        void refresh();
    private:
        Ui::FileWidget *ui;
        LargeFile *source;
};

#endif // FILEWIDGET_H
