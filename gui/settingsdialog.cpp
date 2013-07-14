/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <transformfactoryplugininterface.h>
#include "pluginconfwidget.h"
#include <QHashIterator>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QListWidgetItem>
#include <commonstrings.h>
#include <deleteablelistitem.h>
#include "quickviewitemconfig.h"
#include <QDebug>
using namespace Pip3lineConst;

const QString SettingsDialog::LOGID = "SettingsDialog";

SettingsDialog::SettingsDialog(GuiHelper *nhelper, QWidget *parent) :
    QDialog(parent)
{
    ui = new(std::nothrow) Ui::SettingsDialog();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::SettingsDialog X{");
    }
    helper = nhelper;
    tManager = helper->getTransformFactory();
    settings = tManager->getSettingsObj();
    ui->setupUi(this);

    ui->portSpinBox->setValue(helper->getDefaultPort());
    ui->decodeCheckBox->setChecked(helper->getDefaultServerDecode());
    ui->encodeCheckBox->setChecked(helper->getDefaultServerEncode());
    ui->pipeNameLineEdit->setText(helper->getDefaultServerPipeName());
    ui->autoUpdateCheckBox->setChecked(settings->value(SETTINGS_AUTO_UPDATE, true).toBool());
    ui->minimizeCheckBox->setChecked(settings->value(SETTINGS_MINIMIZE_TO_TRAY, true).toBool());
    ui->hexWidget->setChar(helper->getDefaultServerSeparator());

    connect(ui->autoUpdateCheckBox, SIGNAL(toggled(bool)), this, SLOT(autoUpdateChanged(bool)));
    connect(ui->minimizeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onMinimizeChanged(bool)));
    connect(ui->checkUpdatePushButton, SIGNAL(clicked()), this, SLOT(onUpdateRequest()));
    setModal(false);

    updatePluginList();
    updateRegisteredList();
    updateSavedMarkingColors();
    updateImportExportFuncs();

    connect(tManager, SIGNAL(savedUpdated()), this, SLOT(updateRegisteredList()));
    connect(helper, SIGNAL(markingsUpdated()), this, SLOT(updateSavedMarkingColors()));
    connect(ui->resetMarkingsPushButton, SIGNAL(clicked()), this, SLOT(onResetMarkings()));
    connect(helper, SIGNAL(importExportUpdated()), this, SLOT(updateImportExportFuncs()));
    connect(ui->resetIEFuncsPushButton, SIGNAL(clicked()), this, SLOT(onResetImportExportFuncs()));
    connect(ui->savedListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onSavedClicked(QListWidgetItem*)));
    connect(ui->pluginsListWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(onPluginClicked(QModelIndex)));
    connect(ui->decodeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onServerDecodeChanged(bool)));
    connect(ui->encodeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onServerEncodeChanged(bool)));
    connect(ui->portSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onServerPortChanged(int)));
    connect(ui->pipeNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onServerPipeNameChanged(QString)));
    connect(ui->hexWidget, SIGNAL(charChanged(char)), this, SLOT(onServerSeparatorChanged(char)));
    connect(ui->importExportListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onDoubleClickImportExportFuncs(QListWidgetItem*)));
    connect(ui->addImportExportPushButton, SIGNAL(clicked()), this, SLOT(onAddImportExportFuncs()));
    connect(ui->offsetBaseComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onOffsetBaseChanged(QString)));
}

SettingsDialog::~SettingsDialog()
{
    delete settings;
    delete ui;
}

void SettingsDialog::setVersionUpdateMessage(QString mess)
{
    ui->versionCheckedLabel->setText(mess);
}

void SettingsDialog::autoUpdateChanged(bool checked)
{
    settings->setValue(SETTINGS_AUTO_UPDATE, checked);
}


void SettingsDialog::onMinimizeChanged(bool checked)
{
    settings->setValue(SETTINGS_MINIMIZE_TO_TRAY, checked);
}

void SettingsDialog::onUpdateRequest()
{
    emit updateCheckRequested();
}

