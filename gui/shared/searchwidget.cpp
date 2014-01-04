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


SearchLine::SearchLine(ByteSourceAbstract *source, QWidget *parent) :
    QLineEdit(parent)
{
    progress = 0;
    sourceSize = 0;
    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
    SearchAbstract *sObject = source->getSearchObject();
    if (sObject == NULL) {
        setPlaceholderText(tr("Search"));
        setToolTip(tr("<html><head/><body><p>Search not implemented for this source</p></body></html>"));
        setEnabled(false);
    } else {
        setPlaceholderText(tr("find (ctrl+f)"));
        setToolTip(tr("<html><head/><body><p>Default search will be based on the current view:<br/>"
                      "Text view (if present) =&gt; normal text search</p>"
                      "Hexa view =&gt; search in the hex view from an hexadecimal value</p>"
                      "<p>Otherwise one can force the conversion with the following shortcuts:<br/>"
                      "shift + enter =&gt; hexadecimal search<br/>"
                      "alt + enter =&gt; UTF-8 search<br/>"
                      "ctrl + enter =&gt; UTF-16 search<br/>"
                      "ctrl + n =&gt; redo last search</p>"
                      "<p>When using shortcut the view will switch to hexadecimal</p></body></html>"));
        connect(sObject, SIGNAL(errorStatus(bool)), SLOT(setError(bool)),Qt::QueuedConnection);
        connect(sObject, SIGNAL(progressUpdate(quint64)),sObject, SLOT(processStats(quint64)),Qt::DirectConnection);
        connect(sObject, SIGNAL(progressStatus(double)), SLOT(updateProgress(double)),Qt::QueuedConnection);
        connect(source,SIGNAL(updated(quintptr)), SLOT(onSourceUpdated(quintptr)), Qt::QueuedConnection);
        sourceSize = source->size();
    }
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
    repaint();
}

void SearchLine::updateProgress(double val)
{
    progress = val;

    repaint();
}

void SearchLine::onSearchStarted()
{
    setEnabled(false);
}

void SearchLine::onSearchEnded()
{
    setEnabled(true);
    progress = 0;
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

        QColor loadingColor = QColor(60, 177, 250);

        int mid = backgroundRect.width() * progress;
        QLinearGradient gradient(10,10,backgroundRect.width(),10);

        gradient.setColorAt(0, loadingColor);
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
        connect(stopPushButton, SIGNAL(clicked()), SLOT(onStopClicked()), Qt::QueuedConnection);
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

void SearchWidget::updateProgress(quint64 val)
{
    lineEdit->updateProgress(val);
}

void SearchWidget::clearSearch()
{
    lineEdit->clear();
}

void SearchWidget::onStopClicked()
{
   // qDebug() << "stopped clicked";
    emit stopSearch();
}

void SearchWidget::onSearch(QString val, int modifiers)
{
   // qDebug() << "Searching for " << val;
    QByteArray searchItem;
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
            searchItem = QByteArray::fromHex(val.toUtf8());
            if (!(modifiers & Qt::ShiftModifier))
                couldBeText = true;
        }

        if (!searchItem.isEmpty()) {
            emit searchRequest(searchItem,couldBeText);
        }
    }

}

