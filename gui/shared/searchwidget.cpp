/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "searchwidget.h"
#include "../sources/bytesourceabstract.h"
#include "../guihelper.h"
#include <QApplication>
#include <QTextCodec>
#include <QTextEncoder>
#include <QPainter>
#include <QLinearGradient>
#include <QHBoxLayout>
#include <QFocusEvent>
#include <QPushButton>
#include <QDebug>
#include <QToolTip>
#include <QEvent>
#include "../sources/searchabstract.h"
#include "searchresultswidget.h"
#include "../floatingdialog.h"
#include "guiconst.h"
using namespace GuiConst;

FoundOffsetsModel::FoundOffsetsModel(QObject *parent) :
    QAbstractListModel(parent)
{
    searchDelegate = NULL;
    ranges = NULL;
}

FoundOffsetsModel::~FoundOffsetsModel()
{

}

void FoundOffsetsModel::setSearchDelegate(SearchAbstract *nsearchObj)
{
    searchDelegate = nsearchObj;
    if (searchDelegate != NULL) {
        connect(searchDelegate, SIGNAL(dataReset()), SLOT(clear()), Qt::UniqueConnection);
    }
}

SearchAbstract *FoundOffsetsModel::getSearchObject() const
{
    return searchDelegate;
}

int FoundOffsetsModel::rowCount(const QModelIndex &) const
{
    return ranges == NULL ? 0 : ranges->size();
}

QVariant FoundOffsetsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && ranges != NULL) {
        if (role == Qt::DisplayRole) {
            return QVariant(QString::number(ranges->at(index.row())->getLowerVal(),16).prepend("0x"));
        }
    }
    return QVariant();
}

QVariant FoundOffsetsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
             return QVariant();

    if (orientation == Qt::Horizontal) {
        if (section == 0)
            return QString("Offset");
    }
    return QVariant();
}

quint64 FoundOffsetsModel::getStartingOffset(const QModelIndex &index)
{
    if (index.isValid() && ranges != NULL) {
        return ranges->at(index.row())->getLowerVal();
    }
    return 0;
}

quint64 FoundOffsetsModel::getStartingOffset(const int index)
{
    if (!(index < 0) &&
            ranges != NULL &&
            index < ranges->size()) {
        return ranges->at(index)->getLowerVal();
    }
    return 0;
}

quint64 FoundOffsetsModel::getEndOffset(const QModelIndex &index)
{
    if (index.isValid() && ranges != NULL) {
        return ranges->at(index.row())->getUpperVal();
    }
    return 0;
}

bool FoundOffsetsModel::lessThanFoundOffset(BytesRange* i1, BytesRange* i2)
{
    return i1->getLowerVal() < i2->getLowerVal();
}

void FoundOffsetsModel::startSearch()
{
    searchTimer.restart();
}

int FoundOffsetsModel::elapsed()
{
    return searchTimer.elapsed();
}

void FoundOffsetsModel::clear()
{
    if (ranges != NULL && !ranges->isEmpty()) {
        disconnect(ranges, SIGNAL(destroyed()), this, SLOT(onRangeDestroyed()));
        beginResetModel();
        while(!ranges->isEmpty())
            delete ranges->takeFirst();
        delete ranges;
        ranges = NULL;
        endResetModel();
        emit updated();
    }
}

void FoundOffsetsModel::setNewList(BytesRangeList *list)
{
    clear();
    beginResetModel();
    ranges = list;
    endResetModel();
    connect(ranges, SIGNAL(destroyed()), SLOT(onRangeDestroyed()));
    emit updated();
}

void FoundOffsetsModel::onRangeDestroyed()
{
    beginResetModel();
    ranges = NULL;
    endResetModel();
    emit updated();
}

BytesRangeList *FoundOffsetsModel::getRanges() const
{
    return ranges;
}

const QColor SearchLine::LOADING_COLOR = QColor(60, 177, 250);
const int SearchLine::MAX_TEXT_SIZE = 2048; // that should be more than enough

SearchLine::SearchLine(ByteSourceAbstract *source, QWidget *parent) :
    QLineEdit(parent)
{
    progress = 0;
    sourceSize = 0;
    searching = false;
    setMaxLength(MAX_TEXT_SIZE);
    setBytesource(source);
    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);

}

SearchLine::~SearchLine()
{
}

void SearchLine::setError(bool val)
{
    if (val)
        setStyleSheet(GuiStyles::LineEditError);
    else
        setStyleSheet(qApp->styleSheet());
    progress = 0;
    update();
}