void SettingsDialog::updatePluginList()
{
    ui->pluginsListWidget->clear();
    stackedList.clear();
    int count = ui->pluginsStackedWidget->count();
    for (int i = 0 ; i < count; i++) {
      QWidget * wid = ui->pluginsStackedWidget->widget(i);
      ui->pluginsStackedWidget->removeWidget(wid);
      delete wid;
    }

    QHash<QString, TransformFactoryPluginInterface *> pluginList = tManager->getPlugins();

    QHashIterator<QString, TransformFactoryPluginInterface *> i(pluginList);
    while (i.hasNext()) {
        i.next();
        PluginConfWidget *widget = new(std::nothrow) PluginConfWidget(i.value());
        if (widget != NULL) {
            stackedList.insert(i.key(), ui->pluginsStackedWidget->addWidget(widget));
            ui->pluginsListWidget->addItem(i.key());
        } else {
            qFatal("Cannot allocate memory for PluginConfWidget X{");
        }
    }
    ui->pluginsListWidget->sortItems();
}

void SettingsDialog::updateRegisteredList()
{
    ui->savedListWidget->clear();
    QStringList list = tManager->getSavedConfs().keys();

    for (int i = 0; i < list.size(); i++) {
        DeleteableListItem *itemWid = new(std::nothrow) DeleteableListItem(list.at(i));
        if (itemWid != NULL) {
            connect(itemWid, SIGNAL(itemDeleted(QString)), this, SLOT(onDeleteSaved(QString)));
            QListWidgetItem *item = new(std::nothrow) QListWidgetItem();
            if (item != NULL) {
                ui->savedListWidget->addItem(item);
                ui->savedListWidget->setItemWidget(item, itemWid);
            } else {
                qFatal("Cannot allocate memory for QListWidgetItem registered X{");
            }
        } else {
            qFatal("Cannot allocate memory for DeleteableListItem registered X{");
        }
    }
}

void SettingsDialog::updateSavedMarkingColors()
{
    ui->markingColorsListWidget->clear();
    QHash<QString, QColor> colors = helper->getMarkingsColor();
    QHashIterator<QString, QColor> i(colors);
    while (i.hasNext()) {
        i.next();
        QPixmap pix(20,20);
        pix.fill(i.value());
        DeleteableListItem *itemWid = new(std::nothrow) DeleteableListItem(i.key(), pix);
        if (itemWid != NULL) {
            connect(itemWid, SIGNAL(itemDeleted(QString)), this, SLOT(onMarkingDelete(QString)));
            QListWidgetItem *item = new(std::nothrow) QListWidgetItem();
            if (item != NULL) {
                ui->markingColorsListWidget->addItem(item);
                ui->markingColorsListWidget->setItemWidget(item, itemWid);
            } else {
                qFatal("Cannot allocate memory for QListWidgetItem Colors X{");
            }
        } else {
            qFatal("Cannot allocate memory for DeleteableListItem Colors X{");
        }
    }
}

void SettingsDialog::updateImportExportFuncs()
{
    ui->importExportListWidget->clear();
    QStringList list = helper->getImportExportFunctions();
    for (int i = 0; i < list.size(); i++) {
        DeleteableListItem *itemWid = new(std::nothrow) DeleteableListItem(list.at(i));
        if (itemWid != NULL) {
            connect(itemWid, SIGNAL(itemDeleted(QString)), this, SLOT(onImportExportFuncDeletes(QString)));
            QListWidgetItem *item = new(std::nothrow) QListWidgetItem();
            if (item != NULL) {
                ui->importExportListWidget->addItem(item);
                ui->importExportListWidget->setItemWidget(item, itemWid);
            } else {
                qFatal("Cannot allocate memory for QListWidgetItem Import/Export X{");
            }
        } else {
            qFatal("Cannot allocate memory for DeleteableListItem Import/Export X{");
        }
    }
}

void SettingsDialog::updateMisc()
{
    int offsetbase = helper->getDefaultOffsetBase();
    switch (offsetbase) {
        case 8:
            ui->offsetBaseComboBox->setCurrentIndex(0);
            break;
        case 10:
            ui->offsetBaseComboBox->setCurrentIndex(1);
            break;
        case 16:
            ui->offsetBaseComboBox->setCurrentIndex(2);
            break;
        default: // this should obviously not happen ...
            helper->setDefaultOffsetBase(16);
            ui->offsetBaseComboBox->setCurrentIndex(2);
    }
}

void SettingsDialog::onImportExportFuncDeletes(const QString &name)
{
    helper->removeImportExportFunctions(name);
}

