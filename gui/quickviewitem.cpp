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
    QWidget(parent),
    ui(new Ui::QuickViewItem)
{
    currentTransform = 0;
    noError = true;
    guiHelper = nguiHelper;
    format = TEXTFORMAT;
    guiConfig = new QuickViewItemConfig(guiHelper, this);
    ui->setupUi(this);
    connect(ui->removePushButton, SIGNAL(clicked()), this, SLOT(deleteLater()));
    connect(this, SIGNAL(updateData(QByteArray)), this, SLOT(renderData(QByteArray)));

    if (!xmlconfig.isEmpty())
        setXmlConf(xmlconfig);
}

QuickViewItem::~QuickViewItem()
{
    if (currentTransform != 0)
        delete currentTransform;

    delete ui;
}

void QuickViewItem::renderData(const QByteArray &displayData)
{
    if (currentTransform != 0) {
        processMessages();
        if (format == TEXTFORMAT)
            ui->outputLineEdit->setText(QString::fromUtf8(displayData));
        else
            ui->outputLineEdit->setText(QString::fromUtf8(displayData.toHex()));
    } else {
        ui->outputLineEdit->setText(tr("No Transform configured"));
    }
}

void QuickViewItem::processData(const QByteArray &data)
{
    dataMutex.lock();
    currentData = data;
    dataMutex.unlock();
    QTimer::singleShot(0,this,SLOT(internalProcess()));
}

bool QuickViewItem::isConfigured()
{
    return currentTransform != 0;
}

void QuickViewItem::internalProcess()
{
    dataMutex.lock();
    if (currentTransform != 0) {
        emit updateData(currentTransform->transform(currentData));
    }
    else {
        currentData.clear();
    }
    dataMutex.unlock();
}

bool QuickViewItem::configure()
{
    guiConfig->setTransform(currentTransform);
    guiConfig->setName(ui->nameLabel->text());
    guiConfig->adjustSize();

    int ret = guiConfig->exec();
    if (ret == QDialog::Accepted) {
        if (currentTransform != 0)
            delete currentTransform;
        currentTransform = guiConfig->getTransform();
        ui->nameLabel->setText(guiConfig->getName());
        if (currentTransform != 0) {
            connect(currentTransform, SIGNAL(error(QString,QString)), this, SLOT(logError(QString,QString)), Qt::UniqueConnection);
            connect(currentTransform, SIGNAL(warning(QString,QString)), this, SLOT(logWarning(QString,QString)), Qt::UniqueConnection);
            format = guiConfig->getFormat();
            QTimer::singleShot(0,this,SLOT(internalProcess()));
            return true;
        }

    }
    return false;
}

void QuickViewItem::processMessages()
{
    messMutex.lock();
    if (toolTipMess.isEmpty())
        ui->outputLineEdit->setStyleSheet("QLineEdit { border: 1px solid gray } QLineEdit QWidget { color: black;}");
    else if (noError)
        ui->outputLineEdit->setStyleSheet("QLineEdit { border: 1px solid orange } QLineEdit QWidget { color: black;}");
    else
        ui->outputLineEdit->setStyleSheet("QLineEdit { border: 1px solid red } QLineEdit QWidget { color: black;}");

    ui->outputLineEdit->setToolTip(toolTipMess);
    toolTipMess.clear();
    noError = true;
    messMutex.unlock();
}

void QuickViewItem::logError(const QString &mess, const QString &)
{
    messMutex.lock();
    if (noError) {
        noError = false;
    }
    if (!toolTipMess.isEmpty())
        toolTipMess.append("\n");
    toolTipMess.append(tr("Error:")).append(mess);
    messMutex.unlock();
}

void QuickViewItem::logWarning(const QString &mess, const QString &)
{
    messMutex.lock();
    if (!toolTipMess.isEmpty())
        toolTipMess.append("\n");
    toolTipMess.append(tr("Warning:")).append(mess);
    messMutex.unlock();
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
    if (currentTransform != 0) {
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
        connect(currentTransform, SIGNAL(error(QString,QString)), this, SLOT(logError(QString,QString)));
        connect(currentTransform, SIGNAL(warning(QString,QString)), this, SLOT(logWarning(QString,QString)));
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

