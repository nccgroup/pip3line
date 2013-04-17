/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "bytetableview.h"
#include "byteitemmodel.h"
#include <QTextStream>
#include <QMouseEvent>
#include <QPainter>
#include <QtCore/qmath.h>
#include <QApplication>
#include <QClipboard>
#include <QItemSelectionRange>
#include <QHeaderView>
#include <QScrollBar>
#include <QDebug>

int HexDelegate::MAXCOL = 32;
int HexDelegate::colFlags[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
                                 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000};
int HexDelegate::COMPLETE_LINE = 0x8001;

HexDelegate::HexDelegate(QObject *parent) : QStyledItemDelegate (parent), normalCell(24,20),  previewCell(130,20){
    labelFont.setFamily("Courier New");
    labelFont.setPointSize(10);
    qDebug() << "Created: " << this;
}

HexDelegate::~HexDelegate()
{
    qDebug() << "Destroyed: " << this;
}

QWidget *HexDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & /* Unused */, const QModelIndex & /* Unused */) const {
    QLineEdit *editor = new QLineEdit(parent);
    editor->setInputMask("HH");
    editor->setFrame(false);
    return editor;
}

void HexDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QLineEdit * weditor = static_cast<QLineEdit *>(editor);
    weditor->setText(index.model()->data(index, Qt::EditRole).toByteArray());
}

void HexDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QLineEdit * weditor = static_cast<QLineEdit *>(editor);
    model->setData(index, weditor->text(), Qt::EditRole);
}

void HexDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /* Unused */) const {
    editor->setGeometry(option.rect);
}

void HexDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.column() == ((ByteItemModel *)index.model())->getColumnNumbers()) {
        QStyleOptionViewItemV4 optionV4 = option;
        initStyleOption(&optionV4, index);
        int maxColumn = qMin(((ByteItemModel *)index.model())->getColumnNumbers(),MAXCOL);

        QStyle *style = optionV4.widget? optionV4.widget->style() : QApplication::style();

        QLabel textData;
        textData.setTextFormat(Qt::PlainText);
        textData.setText(optionV4.text);
        int row = index.row();
        textData.setFont(labelFont);
        textData.setTextInteractionFlags(Qt::TextSelectableByKeyboard);

        if (selectedLines.contains(row)) {
            int rowval = selectedLines.value(row);
            if ( rowval == COMPLETE_LINE)
                textData.setSelection(0,optionV4.text.size());
            else {

                int start = maxColumn + 1;
                int length = 1;
                bool selectionActive = false;
                for (int i = 0; i < maxColumn ; i++) {
                    if (rowval & colFlags[i]) {
                        selectionActive = ! selectionActive;
                    }
                    if (selectionActive) {
                        if (start == maxColumn + 1)
                            start = i;
                        length++;
                    }

                }
                if (selectionActive)
                    length = 1;
                textData.setSelection(start,length);
            }
        }

        textData.setStyleSheet("QLabel { background-color : white; color : black; }");

        optionV4.text = QString();
        style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter);


        QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4);
        painter->save();
        QPoint offset = textRect.topLeft();
        painter->translate(offset);
        painter->setClipRect(textRect.translated(-offset));
        textData.render(painter);
        painter->restore();

    } else
        QStyledItemDelegate::paint(painter, option, index);
}

QSize HexDelegate::sizeHint(const QStyleOptionViewItem & /*unused*/, const QModelIndex & index) const {
    if (index.column() == ((ByteItemModel *)index.model())->getColumnNumbers()) {
        return previewCell;
    } else
        return normalCell;
}

void HexDelegate::clearSelected()
{
    selectedLines.clear();
}

// ===================================== SelectionModel functions =============================================

HexSelectionModel::HexSelectionModel(QAbstractItemModel *model) : QItemSelectionModel(model)
{
    qDebug() << "Created: " << this;
}

HexSelectionModel::HexSelectionModel(QAbstractItemModel *model, QObject *parent) : QItemSelectionModel(model,parent)
{
    qDebug() << "Created: " << this << " parent:" << parent;
}

HexSelectionModel::~HexSelectionModel()
{
    qDebug() << "Destroyed: " << this;
}

void HexSelectionModel::setDelegate(HexDelegate *ndelegate)
{
    delegate = ndelegate;
}


void HexSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
    delegate->clearSelected();
    delegate->selectedLines.insert(index.row(),HexDelegate::colFlags[index.column()]);
    QItemSelectionModel::select(index,command);
}