void SearchLine::updateProgress(double val)
{
    if (searching) { // it means that we are searching ..
        progress = val;
        update();
    } // ignore the update otherwise, it would be an old signal kicking in
}

void SearchLine::onSearchStarted()
{
    searching = true;
    setDisabled(true);
}

void SearchLine::onSearchEnded()
{
    searching = false;
    setDisabled(false);
    progress = 0;
    update();
    setFocus();
}

void SearchLine::onSourceUpdated(quintptr)
{
    ByteSourceAbstract *source = static_cast<ByteSourceAbstract *>(sender());
    if (source != NULL) {
        sourceSize = source->size();
    }
}

void SearchLine::focusInEvent(QFocusEvent *event)
{
    if (event->gotFocus()) {
        selectAll();
    }
}

void SearchLine::paintEvent(QPaintEvent *event)
{
    if (progress > 0 && progress < 1) {
        QPainter p(this);
        QStyleOptionFrameV2 panel;
        initStyleOption(&panel);
        style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);

        QPainter painter(this);
        QStyleOptionFrameV2 lenap;
        initStyleOption(&lenap);
        QRect backgroundRect = style()->subElementRect(QStyle::SE_LineEditContents, &lenap, this);

        int mid = backgroundRect.width() * progress;
        QLinearGradient gradient(10,10,backgroundRect.width(),10);

        gradient.setColorAt(0, LOADING_COLOR);
        gradient.setColorAt(1, QColor::fromRgb(211, 235, 250));
        painter.setBrush(gradient);
        painter.setPen(Qt::transparent);

        QRect progressRect(backgroundRect.x(), backgroundRect.y(), mid, backgroundRect.height());
        painter.drawRect(progressRect);

        painter.setPen(Qt::SolidLine);
        painter.drawText(backgroundRect,Qt::AlignLeft|Qt:: AlignVCenter, " " + this->text());

    } else {
        QLineEdit::paintEvent(event);
    }
}

void SearchLine::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) {
        emit newSearch(text(),event->modifiers());
        event->accept();
        return;
    } else if (event->key() == Qt::Key_N && event->modifiers().testFlag(Qt::ControlModifier))  {
        emit requestJumpToNext();
        return;
    }
    QLineEdit::keyPressEvent(event);
}

ByteSourceAbstract *SearchLine::getBytesource() const
{
    return bytesource;
}

void SearchLine::setBytesource(ByteSourceAbstract * source)
{
    if (source != NULL) {
        bytesource = source;
        connect(bytesource,SIGNAL(updated(quintptr)), SLOT(onSourceUpdated(quintptr)), Qt::QueuedConnection);
        sourceSize = bytesource->size();
    }
}

const QString SearchWidget::FIND_PLACEHOLDER_TEXT = QObject::tr("find (ctrl+f)");
const QString SearchWidget::PLACEHOLDER_DISABLED_TEXT = QObject::tr("Search not implemented for this source :(");
const QString SearchWidget::TOOLTIP_TEXT = QObject::tr("<html><head/><body><p>Default search will be based on the current view:<br/>"
                                                       "Text view (if present) =&gt; normal text search</p>"
                                                       "Hexa view =&gt; search in the hex view from an hexadecimal value</p>"
                                                       "<p>Otherwise one can force the conversion with the following shortcuts:<br/>"
                                                       "shift + enter =&gt; hexadecimal search<br/>"
                                                       "alt + enter =&gt; UTF-8 search<br/>"
                                                       "ctrl + enter =&gt; UTF-16 search<br/>"
                                                       "ctrl + n =&gt; redo last search</p>"
                                                       "<p>When using shortcut the view will switch to hexadecimal</p>"
                                                       "<p>You can use mask value XX in hexadecimal searches</p></body></html>");

