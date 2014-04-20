/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "bytetableview.h"
#include "byteitemmodel.h"
#include "../crossplatform.h"
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
#include <QTime>
#include <QMessageBox>
#include <QFontMetrics>
#include <QTimer>
#if QT_VERSION >= 0x050000
#include <QtConcurrent>
#endif
#include <QtConcurrentRun>
#include "../sources/searchabstract.h"


TextCell::TextCell(QWidget *parent, Qt::WindowFlags f) :
    QLabel(parent,f)
{

}

TextCell::TextCell(const QString &text, QWidget *parent, Qt::WindowFlags f) :
    QLabel(text, parent,f)
{

}


HexValidator::HexValidator(int size, QObject *parent) :
    QValidator(parent)
{
    maxSize = size < 0 ? 0 : size;
}

QValidator::State HexValidator::validate(QString &input, int &) const
{
    if (input.isEmpty() || input.size() < maxSize)
        return QValidator::Intermediate;

    if (input.size() > maxSize)
        return QValidator::Invalid;

    // we don't need to verify if these are hex characters, as the filter should already prevent invalid char

    return QValidator::Acceptable;
}

HexLineEdit::HexLineEdit(QWidget *parent) : QLineEdit(parent)
{
    setInputMask("HH");
    setFrame(false);
    HexValidator *validator = new(std::nothrow) HexValidator(maxLength(),parent);
    if (validator == NULL) {
        qFatal("Cannot allocate memory for HexValidator X{");
    } else {
        setValidator(validator);
    }
    connect(this, SIGNAL(textEdited(QString)), this, SLOT(onInputChanged()));
}

void HexLineEdit::onInputChanged()
{
    if (hasAcceptableInput()) {
        emit inputValid();
    }
}

void HexLineEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
        event->accept();
    else
        QLineEdit::keyPressEvent(event);
}

// ===============================================================================

int HexDelegate::colFlags[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
                                 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000};
int HexDelegate::COMPLETE_LINE = 0x8001;

HexDelegate::HexDelegate(int nhexColumncount, QObject *parent) : QStyledItemDelegate (parent), normalCellSize(24,20),  previewCellSize(130,20){
    hexColumncount = nhexColumncount;
    allSelected = false;
    //qDebug() << "Created: " << this;
}

HexDelegate::~HexDelegate()
{
    //qDebug() << "Destroyed: " << this;
}

QWidget *HexDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & /* Unused */, const QModelIndex & /* Unused */) const {
    HexLineEdit *editor = new(std::nothrow) HexLineEdit(parent);
    if (editor == NULL) {
        qFatal("Cannot allocate memory for delegate editor X{");
    }
    connect(editor, SIGNAL(inputValid()), this, SLOT(onEditorValid()));
    return editor;
}

void HexDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QLineEdit * weditor = static_cast<QLineEdit *>(editor);
    weditor->setText(index.model()->data(index, Qt::DisplayRole).toString());
}

void HexDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QLineEdit * weditor = static_cast<QLineEdit *>(editor);
    model->setData(index, weditor->text(), Qt::EditRole);
}

void HexDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /* Unused */) const {
    editor->setGeometry(option.rect);
}

void HexDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.column() == hexColumncount) {
        QStyleOptionViewItemV4 optionV4 = option;
        initStyleOption(&optionV4, index);


        QStyle *style = optionV4.widget? optionV4.widget->style() : QApplication::style();

        TextCell textData;
        textData.setTextFormat(Qt::PlainText);
        textData.setText(optionV4.text);
        textData.setFont(ByteTableView::RegularFont);
        textData.setTextInteractionFlags(Qt::TextSelectableByKeyboard);

        int row = index.row();
        int textSize = optionV4.text.size();
        int maxColumn = qMin(qMin(hexColumncount,ByteTableView::MAXCOL),textSize);
        // apply the selection from the hexa cells
        if (allSelected) {
            textData.setSelection(0,textSize);
        } else if (selectedLines.contains(row)) {
            int rowval = selectedLines.value(row);
            if ( rowval == COMPLETE_LINE)
                textData.setSelection(0,textSize);
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
//                if (start < textSize)
                    textData.setSelection(start,length);
            }
        }

        optionV4.text = QString();
        style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter);


        QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4);
        painter->save();
        QPoint offset = textRect.topLeft();
        painter->translate(offset);
        painter->setClipRect(textRect.translated(-offset));
        textData.render(painter,QPoint(1,2));
        painter->restore();

    } else
        QStyledItemDelegate::paint(painter, option, index);
}