void HexSelectionModel::select(const QItemSelection &/* Unused */, QItemSelectionModel::SelectionFlags command)
{
    QItemSelection newSelection;
    delegate->clearSelected();
    if (!startIndex.isValid() || !endIndex.isValid()) {
        return;
    }

    int maxColumn = ((ByteItemModel *)model())->getColumnNumbers() -1;

    if (startIndex == endIndex) {
        delegate->selectedLines.insert(startIndex.row(),HexDelegate::colFlags[startIndex.column()]);
        newSelection.select(startIndex,startIndex);
    } else if (startIndex.row() == endIndex.row()) {
        delegate->selectedLines.insert(startIndex.row(),HexDelegate::colFlags[startIndex.column()] | HexDelegate::colFlags[endIndex.column()]);
        newSelection.select(startIndex,endIndex);
    } else {
        QPersistentModelIndex temp = startIndex;
        QPersistentModelIndex start = startIndex;
        QPersistentModelIndex end = endIndex;
        if (start.row() > end.row()) {
            start = end;
            end = temp;
        }

        newSelection.select(start, start.sibling(start.row(),maxColumn));
        delegate->selectedLines.insert(start.row(),HexDelegate::colFlags[start.column()] | HexDelegate::colFlags[maxColumn]);
        newSelection.select(end.sibling(end.row(),0), end);
        delegate->selectedLines.insert(end.row(),HexDelegate::colFlags[0] | HexDelegate::colFlags[end.column()]);

        for (int i = start.row() + 1; i < end.row(); i++) {
            newSelection.select(start.sibling(i, 0),start.sibling(i,maxColumn));
            delegate->selectedLines.insert(i,HexDelegate::COMPLETE_LINE);
        }
    }

    QItemSelectionModel::select(newSelection,command);

}

void HexSelectionModel::clear()
{
    delegate->clearSelected();
    QItemSelectionModel::clear();
}


// ===================================== TableView functions =============================================
ByteTableView::ByteTableView(QWidget *parent) :
    QTableView(parent)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    delegate = new HexDelegate(this);
    setItemDelegate(delegate);
    setSelectionMode(QAbstractItemView::ContiguousSelection);
    //setAttribute(Qt::WA_NoMousePropagation);
#if QT_VERSION >= 0x050000
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
#else
    verticalHeader()->setResizeMode(QHeaderView::Fixed);
    horizontalHeader()->setResizeMode(QHeaderView::Fixed);
#endif
    currentSelectionModel = 0;
    currentModel = 0;
    qDebug() << "Created: " << this;

}

ByteTableView::~ByteTableView()
{
    // no need to delete currentModel as it should be taken care by the parent
    delete delegate;
    if (currentSelectionModel != 0)
        delete currentSelectionModel;
    qDebug() << "Destroyed: " << this;
}

void ByteTableView::setModel(ByteItemModel *nmodel)
{
    QAbstractItemModel *m = QTableView::model();
    QTableView::setModel(nmodel);
    delete m;

    currentModel = nmodel;
    for (int i = 0; i < 16; i++)
        setColumnWidth(i,25);
    setColumnWidth(16,131);

    currentSelectionModel = new HexSelectionModel(nmodel, this);
    currentSelectionModel->setDelegate(delegate);

    connect(currentSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this , SLOT(onSelectionChanged(QItemSelection,QItemSelection)));

    QItemSelectionModel *sm = QTableView::selectionModel();
    QTableView::setSelectionModel(currentSelectionModel);
    delete sm;
}

void ByteTableView::mousePressEvent(QMouseEvent *event)
{
    QPersistentModelIndex current = indexAt(event->pos());
    if (event->button() == Qt::RightButton) {
        QTableView::mousePressEvent(event);
        return;
    }
    currentSelectionModel->clear();
    if (current.isValid()) {
        if (currentModel->position(current) == ByteItemModel::INVALID_POSITION) {
            current = current.sibling(-1,-1);
            viewport()->update();
        }
        currentSelectionModel->startIndex = current;
        currentSelectionModel->endIndex = current;
    }
    QTableView::mousePressEvent(event);

}

void ByteTableView::mouseMoveEvent(QMouseEvent *event)
{
    QPersistentModelIndex current = indexAt(event->pos());
    if (current.isValid() && currentModel->position(current) != ByteItemModel::INVALID_POSITION ) {
        currentSelectionModel->endIndex = indexAt(event->pos());
    }
    QTableView::mouseMoveEvent(event);
}

void ByteTableView::keyPressEvent(QKeyEvent *event)
{

    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        switch (event->key())
        {
            case Qt::Key_A:
                selectAllBytes();
                event->accept();
                break;
            case Qt::Key_C:
            {
                QClipboard *clipboard = QApplication::clipboard();
                QString temp = QString::fromUtf8(getSelectedBytes().toHex());
                if (event->modifiers().testFlag(Qt::ShiftModifier)) {
                    clipboard->setText(temp);
                } else {
                    QString temp2;
                    for ( int i = 0; i < temp.size(); i += 2) {
                        temp2.append(QString("\\x%1%2").arg(temp.at(i)).arg(temp.at(i+1)));
                    }
                    clipboard->setText(temp2);
                }
                event->accept();
            }
                break;
            case Qt::Key_Z:
                if (event->modifiers().testFlag(Qt::ShiftModifier)) {
                    currentModel->historyForward();
                    event->accept();
                }
                else {
                    currentModel->historyBackward();
                    event->accept();
                }
                break;
            default:
                QAbstractItemView::keyPressEvent(event);
        }
    } else if (event->key() == Qt::Key_Delete && event->modifiers().testFlag(Qt::NoModifier)) {
        deleteSelectedBytes();
        event->accept();
    } else if (event->modifiers().testFlag(Qt::AltModifier)) {
        QClipboard *clipboard = QApplication::clipboard();
        QString temp = QString::fromUtf8(getSelectedBytes().toHex());
        QString temp2;
        temp2.append("{");
        for (int i = 0; i < temp.size(); i += 2) {
            temp2.append(QString(" 0x%1%2,").arg(temp.at(i)).arg(temp.at(i+1)));
        }
        temp2.chop(1);
        temp2.append(" }");
        clipboard->setText(temp2);

        event->accept();
    } else {
        QAbstractItemView::keyPressEvent(event);
    }
}

