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
#include <transformmgmt.h>
#include <QListWidgetItem>
#include "guihelper.h"
#include <QSettings>
#include <QStandardItemModel>
#include <QDebug>
using namespace Pip3lineConst;

const QString SettingsDialog::LOGID = "SettingsDialog";

SettingsDialog::SettingsDialog(GuiHelper *nhelper, QWidget *parent) :
    AppDialog(nhelper, parent)
{
    ui = new(std::nothrow) Ui::SettingsDialog();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::SettingsDialog X{");
    }

    tManager = guiHelper->getTransformFactory();
    settings = tManager->getSettingsObj();
    ui->setupUi(this);

    ui->portSpinBox->setValue(guiHelper->getDefaultPort());
    ui->decodeCheckBox->setChecked(guiHelper->getDefaultServerDecode());
    ui->encodeCheckBox->setChecked(guiHelper->getDefaultServerEncode());
    ui->pipeNameLineEdit->setText(guiHelper->getDefaultServerPipeName());
    ui->autoUpdateCheckBox->setChecked(settings->value(SETTINGS_AUTO_UPDATE, true).toBool());
    ui->minimizeCheckBox->setChecked(settings->value(SETTINGS_MINIMIZE_TO_TRAY, true).toBool());
    ui->hexWidget->setChar(guiHelper->getDefaultServerSeparator());

    connect(ui->autoUpdateCheckBox, SIGNAL(toggled(bool)), this, SLOT(autoUpdateChanged(bool)));
    connect(ui->minimizeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onMinimizeChanged(bool)));
    connect(ui->checkUpdatePushButton, SIGNAL(clicked()), this, SLOT(onUpdateRequest()));
    setModal(false);

    updatePluginList();
    updateRegisteredList();
    updateSavedMarkingColors();
    updateImportExportFuncs();
    updateFilter();

    connect(tManager, SIGNAL(savedUpdated()), this, SLOT(updateRegisteredList()));
    connect(guiHelper, SIGNAL(markingsUpdated()), this, SLOT(updateSavedMarkingColors()));
    connect(ui->resetMarkingsPushButton, SIGNAL(clicked()), this, SLOT(onResetMarkings()));
    connect(guiHelper, SIGNAL(importExportUpdated()), this, SLOT(updateImportExportFuncs()));
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
    QHash<QString, QColor> colors = guiHelper->getMarkingsColor();
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
    QStringList list = guiHelper->getImportExportFunctions();
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
    int offsetbase = guiHelper->getDefaultOffsetBase();
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
            guiHelper->setDefaultOffsetBase(16);
            ui->offsetBaseComboBox->setCurrentIndex(2);
    }
}

void SettingsDialog::updateFilter()
{
    QStringList typesList = tManager->getTypesList();
    QStandardItemModel *model = new(std::nothrow) QStandardItemModel(typesList.size(), 1);
    if (model == NULL ) {
        qFatal("Cannot allocate memory for QStandardItemModel X{");
        return;
    }
    QSet<QString> typesBlacklist = guiHelper->getTypesBlacklist();
    QStandardItem* item = NULL;

    for (int i = 0; i < typesList.size(); ++i)
    {
        item = new(std::nothrow) QStandardItem(typesList.at(i));
        if (item != NULL) {
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setData( (typesBlacklist.contains(typesList.at(i)) ? Qt::Unchecked : Qt::Checked), Qt::CheckStateRole);
            model->setItem(i, 0, item);
        } else {
          qFatal("Cannot allocate memory for QStandardItem 2 X{");
        }
    }
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), guiHelper, SLOT(onFilterChanged(QModelIndex,QModelIndex)));
    ui->filterListView->setModel(model);
}

void SettingsDialog::onImportExportFuncDeletes(const QString &name)
{
    guiHelper->removeImportExportFunctions(name);
}

void SettingsDialog::onResetImportExportFuncs()
{
    guiHelper->resetImportExportFuncs();
}

void SettingsDialog::onDoubleClickImportExportFuncs(QListWidgetItem *item)
{
    DeleteableListItem *itemWid = (DeleteableListItem *)ui->importExportListWidget->itemWidget(item);

    QString name = itemWid->getName();

    QuickViewItemConfig *itemConfig = new(std::nothrow) QuickViewItemConfig(guiHelper, this);
    if (itemConfig != NULL) {
        itemConfig->setWayBoxVisible(false);
        itemConfig->setFormatVisible(false);
        TransformAbstract *ta = guiHelper->getImportExportFunction(name);

        if (ta != NULL) {

            ta = tManager->cloneTransform(ta);
            if (ta != NULL) {
                itemConfig->setTransform(ta);
                itemConfig->setName(name);
                int ret = itemConfig->exec();
                if (ret == QDialog::Accepted) {
                    delete ta;
                    ta = itemConfig->getTransform();
                    QString newName = itemConfig->getName();

                    guiHelper->removeImportExportFunctions(name);
                    guiHelper->addImportExportFunctions(newName, ta);
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
    QuickViewItemConfig *itemConfig = new(std::nothrow) QuickViewItemConfig(guiHelper, this);
    if (itemConfig != NULL) {
        itemConfig->setWayBoxVisible(false);
        itemConfig->setFormatVisible(false);
        int ret = itemConfig->exec();
        if (ret == QDialog::Accepted) {
            TransformAbstract *ta = itemConfig->getTransform();
            if (ta != NULL) {
                guiHelper->addImportExportFunctions(itemConfig->getName(),ta);
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
        guiHelper->getLogger()->logError(tr("Saved conf \"%1\" not found").arg(name),LOGID);
}

void SettingsDialog::onDeleteSaved(const QString &name)
{
    tManager->unregisterChainConf(name);
    ui->savedDescriptLabel->clear();
}

void SettingsDialog::onMarkingDelete(const QString &name)
{
    guiHelper->removeMarkingColor(name);
}

void SettingsDialog::onResetMarkings()
{
    guiHelper->resetMarkings();
}

void SettingsDialog::onServerPortChanged(int port)
{
    guiHelper->setDefaultServerPort(port);
}

void SettingsDialog::onServerDecodeChanged(bool val)
{
    guiHelper->setDefaultServerDecode(val);
}

void SettingsDialog::onServerEncodeChanged(bool val)
{
    guiHelper->setDefaultServerEncode(val);
}

void SettingsDialog::onServerSeparatorChanged(char c)
{
    guiHelper->setDefaultServerSeparator(c);
}

void SettingsDialog::onServerPipeNameChanged(QString name)
{
    guiHelper->setDefaultServerPipeName(name);
}

void SettingsDialog::onOffsetBaseChanged(QString val)
{
    bool ok = false;
    int intval = val.toInt(&ok);

    if (!ok) {
        guiHelper->getLogger()->logError("Invalid integer value for offset base in settings T_T");
    } else {
        guiHelper->setDefaultOffsetBase(intval);
    }
}

