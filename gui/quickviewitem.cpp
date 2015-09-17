/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "quickviewitem.h"
#include <QMutexLocker>
#include <QXmlStreamReader>
#include <QTimer>
#include <QMouseEvent>
#include <QClipboard>
#include "loggerwidget.h"
#include <QDebug>
#include "ui_quickviewitem.h"
#include <threadedprocessor.h>
#include <QTextCursor>
#include <QScrollBar>

const QString QuickViewItem::LOGID = "QuickView";

QuickViewItem::QuickViewItem(GuiHelper *nguiHelper, QWidget *parent, const QString &xmlconfig) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::QuickViewItem();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::QuickViewItem X{");
    }
    currentTransform = NULL;
    noError = true;
    guiHelper = nguiHelper;
    outputLine = NULL;
    outputBlock = NULL;
    format = TEXTFORMAT;
    outputType = ONELINE;
    guiConfig = new(std::nothrow) QuickViewItemConfig(guiHelper, this);
    if (guiConfig == NULL) {
        qFatal("Cannot allocate memory for guiConfig X{");
    }
    ui->setupUi(this);
    connect(ui->removePushButton, SIGNAL(clicked()), this, SLOT(deleteLater()));

    if (!xmlconfig.isEmpty())
        setXmlConf(xmlconfig);

    ThreadedProcessor * proc = guiHelper->getCentralTransProc();
    connect(this, SIGNAL(sendRequest(TransformRequest *)), proc, SLOT(processRequest(TransformRequest*)), Qt::QueuedConnection);
    connect(ui->copyPushButton, SIGNAL(clicked()), SLOT(onCopy()));
}

QuickViewItem::~QuickViewItem()
{
    delete currentTransform;
    guiHelper = NULL;
    delete ui;
}

void QuickViewItem::processingFinished(QByteArray output, Messages messages)
{
    toolTipMess.clear();
    QString text;
    QWidget * outputWidget = NULL;
  //  QString stylesheet = QString("%1 { border: 1px solid %2; border-radius: 0px}");

    if (format == TEXTFORMAT)
        text = QString::fromUtf8(output);
    else
        text = QString::fromUtf8(output.toHex());

    if (outputType == ONELINE) {
        if (outputLine == NULL) {
            outputLine = new(std::nothrow) QLineEdit(this);
            outputLine->setMinimumWidth(350);
            outputLine->setReadOnly(true);
            ui->mainLayout->insertWidget(1,outputLine);
        }
        if (outputBlock != NULL) {
            delete outputBlock;
        }

        outputLine->setText(text);

        outputLine->setCursorPosition(0);
        outputWidget = outputLine;
    } else {
        if (outputBlock == NULL) {
            outputBlock = new(std::nothrow) QTextEdit(this);
            outputBlock->setMinimumWidth(350);
            outputBlock->setReadOnly(true);
            ui->mainLayout->insertWidget(1,outputBlock);
        }
        if (outputLine != NULL) {
            delete outputLine;
        }
        outputBlock->setText(text);

        outputBlock->textCursor().setPosition(0);
        outputBlock->setFrameShape(QFrame::NoFrame);
        outputWidget = outputBlock;
    }

    LOGLEVEL level = LSTATUS;
    for (int i = 0; i < messages.size() ; i++) {
        toolTipMess.append(messages.at(i).message);
        if (messages.at(i).level > level) {
            level = messages.at(i).level;
        }
    }

    outputWidget->setToolTip(toolTipMess);
    QString color;
    switch (level) {
        case (LERROR):
            color = "red";
            break;
        case (LWARNING):
            color = "orange";
            break;
        case (LSTATUS):
        default:
            color = "gray";
    }

    if (outputLine == NULL) {
        outputBlock->setStyleSheet(QString("QTextEdit { border: 1px solid %1; border-radius: 0px;}").arg(color));
    } else {
        outputLine->setStyleSheet(QString("QLineEdit { border: 1px solid %1; border-radius: 0px}").arg(color));
    }
}

void QuickViewItem::processData(const QByteArray &data)
{
    currentData = data;
    if (currentTransform != NULL) {
        TransformRequest *tr = new(std::nothrow) TransformRequest(
                    currentTransform,
                    data,
                    (quintptr) this,
                    false
                                   );
        if (tr == NULL) {
            qFatal("Cannot allocate memory for TransformRequest X{");
        }

        connect(tr,SIGNAL(finishedProcessing(QByteArray,Messages)), this, SLOT(processingFinished(QByteArray,Messages)));
        emit sendRequest(tr);
    }
}

bool QuickViewItem::isConfigured()
{
    return currentTransform != NULL;
}

bool QuickViewItem::configure()
{
    guiConfig->setTransform(currentTransform);
    guiConfig->setName(ui->nameLabel->text());
    guiConfig->adjustSize();

    int ret = guiConfig->exec();
    if (ret == QDialog::Accepted) {
        delete currentTransform;
        currentTransform = guiConfig->getTransform();
        if (currentTransform != 0) {
            ui->nameLabel->setText(guiConfig->getName());
            format = guiConfig->getFormat();
            outputType = guiConfig->getOutputType();
            processData(currentData);
            return true;
        }
    }
    return false;
}

void QuickViewItem::mouseDoubleClickEvent(QMouseEvent * event)
{
    if (configure())
        processData(currentData);

    event->accept();
}

void QuickViewItem::onCopy()
{
    QClipboard *clipboard = QApplication::clipboard();
    if (outputType == ONELINE) {
        if (outputLine != NULL) {
            clipboard->setText(outputLine->text());
        }
    } else {
        if (outputBlock != NULL) {
            clipboard->setText(outputBlock->toPlainText());
        }
    }
}

QString QuickViewItem::getXmlConf()
{
    QString ret;
    if (currentTransform != NULL) {
        this->setEnabled(false);
        TransformChain list;
        list.append(currentTransform);
        list.setName(ui->nameLabel->text());
        list.setFormat(format);
        list.setPreferredOutputType(outputType);

        QXmlStreamWriter streamin(&ret);
        guiHelper->getTransformFactory()->saveConfToXML(list, &streamin);
        this->setEnabled(true);
    }

    return ret;
}

bool QuickViewItem::setXmlConf(const QString &conf)
{
    if (conf.isEmpty()) {
        guiHelper->getLogger()->logError(tr("Configuration empty").arg(conf), LOGID);
        return false;
    }

    QXmlStreamReader reader(conf);

    TransformChain talist = guiHelper->getTransformFactory()->loadConfFromXML(&reader);
    if (talist.isEmpty()) {
        guiHelper->getLogger()->logError(tr("Could not load the configuration\n%1").arg(conf), LOGID);
        return false;
    }
    else {
        currentTransform = talist.takeFirst();
        if (talist.size() > 0) {
            guiHelper->getLogger()->logError(tr("Configuration loaded multiple transform, clearing the remaining ones."), LOGID);
            while (!talist.isEmpty()) {
                delete talist.takeLast();
            }
        }

        guiConfig->setTransform(currentTransform);
        ui->nameLabel->setText(talist.getName());
        guiConfig->setName(talist.getName());
        format = talist.getFormat();
        outputType = talist.getPreferredOutputType();
        guiConfig->setFormat(format);
        guiConfig->setOutputType(outputType);
    }
    return true;
}