QSize HexDelegate::sizeHint(const QStyleOptionViewItem & /*unused*/, const QModelIndex & index) const {
    if (index.column() == hexColumncount) {
        return previewCellSize;
    } else
        return normalCellSize;
}

void HexDelegate::clearSelected()
{
    selectedLines.clear();
    allSelected = false;
}

void HexDelegate::selectAll()
{
    selectedLines.clear();
    allSelected = true;
}

void HexDelegate::setColumnCount(int val)
{
    if (val < ByteTableView::MINCOL || val > ByteTableView::MAXCOL)
        qWarning("invalid column count for HexDelegate, ignoring");
    else {
        hexColumncount = val;
    }
}

void HexDelegate::onEditorValid()
{
    QLineEdit * editor = static_cast<QLineEdit *>(sender());
    emit commitData(editor);
    emit closeEditor(editor, QAbstractItemDelegate::EditNextItem);
}

// ===================================== SelectionModel functions =============================================

HexSelectionModel::HexSelectionModel(int nhexColumncount, QAbstractItemModel *model) : QItemSelectionModel(model)
{
    delegate = NULL;
    hexColumncount = nhexColumncount;
    //qDebug() << "Created: " << this;
}

HexSelectionModel::HexSelectionModel(int nhexColumncount, QAbstractItemModel *model, QObject *parent) : QItemSelectionModel(model,parent)
{
    delegate = NULL;
    hexColumncount = nhexColumncount;
    //qDebug() << "Created: " << this << " parent:" << parent;
}

HexSelectionModel::~HexSelectionModel()
{
    //qDebug() << "Destroyed: " << this;
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

void HexSelectionModel::select(const QItemSelection & , QItemSelectionModel::SelectionFlags command)
{
    QItemSelection newSelection;

    delegate->clearSelected();
    if (!startIndex.isValid() || !endIndex.isValid()) {
        return;
    }

    int maxColumn = hexColumncount - 1;

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
        if ((end.row() - start.row()) > 1) {
            newSelection.select(start.sibling(start.row() + 1, 0),start.sibling(end.row() - 1,maxColumn));
            for (int i = start.row() + 1; i < end.row(); i++) {
                delegate->selectedLines.insert(i,HexDelegate::COMPLETE_LINE);
            }
        }
    }
    QItemSelectionModel::select(newSelection,command);

}

void HexSelectionModel::selectAll()
{   // startIndex and endIndex should have been set externally
    QItemSelection newSelection;
    int maxColumn = hexColumncount - 1;
    delegate->clearSelected();


    newSelection.select(startIndex, endIndex.sibling(endIndex.row(),maxColumn));
    delegate->selectAll();
    QItemSelectionModel::select(newSelection,QItemSelectionModel::Select);

}

void HexSelectionModel::clear()
{
    // this function only clear the delegate and the parent selection
    // it does not clear the actual selection (on purpose)
    delegate->clearSelected();

    QItemSelectionModel::clear();
}

void HexSelectionModel::setColumnCount(int val)
{
    if (val < ByteTableView::MINCOL || val > ByteTableView::MAXCOL)
        qWarning("invalid column count for HexSelectionModel, ignoring");
    else {
        hexColumncount = val;
    }
}


// ===================================== TableView functions =============================================

