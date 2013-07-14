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
#include <QKeyEvent>
#include "byteitemmodel.h"

#include <QValidator>

class HexValidator : public QValidator
{
        Q_OBJECT
    public:
        explicit HexValidator(int size, QObject *parent = 0);
        State validate(QString & input, int & pos) const;
    private:
        int maxSize;
};

class HexLineEdit : public QLineEdit
{
        Q_OBJECT
    public:
        explicit HexLineEdit(QWidget *parent = 0);
    signals:
        void inputValid();
    private slots:
        void onInputChanged();
    private:
        void keyPressEvent(QKeyEvent * event);
};

class HexDelegate : public QStyledItemDelegate
{
        Q_OBJECT
    public:
        explicit HexDelegate(int hexColumncount, QObject *parent = 0);
        ~HexDelegate();
        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem & /* Unused */, const QModelIndex & /* Unused */) const;
        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /* Unused */) const;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const ;
        QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
        void clearSelected();
    public slots:
        void setColumnCount(int val);
        void onEditorValid();
    private:
        Q_DISABLE_COPY(HexDelegate)
        QFont labelFont;
        QHash<int,int> selectedLines;
        static int colFlags[];
        static int COMPLETE_LINE;
        int hexColumncount;
        QSize normalCell;
        QSize previewCell;
        friend class HexSelectionModel;
};

class HexSelectionModel : public QItemSelectionModel
{
        Q_OBJECT
    public:
        explicit HexSelectionModel(int hexColumncount,QAbstractItemModel * model);
        explicit HexSelectionModel(int hexColumncount,QAbstractItemModel * model, QObject * parent);
        ~HexSelectionModel();
        void setDelegate(HexDelegate * delegate);
    public slots:
         void select(const QModelIndex & index, QItemSelectionModel::SelectionFlags command);
         void select(const QItemSelection & selection, QItemSelectionModel::SelectionFlags command);
         void clear();
         void setColumnCount(int val);
    private:
         Q_DISABLE_COPY(HexSelectionModel)
         HexDelegate * delegate;
         QPersistentModelIndex startIndex;
         QPersistentModelIndex endIndex;
         friend class ByteTableView;
         int hexColumncount;
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
        qint64 getCurrentPos() const;
        void markSelected(const QColor &color, QString text = QString());
        void clearMarkOnSelected();
        bool hasSelection();
        bool goTo(qint64 offset, bool absolute, bool select = false);
        bool search(QByteArray item, bool flag = false);
        static int MAXCOL;
        static int MINCOL;
        static int TEXTCOLUMNWIDTH;
        static int HEXCOLUMNWIDTH;
    public slots:
        void setColumnCount(int val);
    signals:
        void newSelection();
        void error(QString mess, QString source);
        void warning(QString mess, QString source);
    private slots:
        void mousePressEvent(QMouseEvent * event);
        void mouseMoveEvent(QMouseEvent * event);
        void keyPressEvent ( QKeyEvent * event );
        void onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected );
        void closeEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint);
        QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    private:
        Q_DISABLE_COPY(ByteTableView)
        static const QString LOGID;
        void wheelEvent(QWheelEvent * event);
        bool getSelectionInfo(int *pos, int *length);
        void setModel(QAbstractItemModel *) {}
        void setSelectionModel (QItemSelectionModel *) {}
        HexSelectionModel *currentSelectionModel;
        ByteItemModel * currentModel;
        HexDelegate * delegate;
        int hexColumncount;
        qint64 lastSearchIndex;
};

#endif // BYTETABLEVIEW_H
