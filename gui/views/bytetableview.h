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
#include <QSize>
#include <QValidator>
#include <QLabel>
#include <QBitArray>

class SearchAbstract;

// need to subclass QLabel just for the text cells of the hexview, in order to apply a specific background color
// there is probably a better way
class TextCell : public QLabel
{
    Q_OBJECT
    public:
        explicit TextCell(QWidget *parent = 0, Qt::WindowFlags f = 0);
        explicit TextCell(const QString &text, QWidget *parent = 0, Qt::WindowFlags f = 0);
};

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
        void selectAll();
    public slots:
        void setColumnCount(int val);
        void onEditorValid();
    private:
        Q_DISABLE_COPY(HexDelegate)
        QHash<int,int> selectedLines;
        static int colFlags[];
        static int COMPLETE_LINE;
        int hexColumncount;
        QSize normalCellSize;
        QSize previewCellSize;
        bool allSelected;
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
         void selectAll();
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
        qint64 getSelectedBytesCount();
        void deleteSelectedBytes();
        void replaceSelectedBytes(char byte);
        void replaceSelectedBytes(QByteArray data);
        void selectBytes(int pos, int length);
        void selectAllBytes();
        QMenu *getDefaultContextMenu();
        int getLowerSelected() const;
        int getHigherSelected() const;
        int getCurrentPos() const;
        void markSelected(const QColor &color, QString text = QString());
        void clearMarkOnSelected();
        bool hasSelection();
        void search(QByteArray item, QBitArray mask);
        void searchAgain();
        static const int MAXCOL;
        static const int MINCOL;
        static const int TEXTCOLUMNWIDTH;
        static const int HEXCOLUMNWIDTH;
        static const int DEFAULTROWSHEIGHT;
        static const QFont RegularFont;
    public slots:
        void setColumnCount(int val);
        void gotoSearch(quint64 soffset, quint64 eoffset);
        bool goTo(quint64 offset, bool absolute,bool negative, bool select = false);
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
        QSize sizeHint() const;
        HexSelectionModel *currentSelectionModel;
        ByteItemModel * currentModel;
        HexDelegate * delegate;
        int hexColumncount;
        quint64 lastSearchIndex;
        QByteArray lastSearch;
        QBitArray lastMask;
        int currentVerticalHeaderWidth;
        SearchAbstract *searchObject;
};

#endif // BYTETABLEVIEW_H