const int ByteTableView::MAXCOL = 32;
const int ByteTableView::MINCOL = 16;
const int ByteTableView::TEXTCOLUMNWIDTH = 131; // arbitratry number, dont' ask
const int ByteTableView::HEXCOLUMNWIDTH = 28; // arbitratry number, dont' ask
const int ByteTableView::DEFAULTROWSHEIGHT = 20; // arbitratry number, dont' ask
const QString ByteTableView::LOGID = "ByteTableView";
const QFont ByteTableView::RegularFont = QFont("Courier New",10);

ByteTableView::ByteTableView(QWidget *parent) :
    QTableView(parent)
{
    hexColumncount = MINCOL;
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    delegate = new(std::nothrow) HexDelegate(hexColumncount,this);
    if (delegate != NULL) {
        setItemDelegate(delegate);
    } else {
        qFatal("Cannot allocate memory for hex delegate X{");
    }
    setSelectionMode(QAbstractItemView::ContiguousSelection);

#if QT_VERSION >= 0x050000
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
#else
    verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(QHeaderView::Fixed);
#endif
    verticalHeader()->setFont(RegularFont);
    horizontalHeader()->setFont(RegularFont);
    currentSelectionModel = NULL;
    currentModel = NULL;
    searchObject = NULL;
    lastSearchIndex = ULONG_LONG_MAX;
    currentVerticalHeaderWidth = 0;
    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::MinimumExpanding);

    //qDebug() << "Created: " << this;

}

ByteTableView::~ByteTableView()
{
    currentModel = NULL;// no need to delete currentModel as it should be taken care by the parent
    searchObject = NULL;
    delete delegate;
    delete currentSelectionModel;
    //qDebug() << "Destroyed: " << this;
}

void ByteTableView::setModel(ByteItemModel *nmodel)
{
    QAbstractItemModel *m = QTableView::model();
    QTableView::setModel(nmodel);
    delete m;

    currentModel = nmodel;
    searchObject = currentModel->getSource()->getSearchObject();
    if (searchObject != NULL) {
        searchObject->setStopAtFirst(true);
        connect(searchObject, SIGNAL(itemFound(quint64,quint64)), SLOT(gotoSearch(quint64,quint64)), Qt::QueuedConnection);
    } else {
        qDebug() << "[ByteTableView] NULL search object returned by " << currentModel->getSource();
    }
    currentModel->setHexColumnCount(hexColumncount);
    for (int i = 0; i < hexColumncount; i++)
        setColumnWidth(i,HEXCOLUMNWIDTH);
    setColumnWidth(hexColumncount,TEXTCOLUMNWIDTH);
    verticalHeader()->setDefaultSectionSize(DEFAULTROWSHEIGHT);

    currentSelectionModel = new(std::nothrow) HexSelectionModel(hexColumncount, nmodel, this);
    if (currentSelectionModel == NULL) {
        qFatal("Cannot allocate memory for currentSelectionModel X{");
    } else {

        currentSelectionModel->setDelegate(delegate);

        connect(currentSelectionModel,
                SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this ,
                SLOT(onSelectionChanged(QItemSelection,QItemSelection)));

        QItemSelectionModel *sm = QTableView::selectionModel();
        QTableView::setSelectionModel(currentSelectionModel);
        delete sm;
    }
}

