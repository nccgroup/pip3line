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
class GuiHelper;
class SearchWidget;
class QTimerEvent;
class FileSearch;
class LargeFile;
class FoundOffsetsModel;

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
        void onSearch(QByteArray item, QBitArray mask, bool);
        void refresh();
        void itemFound(quint64 soffset,quint64 eoffset);
        void onDoubleClick(QModelIndex index);
        void onEndSearch();
        //        void onRightClick(QPoint pos);
        //        void contextMenuAction(QAction * action);
    private:
        Ui::FileWidget *ui;
        LargeFile *source;
        FileSearch * multiSearch;
        FoundOffsetsModel *itemsFoundModel;
        SearchWidget *searchWidget;
};

#endif // FILEWIDGET_H
