/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "quickviewitem.h"
#include <QMutexLocker>
#include <QTimer>
#include <QDebug>
#include "ui_quickviewitem.h"

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
    format = TEXTFORMAT;
    guiConfig = new(std::nothrow) QuickViewItemConfig(guiHelper, this);
    if (guiConfig == NULL) {
        qFatal("Cannot allocate memory for guiConfig X{");
    }
    ui->setupUi(this);
    connect(ui->removePushButton, SIGNAL(clicked()), this, SLOT(deleteLater()));

    if (!xmlconfig.isEmpty())
        setXmlConf(xmlconfig);
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

    if (format == TEXTFORMAT)
        ui->outputLineEdit->setText(QString::fromUtf8(output));
    else
        ui->outputLineEdit->setText(QString::fromUtf8(output.toHex()));

    LOGLEVEL level = LSTATUS;
    for (int i = 0; i < messages.size() ; i++) {
        toolTipMess.append(messages.at(i).message);
        if (messages.at(i).level > level) {
            level = messages.at(i).level;
        }
    }

    ui->outputLineEdit->setToolTip(toolTipMess);

    switch (level) {
        case (LERROR):
            ui->outputLineEdit->setStyleSheet("QLineEdit { border: 1px solid red } QLineEdit QWidget { color: black;}");
            break;
        case (LWARNING):
            ui->outputLineEdit->setStyleSheet("QLineEdit { border: 1px solid orange } QLineEdit QWidget { color: black;}");
            break;
        case (LSTATUS):
        default:
            ui->outputLineEdit->setStyleSheet("QLineEdit { border: 1px solid gray } QLineEdit QWidget { color: black;}");
    }
}

void QuickViewItem::processData(const QByteArray &data)
{
    currentData = data;
    if (currentTransform != NULL) {
        TransformRequest *tr = new TransformRequest(
                    currentTransform,
                    data,
                    (quintptr) this,
                    false
                                   );

        connect(tr,SIGNAL(finishedProcessing(QByteArray,Messages)), this, SLOT(processingFinished(QByteArray,Messages)));
        guiHelper->processTransform(tr);
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
            QTimer::singleShot(0,this,SLOT(internalProcess()));
            return true;
        }
    }
    return false;
}

void QuickViewItem::mouseDoubleClickEvent(QMouseEvent * event)
{
    if (configure())
        QTimer::singleShot(0,this,SLOT(internalProcess()));

    event->accept();
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
        guiConfig->setFormat(format);
    }
    return true;
}