void ByteTableView::mousePressEvent(QMouseEvent *event)
{
    QPersistentModelIndex current = indexAt(event->pos());
    qint64 pos = currentModel->position(current);
    if (event->button() == Qt::RightButton) { // handling right click

        qint64 start = currentModel->position(currentSelectionModel->startIndex);
        qint64 stop = currentModel->position(currentSelectionModel->endIndex);

        if (pos == ByteItemModel::INVALID_POSITION ||
                (currentSelectionModel->startIndex.isValid()
                && currentSelectionModel->endIndex.isValid()
                && pos >= qMin(start, stop)
                && pos <= qMax(start, stop))) {

            // inside the selection or outside the table, nothing to do
            QTableView::mousePressEvent(event);
            return;

        }

    }
    currentSelectionModel->clear();
    if (current.isValid()) {
        if (pos == ByteItemModel::INVALID_POSITION) {
            current = current.sibling(-1,-1);
            currentSelectionModel->startIndex = current;
            currentSelectionModel->endIndex = current;
            emit newSelection();
        } else {
            if (!(event->modifiers().testFlag(Qt::ShiftModifier) && currentSelectionModel->startIndex.isValid()))
                currentSelectionModel->startIndex = current;
            currentSelectionModel->endIndex = current;
        }
    } else {
        currentSelectionModel->startIndex = current;
        currentSelectionModel->endIndex = current;
        emit newSelection();
    }

    QTableView::mousePressEvent(event);

}

void ByteTableView::mouseMoveEvent(QMouseEvent *event)
{
    QPersistentModelIndex current = indexAt(event->pos());
    // checking if the mouse pointer is in the table, and extending the selection if needed
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
            case Qt::Key_N:
                searchAgain();
                event->accept();
                break;
            default:
                QAbstractItemView::keyPressEvent(event);
        }
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

    } else if (event->key() == Qt::Key_Delete && event->modifiers().testFlag(Qt::NoModifier)) {
        deleteSelectedBytes();
        event->accept();
    } else {
        QAbstractItemView::keyPressEvent(event);
    }
}

void ByteTableView::onSelectionChanged(const QItemSelection & /* Unused */, const QItemSelection & /* Unused */)
{
    viewport()->update();
    emit newSelection();
}

void ByteTableView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    if (hint == QAbstractItemDelegate::SubmitModelCache) {
        hint = QAbstractItemDelegate::EditNextItem;
    }
    QTableView::closeEditor(editor, hint);
}

QModelIndex ByteTableView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    QModelIndex nextIndex;
    if (cursorAction == QAbstractItemView::MoveNext || cursorAction == QAbstractItemView::MoveRight) {
        nextIndex = currentModel->createIndex(getCurrentPos() + 1);
    } else if (cursorAction == QAbstractItemView::MovePrevious || cursorAction == QAbstractItemView::MoveLeft) {
        nextIndex = currentModel->createIndex(getCurrentPos() - 1);
    } else if (cursorAction == QAbstractItemView::MoveDown) {
        nextIndex = currentModel->createIndex(getCurrentPos() + hexColumncount);
    } else if (cursorAction == QAbstractItemView::MoveUp) {
        nextIndex = currentModel->createIndex(getCurrentPos() - hexColumncount);
    } else if (cursorAction == QAbstractItemView::MoveHome) {
        nextIndex = currentModel->createIndex(currentModel->getSource()->getViewOffset(currentModel->getSource()->lowByte()));
    } else if (cursorAction == QAbstractItemView::MoveEnd) {
        nextIndex = currentModel->createIndex(currentModel->getSource()->getViewOffset(currentModel->getSource()->highByte()));
    } else if (cursorAction == QAbstractItemView::MovePageDown) {
        nextIndex = currentModel->createIndex(currentModel->getSource()->getViewOffset(currentModel->getSource()->getRealOffset(getCurrentPos()) + (hexColumncount * 16)));
        if (!nextIndex.isValid()) {
            if (getCurrentPos() % hexColumncount < currentModel->size() % hexColumncount)
                nextIndex = currentModel->createIndex((currentModel->rowCount() - 1), getCurrentPos() % hexColumncount);
            else
                nextIndex = currentModel->createIndex(currentModel->size() - 1);
        }
    } else if (cursorAction == QAbstractItemView::MovePageUp) {
        nextIndex = currentModel->createIndex(currentModel->getSource()->getViewOffset(currentModel->getSource()->getRealOffset(getCurrentPos()) - (hexColumncount * 16)));
        if (!nextIndex.isValid())
            nextIndex = currentModel->createIndex(getCurrentPos() % hexColumncount);
    }

    if (nextIndex.isValid() && currentModel->position(nextIndex) != ByteItemModel::INVALID_POSITION) {
        if (! (modifiers & Qt::ShiftModifier))
            currentSelectionModel->startIndex = nextIndex;

        currentSelectionModel->endIndex = nextIndex;

        currentSelectionModel->clear();
       // currentSelectionModel->setCurrentIndex(nextIndex,QItemSelectionModel::Select);

    }

    return nextIndex;
}