void SettingsDialog::onResetImportExportFuncs()
{
    helper->resetImportExportFuncs();
}

void SettingsDialog::onDoubleClickImportExportFuncs(QListWidgetItem *item)
{
    DeleteableListItem *itemWid = (DeleteableListItem *)ui->importExportListWidget->itemWidget(item);

    QString name = itemWid->getName();

    QuickViewItemConfig *itemConfig = new(std::nothrow) QuickViewItemConfig(helper, this);
    if (itemConfig != NULL) {
        itemConfig->setWayBoxVisible(false);
        itemConfig->setFormatVisible(false);
        TransformAbstract *ta = helper->getImportExportFunction(name);

        if (ta != NULL) {

            ta = tManager->loadTransformFromConf(ta->getConfiguration());
            if (ta != NULL) {
                itemConfig->setTransform(ta);
                itemConfig->setName(name);
                int ret = itemConfig->exec();
                if (ret == QDialog::Accepted) {
                    delete ta;
                    ta = itemConfig->getTransform();
                    QString newName = itemConfig->getName();

                    helper->removeImportExportFunctions(name);
                    helper->addImportExportFunctions(newName, ta);
                } else {
                    delete ta;
                }

            }
        }
        delete itemConfig;
    } else {
        qFatal("Cannot allocate memory for QuickViewItemConfig Import/Export double X{");
    }
}

void SettingsDialog::onAddImportExportFuncs()
{
    QuickViewItemConfig *itemConfig = new(std::nothrow) QuickViewItemConfig(helper, this);
    if (itemConfig != NULL) {
        itemConfig->setWayBoxVisible(false);
        itemConfig->setFormatVisible(false);
        int ret = itemConfig->exec();
        if (ret == QDialog::Accepted) {
            TransformAbstract *ta = itemConfig->getTransform();
            if (ta != NULL) {
                helper->addImportExportFunctions(itemConfig->getName(),ta);
            }
        }

        delete itemConfig;
    } else {
        qFatal("Cannot allocate memory for QuickViewItemConfig Import/Export add X{");
    }
}

void SettingsDialog::onPluginClicked(QModelIndex index)
{
    QString name = index.data().toString();

    if (stackedList.contains(name)) {
        ui->pluginsStackedWidget->setCurrentIndex(stackedList.value(name));
    } else {
        qWarning() << tr("[SettingsDialog] Cannot find the item %1 in stackedList T_T").arg(name);
    }
}


void SettingsDialog::onSavedClicked(QListWidgetItem * item)
{
    DeleteableListItem *itemWid = (DeleteableListItem *)ui->savedListWidget->itemWidget(item);

    QString name = itemWid->getName();

    QHash<QString, QString> list = tManager->getSavedConfs();
    if (list.contains(name))
        ui->savedDescriptLabel->setText(tManager->getSavedConfs().value(name));
    else
        helper->getLogger()->logError(tr("Saved conf \"%1\" not found").arg(name),LOGID);
}

void SettingsDialog::onDeleteSaved(const QString &name)
{
    tManager->unregisterChainConf(name);
    ui->savedDescriptLabel->clear();
}

void SettingsDialog::onMarkingDelete(const QString &name)
{
    helper->removeMarkingColor(name);
}

void SettingsDialog::onResetMarkings()
{
    helper->resetMarkings();
}

void SettingsDialog::onServerPortChanged(int port)
{
    helper->setDefaultServerPort(port);
}

void SettingsDialog::onServerDecodeChanged(bool val)
{
    helper->setDefaultServerDecode(val);
}

void SettingsDialog::onServerEncodeChanged(bool val)
{
    helper->setDefaultServerEncode(val);
}

void SettingsDialog::onServerSeparatorChanged(char c)
{
    helper->setDefaultServerSeparator(c);
}

void SettingsDialog::onServerPipeNameChanged(QString name)
{
    helper->setDefaultServerPipeName(name);
}

void SettingsDialog::onOffsetBaseChanged(QString val)
{
    bool ok = false;
    int intval = val.toInt(&ok);

    if (!ok) {
        helper->getLogger()->logError("Invalid integer value for offset base in settings T_T");
    } else {
        helper->setDefaultOffsetBase(intval);
    }
}

