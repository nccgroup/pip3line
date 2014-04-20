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

const QColor SearchLine::LOADING_COLOR = QColor(60, 177, 250);
const QString SearchLine::FIND_PLACEHOLDER_TEXT = QObject::tr("find (ctrl+f)");
const QString SearchLine::PLACEHOLDER_DISABLED_TEXT = QObject::tr("Search not implemented for this source :(");
const QString SearchLine::TOOLTIP_TEXT = QObject::tr("<html><head/><body><p>Default search will be based on the current view:<br/>"
                                                       "Text view (if present) =&gt; normal text search</p>"
                                                       "Hexa view =&gt; search in the hex view from an hexadecimal value</p>"
                                                       "<p>Otherwise one can force the conversion with the following shortcuts:<br/>"
                                                       "shift + enter =&gt; hexadecimal search<br/>"
                                                       "alt + enter =&gt; UTF-8 search<br/>"
                                                       "ctrl + enter =&gt; UTF-16 search<br/>"
                                                       "ctrl + n =&gt; redo last search</p>"
                                                       "<p>When using shortcut the view will switch to hexadecimal</p></body></html>");

SearchLine::SearchLine(ByteSourceAbstract *source, QWidget *parent) :
    QLineEdit(parent)
{
    progress = 0;
    sourceSize = 0;
    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
    sObject = source->getSearchObject();
    if (sObject == NULL) {
        setPlaceholderText(PLACEHOLDER_DISABLED_TEXT);
        setEnabled(false);
    } else {
        setPlaceholderText(FIND_PLACEHOLDER_TEXT);
        setToolTip(TOOLTIP_TEXT);
        sObject->setProcessStatsInternally(true);
        connect(sObject, SIGNAL(errorStatus(bool)), SLOT(setError(bool)),Qt::QueuedConnection);
        connect(sObject, SIGNAL(progressStatus(double)), SLOT(updateProgress(double)),Qt::QueuedConnection);
        connect(source,SIGNAL(updated(quintptr)), SLOT(onSourceUpdated(quintptr)), Qt::QueuedConnection);
        sourceSize = source->size();
    }
}

SearchLine::~SearchLine()
{
    sObject = NULL;
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
    if (!isEnabled()) { // it means that we are searching ..
        progress = val;
        update();
    } // ignore the update otherwise, it should be an old signal kicking in
}

void SearchLine::onSearchStarted()
{
   setDisabled(true);
}

void SearchLine::onSearchEnded()
{
    setDisabled(false);
    progress = 0;
    update();
    setFocus();
}

void SearchLine::onSourceUpdated(quintptr)
{
    ByteSourceAbstract *source = static_cast<ByteSourceAbstract *>(sender());
    sourceSize = source->size();
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
    }
    QLineEdit::keyPressEvent(event);
}

SearchWidget::SearchWidget(ByteSourceAbstract * source, QWidget *parent) :
    QWidget(parent)
{
    lineEdit = new(std::nothrow) SearchLine(source, this);
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

    stopPushButton = new(std::nothrow) QPushButton(QIcon(":/Images/icons/edit-delete-4.png"), QString(),this);
    if (stopPushButton == NULL) {
        qFatal("Cannot allocate memory for QPushButton X{");
    }
    stopPushButton->setFlat(true);
    stopPushButton->setEnabled(false);
    stopPushButton->setMaximumWidth(25);
    layout->addWidget(stopPushButton);

    setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Preferred);

    SearchAbstract *sObject = source->getSearchObject();
    if (sObject != NULL) {
        connect(stopPushButton, SIGNAL(clicked()), sObject, SLOT(stopSearch()), Qt::DirectConnection);
        connect(stopPushButton, SIGNAL(clicked()), SIGNAL(stopSearch()), Qt::QueuedConnection);
        connect(sObject, SIGNAL(searchStarted()), SLOT(onSearchStarted()), Qt::QueuedConnection);
        connect(sObject, SIGNAL(searchEnded()), SLOT(onSearchEnded()), Qt::QueuedConnection);
    }

    connect(lineEdit, SIGNAL(newSearch(QString,int)),SLOT(onSearch(QString,int)));
}

SearchWidget::~SearchWidget()
{

}

QString SearchWidget::text()
{
    return lineEdit->text();
}

void SearchWidget::setStopVisible(bool val)
{
    stopPushButton->setVisible(val);
}

void SearchWidget::setError(bool val)
{
    lineEdit->setError(val);
}

void SearchWidget::onSearchStarted()
{
    setCursor(Qt::WaitCursor);
    lineEdit->onSearchStarted();
    stopPushButton->setEnabled(true);
}

void SearchWidget::onSearchEnded()
{
    setCursor(Qt::ArrowCursor);
    stopPushButton->setEnabled(false);
    lineEdit->onSearchEnded();
}

void SearchWidget::updateStatusProgress(double val)
{
    lineEdit->updateProgress(val);
}

void SearchWidget::clearSearch()
{
    lineEdit->clear();
}

void SearchWidget::onSearch(QString val, int modifiers)
{
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
            emit searchRequest(searchItem, mask, couldBeText);
        }
    }

}