void ByteTableView::wheelEvent(QWheelEvent *event)
{
    int selectionStart = currentModel->position(currentSelectionModel->startIndex);
    int selectionEnd = currentModel->position(currentSelectionModel->endIndex);
    int moved = 0;
    if (verticalScrollBar()->isVisible()) {
        if (verticalScrollBar()->sliderPosition() == 0
                && event->delta() >= 0
                && currentModel->getSource()->tryMoveUp(hexColumncount)) {
            setAttribute(Qt::WA_NoMousePropagation,true);
            moved = hexColumncount;

        } else if (verticalScrollBar()->sliderPosition() == verticalScrollBar()->maximum()
                 && event->delta() < 0
                 && currentModel->getSource()->tryMoveDown(hexColumncount)) {
            setAttribute(Qt::WA_NoMousePropagation,true);
            moved = -1 * hexColumncount;
        }
    } else {
        if (event->delta() >= 0 && currentModel->getSource()->tryMoveUp(hexColumncount)) {
            setAttribute(Qt::WA_NoMousePropagation,true);
            moved = hexColumncount;
        }
        else if (event->delta() < 0 && currentModel->getSource()->tryMoveDown(hexColumncount)) {
            setAttribute(Qt::WA_NoMousePropagation,true);
            moved = -1 * hexColumncount;
        }
    }
    if (moved != 0 && selectionStart != -1 && selectionEnd != -1) {

        currentSelectionModel->startIndex = currentModel->createIndex(selectionStart + moved);
        currentSelectionModel->endIndex = currentModel->createIndex(selectionEnd + moved);
        currentSelectionModel->clear();
        currentSelectionModel->setCurrentIndex(currentSelectionModel->endIndex,QItemSelectionModel::Select);
    }

    QTableView::wheelEvent(event);
    setAttribute(Qt::WA_NoMousePropagation, false);
}

bool ByteTableView::getSelectionInfo(int *pos, int *length)
{
    if (currentSelectionModel->startIndex.isValid() && currentSelectionModel->endIndex.isValid()) {
        qint64 pos1 = currentModel->position(currentSelectionModel->startIndex);
        qint64 pos2 = currentModel->position(currentSelectionModel->endIndex);
        qint64 diff = qAbs(pos1 -pos2);
        if (diff > INT_MAX) {
            QMessageBox::warning(this, tr("Selection too large"), tr("The selection size is above INT_MAX(%1), cannot process the request").arg(INT_MAX),QMessageBox::Ok);
            (*length) = 0;
            return false;
        }
        (*length) = (int)diff + 1;
        if (pos1 <= pos2)
            (*pos) = (int)pos1;
        else
            (*pos) = (int)pos2;

        return true;
    }
    return false;
}