SearchWidget::SearchWidget(ByteSourceAbstract * source, GuiHelper *nguiHelper, QWidget *parent) :
    QWidget(parent)
{
    lineEdit = NULL;
    stopPushButton = NULL;
    resultWidget = NULL;
    model = NULL;
    lastJumpStart = 0;
    advancedSearchDialog = NULL;
    bytesource = source;
    guiHelper = nguiHelper;

    searchDelegate = bytesource->getSearchObject();
    if (searchDelegate != NULL) {
        lineEdit = new(std::nothrow) SearchLine(bytesource, this);
        if (lineEdit == NULL) {
            qFatal("Cannot allocate memory for SearchLine X{");
        }
        setFocusProxy(lineEdit);
        QHBoxLayout * layout = new(std::nothrow) QHBoxLayout(this);
        if (layout == NULL) {
            qFatal("Cannot allocate memory for QHBoxLayout X{");
        }
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(0);
        setLayout(layout);
        layout->addWidget(lineEdit);

        advancedPushButton = new(std::nothrow) QPushButton(QIcon(":/Images/icons/edit-find-10.png"), QString(),this);
        if (advancedPushButton == NULL) {
            qFatal("Cannot allocate memory for QPushButton X{");
        }
        advancedPushButton->setFlat(true);
        advancedPushButton->setMaximumWidth(25);
        advancedPushButton->setToolTip("Overall results");
        layout->addWidget(advancedPushButton);

        stopPushButton = new(std::nothrow) QPushButton(QIcon(":/Images/icons/edit-delete-4.png"), QString(),this);
        if (stopPushButton == NULL) {
            qFatal("Cannot allocate memory for QPushButton X{");
        }
        stopPushButton->setFlat(true);
        stopPushButton->setEnabled(false);
        stopPushButton->setMaximumWidth(25);
        layout->addWidget(stopPushButton);

        setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Preferred);

        connect(stopPushButton, SIGNAL(clicked()), SIGNAL(stopSearch()), Qt::QueuedConnection);
        connect(advancedPushButton, SIGNAL(clicked()), SLOT(onAdvanced()));

        connect(lineEdit, SIGNAL(newSearch(QString,int)),SLOT(onSearch(QString,int)));
        connect(lineEdit, SIGNAL(requestJumpToNext()), SLOT(onRequestNext()));
        setSearchDelegate(searchDelegate);
    }
}

SearchWidget::~SearchWidget()
{
    delete model;
    model = NULL;
    resultWidget = NULL;
}

QString SearchWidget::text()
{
    if (searchDelegate != NULL)
        return lineEdit->text();
    else
        return QString();
}

void SearchWidget::setText(QString data)
{
    if (searchDelegate != NULL)
        lineEdit->setText(data);
}

void SearchWidget::setStopVisible(bool val)
{
    if (searchDelegate != NULL)
        stopPushButton->setVisible(val);
}

void SearchWidget::setError(bool val)
{
    if (searchDelegate != NULL)
        lineEdit->setError(val);
}

void SearchWidget::onSearchStarted()
{
    if (searchDelegate != NULL) {
        setCursor(Qt::WaitCursor);
        lineEdit->onSearchStarted();
        stopPushButton->setEnabled(true);
        if (model != NULL)
            model->startSearch();
    }
}

void SearchWidget::onSearchEnded()
{
    if (searchDelegate != NULL) {
        setCursor(Qt::ArrowCursor);
        stopPushButton->setEnabled(false);
        lineEdit->onSearchEnded();
    }
}

void SearchWidget::updateStatusProgress(double val)
{
    if (searchDelegate != NULL)
        lineEdit->updateProgress(val);
}

void SearchWidget::clearSearch()
{
    if (searchDelegate != NULL)
        lineEdit->clear();
}

void SearchWidget::nextFind(quint64 pos)
{
    if (searchDelegate != NULL) {
        BytesRangeList * ranges = model->getRanges();
        if (ranges != NULL && ranges->size() > 0) {
            BytesRange * initial = ranges->at(0);
            for (int i = 0 ; i < ranges->size(); i++) {
                if (pos < ranges->at(i)->getLowerVal()) {
                    processJump(ranges->at(i)->getLowerVal(), ranges->at(i)->getUpperVal());
                    return; // getting out of here
                }
            }
            // nothing found, getting the first one
            processJump(initial->getLowerVal(), initial->getUpperVal());
        }
    }
}

