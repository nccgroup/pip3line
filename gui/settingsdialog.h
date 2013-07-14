/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <transformmgmt.h>
#include <QHash>
#include <QModelIndex>
#include <QListWidgetItem>
#include "guihelper.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
        Q_OBJECT
    public:
        explicit SettingsDialog(GuiHelper *helper, QWidget *parent = 0);
        ~SettingsDialog();
        void setVersionUpdateMessage(QString mess);
    signals:
        void updateCheckRequested();
        void error(QString, QString);
    private slots:
        void autoUpdateChanged(bool checked);
        void onUpdateRequest();
        void updatePluginList();
        void updateRegisteredList();
        void updateSavedMarkingColors();
        void updateImportExportFuncs();
        void updateMisc();
        void onPluginClicked(QModelIndex index);
        void onSavedClicked(QListWidgetItem*item);
        void onDeleteSaved(const QString &name);
        void onMarkingDelete(const QString &name);
        void onResetMarkings();
        void onImportExportFuncDeletes(const QString &name);
        void onResetImportExportFuncs();
        void onDoubleClickImportExportFuncs(QListWidgetItem* item);
        void onAddImportExportFuncs();
        void onMinimizeChanged(bool checked);
        void onServerPortChanged(int port);
        void onServerDecodeChanged(bool val);
        void onServerEncodeChanged(bool val);
        void onServerSeparatorChanged(char c);
        void onServerPipeNameChanged(QString name);
        void onOffsetBaseChanged(QString val);
        
    private:
        Q_DISABLE_COPY(SettingsDialog)
        static const QString LOGID;
        TransformMgmt *tManager;
        QHash<QString, int> stackedList;
        Ui::SettingsDialog *ui;
        QSettings *settings;
        GuiHelper *helper;
};

#endif // SETTINGSDIALOG_H
