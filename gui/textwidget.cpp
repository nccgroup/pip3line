/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "textwidget.h"
#include "ui_textwidget.h"
#include <QFont>
#include <QMimeData>
#include <QScrollBar>
#include <QDebug>

int TextWidget::MAX_TEXT_VIEW = 100000;

TextWidget::TextWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TextWidget)
{
    currentModel = 0;

    ui->setupUi(this);

    qDebug() << "Created" << this;
    renderThread = new RenderTextView(this);
    connect(renderThread, SIGNAL(startingRendering()), this, SLOT(textRenderingStarted()),Qt::QueuedConnection);
    connect(renderThread, SIGNAL(finishedRendering()), this, SLOT(textRenderingFinished()),Qt::QueuedConnection);
    connect(renderThread, SIGNAL(dataChunk(QString)), this, SLOT(reveceivingTextChunk(QString)),Qt::QueuedConnection);
    renderThread->start();

    ui->plainTextEdit->installEventFilter(this);
    connect(ui->plainTextEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    connect(ui->plainTextEdit,SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
    ui->plainTextEdit->setContextMenuPolicy(Qt::NoContextMenu);
    setAcceptDrops(true);
    QFont textViewFont;
    textViewFont.setFamily("Monospace");
    textViewFont.setPointSize(10);
    ui->plainTextEdit->setFont(textViewFont);
}

TextWidget::~TextWidget()
{
    renderThread->stop();
    renderThread->wait();
    delete renderThread;
    delete ui;
}

void TextWidget::setModel(ByteItemModel *dataModel)
{
    currentModel = dataModel;
    connect(currentModel,SIGNAL(updatedFrom(ByteItemModel::UpdateSource)), this, SLOT(updateText(ByteItemModel::UpdateSource)), Qt::UniqueConnection);
}

QByteArray TextWidget::getSelection()
{
    QByteArray ret;
    QString text = ui->plainTextEdit->toPlainText();
    QTextCursor cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection()){
        ret = text.mid(cursor.selectionStart(), cursor.selectionEnd() - cursor.selectionStart()).toUtf8();
    }
    return ret;
}

bool TextWidget::hasSelection()
{
    return ui->plainTextEdit->textCursor().hasSelection();
}

void TextWidget::selectAll()
{
    QTextCursor c = ui->plainTextEdit->textCursor();
    c.select(QTextCursor::Document);
    ui->plainTextEdit->setTextCursor(c);
}

void TextWidget::reset()
{
    ui->plainTextEdit->clear();
    ui->plainTextEdit->setEnabled(true);
}

void TextWidget::dropEvent(QDropEvent *event)
{
    qDebug() << "Text drop Event";
    emit sendDropEvent(event);
}

void TextWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasHtml() || event->mimeData()->hasText() || event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void TextWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Z:
            if (event->modifiers().testFlag(Qt::ShiftModifier) && event->modifiers().testFlag(Qt::ControlModifier)) {
                currentModel->historyForward();
                event->accept();
            }
            else if (event->modifiers().testFlag(Qt::ControlModifier)) {
                currentModel->historyBackward();
                event->accept();
            }
            break;

        default:
            QWidget::keyPressEvent(event);
    }
}

void TextWidget::onTextChanged()
{
    currentModel->setRawData(ui->plainTextEdit->toPlainText().toUtf8(),ByteItemModel::TEXTVIEW);
    updateStats();
}

void TextWidget::onSelectionChanged()
{
    updateStats();
    emit textSelectionChanged();
}

void TextWidget::updateText(ByteItemModel::UpdateSource source)
{
    if (source == ByteItemModel::TEXTVIEW)
        return;
    ui->plainTextEdit->blockSignals(true);
    ui->plainTextEdit->clear();
    if (currentModel->isStringValid()) {
        if (currentModel->size() > MAX_TEXT_VIEW) {
            ui->plainTextEdit->appendPlainText("Data Too large for this view");
            ui->plainTextEdit->blockSignals(false);
            ui->plainTextEdit->setEnabled(false);
            ui->statsLabel->setText(QString(""));
            emit invalidText();
        } else {
            ui->plainTextEdit->blockSignals(false);
            renderThread->setDataForRendering(QString::fromUtf8(currentModel->getRawData()));
        }
    } else {
        ui->plainTextEdit->appendPlainText("INVALID");
        ui->plainTextEdit->blockSignals(false);
        ui->plainTextEdit->setEnabled(false);
        ui->statsLabel->setText(QString(""));
        emit invalidText();
    }
}

void TextWidget::textRenderingStarted()
{
    ui->plainTextEdit->setEnabled(false);
    ui->plainTextEdit->blockSignals(true);
    ui->plainTextEdit->clear();
}

void TextWidget::textRenderingFinished()
{
    ui->plainTextEdit->moveCursor(QTextCursor::Start);
    ui->plainTextEdit->ensureCursorVisible();
    ui->plainTextEdit->blockSignals(false);
    ui->plainTextEdit->setEnabled(true);
    updateStats();
}

void TextWidget::reveceivingTextChunk(const QString &chunk)
{
    ui->plainTextEdit->insertPlainText(chunk);
}

void TextWidget::updateStats()
{
    if (ui->plainTextEdit->isEnabled()) {
        QString plainText = ui->plainTextEdit->toPlainText();
        QTextCursor cursor = ui->plainTextEdit->textCursor();
        int size = 0;
        if (cursor.hasSelection()){
            size = cursor.selectionEnd() - cursor.selectionStart();
        }
        // updating text info
        QString ret = "Size: ";
        ret.append(QString::number(plainText.size())).append(tr(" characters"));
        if (size > 0)
            ret.append(tr(" (%1 selected) |").arg(size));
        ret.append(tr(" Lines: ")).append(QString::number(plainText.count("\n") + 1));

        ui->statsLabel->setText(ret);
    }
}


bool TextWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->plainTextEdit) {
        ui->plainTextEdit->setAttribute(Qt::WA_NoMousePropagation, false);
        if (event->type() == QEvent::KeyPress) {
             QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
             switch (keyEvent->key())
             {
                 case Qt::Key_Z:
                     if (keyEvent->modifiers().testFlag(Qt::ShiftModifier) && keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
                         currentModel->historyForward();
                         keyEvent->accept();
                     }
                     else if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
                         currentModel->historyBackward();
                         keyEvent->accept();
                     }
                    break;
                 default:
                     return false;
             }
             return true;
        } else if (event->type() == QEvent::Wheel) {
            if (ui->plainTextEdit->verticalScrollBar()->isVisible()) {
                ui->plainTextEdit->setAttribute(Qt::WA_NoMousePropagation);
            }

            return false;
        }
    }

    return QWidget::eventFilter(obj, event);

}
