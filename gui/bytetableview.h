/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BYTETABLEVIEW_H
#define BYTETABLEVIEW_H

#include <QTableView>
#include <QAbstractItemDelegate>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QStyledItemDelegate>
#include <QItemSelectionModel>
#include <QMenu>
#include "byteitemmodel.h"

class HexDelegate : public QStyledItemDelegate
{
        Q_OBJECT
    public:
        explicit HexDelegate(QObject *parent = 0);
        ~HexDelegate();
        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem & /* Unused */, const QModelIndex & /* Unused */) const;
        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /* Unused */) const;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const ;
        QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
        void clearSelected();

    private:
        QFont labelFont;
        QHash<int,int> selectedLines;
        static int MAXCOL;
        static int colFlags[];
        static int COMPLETE_LINE;
        QSize normalCell;
        QSize previewCell;
        friend class HexSelectionModel;
};

class HexSelectionModel : public QItemSelectionModel
{
        Q_OBJECT
    public:
        explicit HexSelectionModel(QAbstractItemModel * model);
        explicit HexSelectionModel(QAbstractItemModel * model, QObject * parent);
        ~HexSelectionModel();
        void setDelegate(HexDelegate * delegate);
    public slots:
         void select(const QModelIndex & index, QItemSelectionModel::SelectionFlags command);
         void select(const QItemSelection & selection, QItemSelectionModel::SelectionFlags command);
         void clear();
    private:
         HexDelegate * delegate;
         QPersistentModelIndex startIndex;
         QPersistentModelIndex endIndex;
         friend class ByteTableView;
};

class ByteTableView : public QTableView
{
        Q_OBJECT
    public:
        explicit ByteTableView(QWidget *parent = 0);
        ~ByteTableView();
        void setModel(ByteItemModel * model);
        QByteArray getSelectedBytes();
        int getSelectedBytesCount();
        void deleteSelectedBytes();
        void replaceSelectedBytes(char byte);
        void replaceSelectedBytes(QByteArray data);
        void selectBytes(int pos, int length);
        void selectAllBytes();
        QMenu *getDefaultContextMenu();
        int getLowerSelected() const;
        int getHigherSelected() const;
        void markSelected(const QColor &color, QString text = QString());
        void clearMarkOnSelected();
        bool hasSelection();

    signals:
        void selectionChanged();
    private slots:
        void mousePressEvent(QMouseEvent * event);
        void mouseMoveEvent(QMouseEvent * event);
        void keyPressEvent ( QKeyEvent * event );
        void onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected );
    private:
        void wheelEvent(QWheelEvent * event);
        bool getSelectionInfo(int *pos, int *length);
        void setModel(QAbstractItemModel *) {}
        void setSelectionModel (QItemSelectionModel *) {}
        HexSelectionModel *currentSelectionModel;
        ByteItemModel * currentModel;
        HexDelegate * delegate;
};

#endif // BYTETABLEVIEW_H
