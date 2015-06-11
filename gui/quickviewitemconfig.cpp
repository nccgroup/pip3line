/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "quickviewitemconfig.h"
#include "ui_quickviewitemconfig.h"
#include <QDebug>

const QString QuickViewItemConfig::LOGID = "QuickViewItemConfig";

QuickViewItemConfig::QuickViewItemConfig(GuiHelper *nguiHelper, QWidget *parent) :
    QDialog(parent)
{
    ui = new(std::nothrow) Ui::QuickViewItemConfig();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::QuickViewItemConfig X{");
    }
    guiHelper = nguiHelper;
    ui->setupUi(this);
    currentTransform = NULL;
    confGui = NULL;
    infoDialog = NULL;
    wayBoxVisible = true;
    formatBoxVisible = true;
    outputTypeVisible = true;
    format = TEXTFORMAT;
    ui->textRadioButton->setChecked(true);
    ui->infoPushButton->setEnabled(false);

    ui->oneLineRadioButton->setChecked(true);

    transformFactory = guiHelper->getTransformFactory();

    guiHelper->buildTransformComboBox(ui->transformComboBox);
    connect(ui->transformComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onTransformSelect(QString)));
    connect(ui->inboundRadioButton, SIGNAL(toggled(bool)), this, SLOT(onInboundWayChange(bool)));
    connect(ui->infoPushButton, SIGNAL(clicked()), this, SLOT(onInfo()));
    connect(ui->textRadioButton, SIGNAL(toggled(bool)), this, SLOT(onTextFormatToggled(bool)));

    ui->wayGroupBox->setVisible(false);
    ui->formatGroupBox->setVisible(false);
    ui->typeGroupBox->setVisible(false);
}

QuickViewItemConfig::~QuickViewItemConfig()
{
    //qDebug() << "Destroying " << this;

    delete currentTransform;

    delete infoDialog;
    guiHelper = NULL;
    delete ui;

}

void QuickViewItemConfig::closeEvent(QCloseEvent *event)
{
    reject();
    QDialog::closeEvent(event);
    deleteLater();
}

TransformAbstract *QuickViewItemConfig::getTransform()
{
    if (currentTransform != NULL)
        return transformFactory->cloneTransform(currentTransform);
    else
        return NULL;
}

void QuickViewItemConfig::setTransform(TransformAbstract *transform)
{
    if (transform != NULL) {
        delete currentTransform;
        currentTransform = transformFactory->cloneTransform(transform);
        if (currentTransform != NULL) {
            int index = ui->transformComboBox->findText(currentTransform->name());
            if (index != -1) {
                ui->transformComboBox->blockSignals(true);
                ui->transformComboBox->setCurrentIndex(index);
                ui->transformComboBox->blockSignals(false);
            } else {
                qWarning() << tr("[QuickViewItemConfig] Could not find the transform index T_T");
            }

            integrateTransform();
        }
    }
}

void QuickViewItemConfig::setName(const QString &name)
{
    ui->nameLineEdit->setText(name);
}

QString QuickViewItemConfig::getName() const
{
    return ui->nameLineEdit->text();
}

OutputFormat QuickViewItemConfig::getFormat() const
{
    return format;
}

void QuickViewItemConfig::setFormat(OutputFormat nformat)
{
    format = nformat;
    if (format == TEXTFORMAT)
        ui->textRadioButton->setChecked(true);
    else
        ui->hexaRadioButton->setChecked(true);
}

void QuickViewItemConfig::setOutputType(OutputType type)
{
    if (type == ONELINE) {
        ui->oneLineRadioButton->setChecked(true);
    } else {
        ui->blockRadioButton->setChecked(true);
    }
}

OutputType QuickViewItemConfig::getOutputType()
{
    return (ui->oneLineRadioButton->isChecked() ? ONELINE : MULTILINES);
}

void QuickViewItemConfig::setWayBoxVisible(bool val)
{
    wayBoxVisible = val;
}

void QuickViewItemConfig::setFormatVisible(bool val)
{
    formatBoxVisible = val;
}

void QuickViewItemConfig::setOutputTypeVisible(bool val)
{
    outputTypeVisible = val;
}

void QuickViewItemConfig::onTransformSelect(QString name)
{
    delete currentTransform;

    currentTransform = transformFactory->getTransform(name);

    if (currentTransform != NULL)
        integrateTransform();

}

void QuickViewItemConfig::onInboundWayChange(bool checked)
{
    if (currentTransform != NULL) {
        if (checked) {
            currentTransform->setWay(TransformAbstract::INBOUND);
        } else {
            currentTransform->setWay(TransformAbstract::OUTBOUND);
        }
    }
}

void QuickViewItemConfig::onTextFormatToggled(bool checked)
{
    if (checked)
        format = TEXTFORMAT;
    else
        format = HEXAFORMAT;
}

void QuickViewItemConfig::onInfo()
{
    if (currentTransform != NULL) {
        if (infoDialog != NULL && infoDialog->getTransform() != currentTransform){
            // if transform changed only
            delete infoDialog;
            infoDialog = NULL;
        }

        infoDialog = new(std::nothrow) InfoDialog(guiHelper, currentTransform,this);
        if (infoDialog == NULL) {
            qFatal("Cannot allocate memory for infoDialog (QuickViewItemConfig) X{");
        }
        else {
            infoDialog->setVisible(true);
        }
    }
}

void QuickViewItemConfig::integrateTransform()
{
    if (currentTransform != NULL) {
        connect(currentTransform, SIGNAL(destroyed()), this, SLOT(onTransformDelete()));
        if (currentTransform->isTwoWays()) {
            ui->inboundRadioButton->setText(currentTransform->inboundString());
            ui->outboundRadioButton->setText(currentTransform->outboundString());
            ui->wayGroupBox->setVisible(wayBoxVisible);
            if (currentTransform->way() == TransformAbstract::INBOUND) {
                ui->inboundRadioButton->setChecked(true);
            } else {
                ui->outboundRadioButton->setChecked(true);
            }
        } else {
            ui->wayGroupBox->setVisible(false);
        }
        ui->formatGroupBox->setVisible(formatBoxVisible);
        ui->typeGroupBox->setVisible(outputTypeVisible);

        if (confGui != NULL)
            ui->confLayout->removeWidget(confGui);
        confGui = currentTransform->getGui(this);
        // never need to delete confgui, as it is taken care of by TransformAbstract upon destruction

        ui->infoPushButton->setEnabled(true);
        ui->nameLineEdit->setText(currentTransform->name());
        if (confGui != NULL) {
            ui->confLayout->addWidget(confGui);
        }
        this->adjustSize();
    }
}

void QuickViewItemConfig::onTransformDelete()
{
    currentTransform = NULL;
    ui->transformComboBox->blockSignals(true);
    ui->transformComboBox->setCurrentIndex(0);
    ui->transformComboBox->blockSignals(false);
    ui->wayGroupBox->setVisible(false);
    ui->formatGroupBox->setVisible(false);
}
