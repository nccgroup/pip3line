/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <transformmgmt.h>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QNetworkProxy>
#include <QPushButton>
#include "analysedialog.h"
#include "massprocessingdialog.h"
#include "regexphelpdialog.h"
#include "loggerwidget.h"
#include "customdialogdockwidget.h"
#include "settingsdialog.h"
#include "downloadmanager.h"
#include "guihelper.h"
#include "maintabs.h"
#include "quickviewdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    public:
        explicit MainWindow(bool debug = false, QWidget *parent = 0);
        ~MainWindow();
        void loadFile(QString fileName);
    private slots:
        void onAboutPip3line();
        void onAnalyse();
        void onHelpWithRegExp();
        void onSettingsDialogOpen();
        void checkForUpdates();
        void processingCheckForUpdate(DownloadManager * dm);
        void processingUrlDownload(DownloadManager * dm);
        void on_actionNew_Tab_triggered();
        void iconActivated(QSystemTrayIcon::ActivationReason reason);
        void onImport(QAction* action);
        void onDataFromURL();
        void onQuickView();
        void on_actionLogs_triggered();
        void updateTrayIcon();
        void onDebug();
    private:
        Q_DISABLE_COPY(MainWindow)
        void buildToolBar();
        void initializeLibTransform();
        void closeEvent(QCloseEvent *event);
        void createTrayIcon();
        void showWindow();

        Ui::MainWindow *ui;
        SettingsDialog *settingsDialog;
        AnalyseDialog *analyseDialog;
        RegExpHelpDialog *regexphelpDialog;
        GuiHelper *guiHelper;
        QNetworkAccessManager networkManager;
        TransformMgmt *transformFactory;
        LoggerWidget *logger;
        MainTabs * mainTabs;
        QSystemTrayIcon *trayIcon;
        QMenu *trayIconMenu;
        QAction *trayIconLabel;
        QSettings *settings;
        QuickViewDialog * quickView;
        bool quickViewWasVisible;
        bool settingsWasVisible;
        QNetworkProxy networkProxy;
      //  QPushButton *newPushButton;
};

#endif // MAINWINDOW_H
