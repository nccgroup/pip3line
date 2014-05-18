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
#include <QPushButton>
#include <QPoint>
#include <QNetworkAccessManager>
#include "sources/blockssource.h"

namespace Ui {
class MainWindow;
}

class DebugDialog;
class ComparisonDialog;
class QuickViewDialog;
class SettingsDialog;
class AnalyseDialog;
class RegExpHelpDialog;
class MainTabs;
class LoggerWidget;
class TransformMgmt;
class GuiHelper;
class DownloadManager;
class BlocksSource;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    public:
        explicit MainWindow(bool debug = false, QWidget *parent = 0);
        ~MainWindow();
        void loadFile(QString fileName);
    private slots:
        void onAboutPip3line();
        void onAnalyse(bool checked);
        void onHelpWithRegExp();
        void onSettingsDialogOpen(bool checked);
        void checkForUpdates();
        void processingCheckForUpdate(QByteArray data);
        void processingUrlDownload(QByteArray data);
        void iconActivated(QSystemTrayIcon::ActivationReason reason);
        void onImport(QAction* action);
        void onDataFromURL();
        void onQuickView(bool checked);
        void onCompare(bool checked);
        void on_actionLogs_triggered();
        void updateTrayIcon();
        void onDebug();
        void onDebugDestroyed();
        void onNewAction(QAction * action);
        void onNewDefault();
        void showWindow();
        void onExternalBlockReceived(const Block &block);

    private:
        Q_DISABLE_COPY(MainWindow)
        void buildToolBar();
        void initializeLibTransform();
        void closeEvent(QCloseEvent *event);
        void createTrayIcon();

        void hideEvent(QHideEvent * event);
        void showEvent(QShowEvent * event);

        void newFileTab(QString fileName);

        static const QString NEW_TRANSFORMTAB;
        static const QString NEW_FILE;
        static const QString NEW_CURRENTMEM;
        static const QString NEW_BASEHEX;
        static const QString NEW_INTERCEP;
        Ui::MainWindow *ui;
        SettingsDialog *settingsDialog;
        AnalyseDialog *analyseDialog;
        RegExpHelpDialog *regexphelpDialog;
        QuickViewDialog * quickView;
        ComparisonDialog *comparisonView;
        DebugDialog * debugDialog;
        GuiHelper *guiHelper;
        TransformMgmt *transformFactory;
        LoggerWidget *logger;
        MainTabs * mainTabs;

        QNetworkAccessManager networkManager;
        QSystemTrayIcon *trayIcon;
        QMenu *trayIconMenu;
        QMenu *newMenu;
        QAction *trayIconLabel;
        QSettings *settings;

        bool quickViewWasVisible;
        bool settingsWasVisible;
        bool compareWasVisible;
        QPoint savedPos;
        BlocksSource * blockListener;
};

#endif // MAINWINDOW_H