void ByteTableView::onSelectionChanged(const QItemSelection & /* Unused */, const QItemSelection & /* Unused */)
{
    viewport()->update();
    emit selectionChanged();
}

void ByteTableView::wheelEvent(QWheelEvent *event)
{
    if (verticalScrollBar()->isVisible()) {
        setAttribute(Qt::WA_NoMousePropagation);
    }
    QTableView::wheelEvent(event);
    setAttribute(Qt::WA_NoMousePropagation, false);
}

bool ByteTableView::getSelectionInfo(int *pos, int *length)
{
    if (currentSelectionModel->startIndex.isValid() && currentSelectionModel->endIndex.isValid()) {
        int pos1 = currentModel->position(currentSelectionModel->startIndex);
        int pos2 = currentModel->position(currentSelectionModel->endIndex);
        (*length) = qAbs(pos1 -pos2) + 1;
        if (pos1 <= pos2)
            (*pos) = pos1;
        else
            (*pos) = pos2;

        return true;
    }
    return false;
}

QByteArray ByteTableView::getSelectedBytes()
{
    QByteArray ret;

    int pos = 0;
    int length = 0;
    if (getSelectionInfo(&pos, &length)) {
        ret = currentModel->extract(pos, length);
    }

    return ret;
}

int ByteTableView::getSelectedBytesCount()
{
    int count = 0;
    int pos = 0;

    getSelectionInfo(&pos, &count);

    return count;
}

void ByteTableView::deleteSelectedBytes()
{
    int pos = 0;
    int length = 0;
    if (getSelectionInfo(&pos, &length)) {
        currentModel->remove(pos, length);
    }
}

void ByteTableView::replaceSelectedBytes(char byte)
{
    int pos = 0;
    int length = 0;
    if (getSelectionInfo(&pos, &length)) {
        currentModel->replace(pos, length, QByteArray(length, byte));
    }
}

void ByteTableView::replaceSelectedBytes(QByteArray data)
{
    int pos = 0;
    int length = 0;
    if (getSelectionInfo(&pos, &length)) {
        currentModel->replace(pos, length, data);
        if (data.size() != length ) {
            selectBytes(pos,data.size());
        }
    }
}

void ByteTableView::selectBytes(int pos, int length)
{
    currentSelectionModel->clear();
    currentSelectionModel->startIndex = currentModel->createIndex(pos);
    currentSelectionModel->endIndex = currentModel->createIndex(pos + length - 1);
    currentSelectionModel->select(QItemSelection(), QItemSelectionModel::Select);
}

void ByteTableView::selectAllBytes()
{
    currentSelectionModel->startIndex = currentModel->createIndex(0);
    currentSelectionModel->endIndex = currentModel->createIndex(currentModel->size() - 1);
    currentSelectionModel->select(QItemSelection(), QItemSelectionModel::Select);
}


int ByteTableView::getLowerSelected() const
{
    QPersistentModelIndex ref = currentSelectionModel->startIndex;
    if (!ref.isValid()) {
        return 0;
    }

    if (currentSelectionModel->endIndex < ref)
        ref = currentSelectionModel->endIndex;

    return currentModel->position(ref);
}

int ByteTableView::getHigherSelected() const
{
    QPersistentModelIndex ref = currentSelectionModel->endIndex;
    if (!ref.isValid()) {
        return currentModel->size() -1;
    }

    if (ref < currentSelectionModel->startIndex )
        ref = currentSelectionModel->startIndex;

    return currentModel->position(ref);
}

void ByteTableView::markSelected(const QColor &color, QString text)
{
    if (currentSelectionModel->startIndex.isValid() && currentSelectionModel->endIndex.isValid()) {
        int pos1 = currentModel->position(currentSelectionModel->startIndex);
        int pos2 = currentModel->position(currentSelectionModel->endIndex);

        currentModel->mark(pos1, pos2, color, text);
    }
}

void ByteTableView::clearMarkOnSelected()
{
    if (currentSelectionModel->startIndex.isValid() && currentSelectionModel->endIndex.isValid()) {
        int pos1 = currentModel->position(currentSelectionModel->startIndex);
        int pos2 = currentModel->position(currentSelectionModel->endIndex);

        currentModel->clearMarking(pos1, pos2);
    }
}

bool ByteTableView::hasSelection()
{
    return currentSelectionModel->startIndex.isValid() && currentSelectionModel->endIndex.isValid();
}