void SearchWidget::onSearch(QString val, int modifiers)
{
    if (searchDelegate != NULL) {
       // qDebug() << "Searching for " << val;
        QByteArray searchItem;
        QBitArray mask;
        lineEdit->setError(false);

        if (!val.isEmpty()) {
            bool couldBeText = false;
            if (modifiers & Qt::ControlModifier) {
                QTextCodec * codec = QTextCodec::codecForName("UTF-16");
                if (codec == NULL) {
                    qCritical() << tr("Could not find the require codec T_T");
                    return;
                } else {
                    QTextEncoder *encoder = codec->makeEncoder(QTextCodec::IgnoreHeader | QTextCodec::ConvertInvalidToNull);
                    searchItem = encoder->fromUnicode(val);
                    if (encoder->hasFailure())
                        lineEdit->setError(true);
                    delete encoder;

                }
            } else if (modifiers & Qt::AltModifier) {
                searchItem = val.toUtf8();
            } else {
                QString HEXCHAR = "abcdef1234567890ABCDEF";
                QChar maskChar = QChar('X');
                mask.resize(val.size()/2);
                int i = 0;
                while (i < val.size() -1 ) { // this is fine, we konw that val.size() > 0
                        if (HEXCHAR.contains(val.at(i)) && HEXCHAR.contains(val.at(i+1))) { // check if valid hexa value
                            mask.setBit(i/2); // this should work with Qt, as the array should be expanded automatically
                            searchItem.append(QByteArray::fromHex(QString(val.at(i)).append(val.at(i+1)).toUtf8()));
                            i += 2;
                            continue;
                        } else if (val.at(i).toUpper() == maskChar && val.at(i+1).toUpper() == maskChar){ // check if this is a mask character
                            searchItem.append('\0');
                            mask.setBit(i/2, false); // idem
                            i += 2;
                            continue;
                        } // otherwise discard the first character
                    i++;
                }
                if (!(modifiers & Qt::ShiftModifier))
                    couldBeText = true;

                // need to "trim" the item based on the mask (don't need start or end masked)
            }

            if (!searchItem.isEmpty()) {
                model->clear();
                emit searchRequest(searchItem, mask, couldBeText);
            }
        }
    }

}

void SearchWidget::onAdvanced()
{
    if (searchDelegate != NULL) {
        if (advancedSearchDialog == NULL) {
            resultWidget = new(std::nothrow) SearchResultsWidget(model, this);
            if (resultWidget == NULL) {
                qFatal("Cannot allocate memory for SearchResultsWidget X{");
            }
            connect(resultWidget, SIGNAL(jumpTo(quint64,quint64)), SLOT(processJump(quint64,quint64)));
            advancedSearchDialog = new(std::nothrow) FloatingDialog(guiHelper,resultWidget ,this);
            if (advancedSearchDialog == NULL) {
                qFatal("Cannot allocate memory for FloatingDialog X{");
            }
            advancedSearchDialog->setWindowTitle("Advanced Search");
            advancedSearchDialog->resize(250,600);
        }
        advancedSearchDialog->raise();
        advancedSearchDialog->show();
    }
}

void SearchWidget::processJump(quint64 start, quint64 end)
{
    lastJumpStart = start;
    emit jumpTo(start, end);
}

void SearchWidget::onRequestNext()
{
    nextFind(lastJumpStart);
}

SearchAbstract *SearchWidget::getSearchDelegate() const
{
    return searchDelegate;
}

void SearchWidget::setSearchDelegate(SearchAbstract * delegate)
{
    if (delegate != NULL) {
        searchDelegate = delegate;
        connect(searchDelegate, SIGNAL(searchStarted()), SLOT(onSearchStarted()), Qt::QueuedConnection);
        connect(searchDelegate, SIGNAL(searchEnded()), SLOT(onSearchEnded()), Qt::QueuedConnection);
        if (model == NULL) {
            model  =  new(std::nothrow)FoundOffsetsModel(this);
            if (model == NULL) {
                qFatal("Cannot allocate memory for FoundOffsetsModel X{");
            }
            if (searchDelegate != NULL) {
                model->setSearchDelegate(searchDelegate);
            }
            connect(searchDelegate, SIGNAL(foundList(BytesRangeList*)), model, SLOT(setNewList(BytesRangeList*)));
        }
        lineEdit->setPlaceholderText(FIND_PLACEHOLDER_TEXT);
        lineEdit->setToolTip(TOOLTIP_TEXT);
        connect(searchDelegate, SIGNAL(errorStatus(bool)), SLOT(setError(bool)),Qt::QueuedConnection);
        connect(searchDelegate, SIGNAL(progressStatus(double)),lineEdit, SLOT(updateProgress(double)),Qt::QueuedConnection);
        connect(stopPushButton, SIGNAL(clicked()), searchDelegate, SLOT(stopSearch()), Qt::DirectConnection);

        setVisible(true);
    } else {
        lineEdit->setPlaceholderText(PLACEHOLDER_DISABLED_TEXT);
        lineEdit->setEnabled(false);
        setVisible(false);
    }
}

ByteSourceAbstract *SearchWidget::getBytesource() const
{
    return bytesource;
}

FoundOffsetsModel *SearchWidget::getModel() const
{
    return model;
}