QSize ByteTableView::sizeHint() const
{
    int w = verticalHeader()->width() + 4; // +4 seems to be needed
    for (int i = 0; i < currentModel->columnCount(); i++)
        w += columnWidth(i); // seems to include gridline (on my machine)

    QSize size = QTableView::sizeHint();
    size.setWidth(w);
    return size;
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

qint64 ByteTableView::getSelectedBytesCount()
{
    int count = 0;
    int pos = 0;

    getSelectionInfo(&pos, &count);

    return count;
}

void ByteTableView::deleteSelectedBytes()
{
    ByteSourceAbstract *curSource = static_cast<ByteItemModel *>(currentModel)->getSource();
    if (curSource->hasCapability(ByteSourceAbstract::CAP_RESIZE)  && !curSource->isReadonly()) {
        int pos = 0;
        int length = 0;
        if (getSelectionInfo(&pos, &length)) {
            currentModel->remove(pos, length);
        }
    } else {
        QMessageBox::warning(this, tr("Cannot delete"), tr("The source does not have the resize capability or is readonly"),QMessageBox::Ok);
    }
}

void ByteTableView::replaceSelectedBytes(char byte)
{
    if (!static_cast<ByteItemModel *>(currentModel)->getSource()->isReadonly()) {
        int pos = 0;
        int length = 0;
        if (getSelectionInfo(&pos, &length)) {
            currentModel->replace(pos, length, QByteArray(length, byte));
        }
    } else {
        QMessageBox::warning(this, tr("Cannot replace"), tr("The source is read-only"),QMessageBox::Ok);
    }

}

void ByteTableView::replaceSelectedBytes(QByteArray data)
{
    ByteSourceAbstract * bsource = static_cast<ByteItemModel *>(currentModel)->getSource();
    int capabilities = bsource->getCapabilities();
    if (!bsource->isReadonly()) {
        int pos = 0;
        int length = 0;
        if (getSelectionInfo(&pos, &length)) {
            if (data.size() != length && !(capabilities & ByteSourceAbstract::CAP_RESIZE)) {
                QMessageBox::warning(this, tr("Cannot replace"), tr("The source does not have the resize capability,"
                                                                    " cannot replace with data of a different size than the selection"),QMessageBox::Ok);
                return;
            }
            currentModel->replace(pos, length, data);
            if (data.size() != length ) {
                selectBytes(pos,data.size());
            }
        }
    } else {
        QMessageBox::warning(this, tr("Cannot replace"), tr("The source is read-only"),QMessageBox::Ok);
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
{ // that's not really clean ..
    currentSelectionModel->startIndex = currentModel->createIndex(0);
    currentSelectionModel->endIndex = currentModel->createIndex(currentModel->size() - 1);
    currentSelectionModel->selectAll();
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
        if (currentModel->size() > 0)
            return currentModel->size() - 1;
        else
            return ByteItemModel::INVALID_POSITION;
    }

    if (ref < currentSelectionModel->startIndex )
        ref = currentSelectionModel->startIndex;

    return currentModel->position(ref);
}

int ByteTableView::getCurrentPos() const
{
    return currentModel->position(currentIndex());
}

void ByteTableView::markSelected(const QColor &color, QString text)
{
    if (currentSelectionModel->startIndex.isValid() && currentSelectionModel->endIndex.isValid()) {
        int pos1 = currentModel->position(currentSelectionModel->startIndex);
        int pos2 = currentModel->position(currentSelectionModel->endIndex);

        currentModel->getSource()->viewMark(pos1, pos2, color,QColor(), text); // pos1 and pos2 are guaranted to be valid at this point (i.e. positive integers)
    }
}

void ByteTableView::clearMarkOnSelected()
{
    if (currentSelectionModel->startIndex.isValid() && currentSelectionModel->endIndex.isValid()) {
        int pos1 = currentModel->position(currentSelectionModel->startIndex);
        int pos2 = currentModel->position(currentSelectionModel->endIndex);

        currentModel->getSource()->viewClearMarking(pos1, pos2);// pos1 and pos2 are guaranted to be valid at this point (i.e. positive integers)
    }
}

bool ByteTableView::hasSelection()
{
    return currentSelectionModel->startIndex.isValid() && currentSelectionModel->endIndex.isValid();
}

bool ByteTableView::goTo(quint64 offset, bool absolute, bool negative, bool select)
{
    if (currentModel->getSource()->size() <= 0) { // safeguard
        emit error(tr("Source is empty, nowhere to goto"),LOGID);
        return false;
    }

    int viewOffset = 0;
    if (absolute) { // absolute offset
        viewOffset = currentModel->getSource()->getViewOffset(offset);
        if (viewOffset < 0) {
            emit error(tr("GotoProcessing returned an invalid view position(%1)").arg(viewOffset),LOGID);
            return false;
        }
    } else { // relative offset
        quint64 currentOffset = (quint64)getCurrentPos() + currentModel->getSource()->startingRealOffset();
        if (negative) { // relative negative
            if (offset > currentOffset) {
                emit error(tr("Real offset would underflow, ignoring goto request"),LOGID);
                return false;
            }
            //nothing should go wrong now ...
            viewOffset = currentModel->getSource()->getViewOffset(currentOffset - offset);
        } else { // relative positive

            if (ULONG_LONG_MAX - offset < currentOffset) {
                emit error(tr("Real offset would overflow, ignoring goto request"),LOGID);
                return false;
            }
            quint64 realOffset = currentOffset + offset;
            if (currentModel->getSource()->size() - 1 < realOffset) {
                emit error(tr("Real offset is beyond the source size value, ignoring goto request"),LOGID);
                return false;
            }

            viewOffset = currentModel->getSource()->getViewOffset(realOffset);

        }
    }

    // we should have a valid offset by now

    QPersistentModelIndex nextIndex = currentModel->createIndex(viewOffset);
    QPersistentModelIndex currIndex = currentIndex();
    if (select && currIndex.isValid()) {
        currentSelectionModel->startIndex = currIndex;
    } else {
        currentSelectionModel->startIndex = nextIndex;
    }
    currentSelectionModel->endIndex = nextIndex;

    currentSelectionModel->clear();
    currentSelectionModel->setCurrentIndex(nextIndex,QItemSelectionModel::Select);
    scrollTo(nextIndex);

    return true;
}

void ByteTableView::search(QByteArray item, QBitArray mask)
{

    if (searchObject == NULL || item.isEmpty()) {
        return;
    }

    lastSearch = item;
    lastMask = mask;
    int currentViewPos = getCurrentPos();
    if (currentViewPos < 0)
        currentViewPos = 0;

    quint64 curPos = (quint64)currentViewPos + currentModel->getSource()->startingRealOffset();

    if (lastSearchIndex == curPos)
        curPos++;

    searchObject->setSearchItem(item, mask);
    searchObject->setStartOffset(curPos);
    searchObject->setEndOffset(curPos);
    QtConcurrent::run(searchObject, &SearchAbstract::startSearch);
}

void ByteTableView::searchAgain()
{
    if (!lastSearch.isEmpty())
        search(lastSearch,lastMask);
}

void ByteTableView::setColumnCount(int val)
{
    if (val < ByteTableView::MINCOL || val > ByteTableView::MAXCOL) {
        emit error(tr("invalid column count for ByteTableView, ignoring"),LOGID);
    }
    else {
        hexColumncount = val;
    }
}

void ByteTableView::gotoSearch(quint64 soffset, quint64 eoffset)
{
    //qDebug() << "gotoSearch" << soffset << eoffset;
    lastSearchIndex = soffset;
    // we are assuming that both offset are correct
    int sviewOffset = currentModel->getSource()->getViewOffset(soffset);
    int eViewOffset = currentModel->getSource()->getViewOffset(eoffset);

    QPersistentModelIndex searchStartIndex = currentModel->createIndex(sviewOffset);
    QPersistentModelIndex searchEndIndex = currentModel->createIndex(eViewOffset);

    if (!(searchStartIndex.isValid() && searchEndIndex.isValid())) {
        return;
    }

    currentSelectionModel->startIndex = searchStartIndex;
    currentSelectionModel->endIndex = searchEndIndex;

    currentSelectionModel->clear();
    currentSelectionModel->setCurrentIndex(searchEndIndex,QItemSelectionModel::Select);
    scrollTo(searchStartIndex);
    //qDebug() << "end gotoSearch";
}
