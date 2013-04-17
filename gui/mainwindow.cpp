/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "debugdialog.h"
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QVBoxLayout>
#include "aboutdialog.h"
#include "messagedialog.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QRect>
#include <QRegExp>
#include <QSettings>
#include <QIcon>
#include <QClipboard>
#include <QRgb>
#include <QDebug>
#include <QColorDialog>
#include <transformabstract.h>
#include <commonstrings.h>
#include "../version.h"
using namespace Pip3lineConst;

MainWindow::MainWindow(bool debug, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qApp->setOrganizationName(APPNAME);
    qApp->setApplicationName(APPNAME);

    ui->setupUi(this);

    if (debug)
        ui->actionDebug_dialog->setVisible(true);
    else
        ui->actionDebug_dialog->setVisible(false);

    connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));

    logger = new LoggerWidget();
    transformFactory = new TransformMgmt();
    connect(transformFactory,SIGNAL(error(QString, QString)),logger,SLOT(logError(QString,QString)));
    connect(transformFactory,SIGNAL(warning(QString,QString)),logger,SLOT(logWarning(QString,QString)));
    connect(transformFactory,SIGNAL(status(QString,QString)),logger,SLOT(logStatus(QString,QString)));

    transformFactory->initialize(QCoreApplication::applicationDirPath());
//    networkProxy.setType(QNetworkProxy::HttpProxy);
//    networkProxy.setHostName("127.0.0.1");
//    networkProxy.setPort(8080);
//    networkManager.setProxy(networkProxy);

    guiHelper = new GuiHelper(transformFactory,&networkManager, logger);

    mainTabs = new MainTabs(guiHelper, this);
    ui->centralWidget->layout()->addWidget(mainTabs);
    analyseDialog = 0;
    regexphelpDialog = 0;
    settingsDialog = 0;
    trayIcon = 0;
    quickView = 0;

    quickViewWasVisible = false;
    settingsWasVisible = false;

    QComboBox * filterComboBox = new QComboBox();
    filterComboBox->installEventFilter(guiHelper);
    guiHelper->buildFilterComboBox(filterComboBox);
    ui->mainToolBar->addWidget(filterComboBox);

    trayIconLabel = 0;
    createTrayIcon();

    connect(ui->actionHelp_with_RegExp, SIGNAL(triggered()), this, SLOT(onHelpWithRegExp()));
    connect(ui->actionAbout_Pip3line, SIGNAL(triggered()), this, SLOT(onAboutPip3line()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionPip3line_settings, SIGNAL(triggered()), this, SLOT(onSettingsDialogOpen()));
    connect(ui->actionMagic, SIGNAL(triggered()), this, SLOT(onQuickView()));
    connect(ui->actionDebug_dialog, SIGNAL(triggered()), this, SLOT(onDebug()));

    settings = transformFactory->getSettingsObj();
    if (settings->value(SETTINGS_AUTO_UPDATE, true).toBool()) {
        checkForUpdates();
    }
}

MainWindow::~MainWindow()
{
    delete mainTabs;
    if (quickView != 0)
        delete quickView;

    if (trayIconMenu != 0)
        delete trayIconMenu;

    // delete logger // no need for that, already done by the main window
    if (analyseDialog != 0)
        delete analyseDialog;

    if (regexphelpDialog != 0)
        delete regexphelpDialog;

    if (trayIcon != 0)
        delete trayIcon;

    delete settings;

    delete guiHelper;
    delete transformFactory;

    if (settingsDialog != 0)
        delete settingsDialog;

    delete ui;

}

void MainWindow::onAnalyse()
{
    if (!analyseDialog) {
        analyseDialog = new AnalyseDialog(this);
    }

    if (!analyseDialog->isVisible()) {
        QRect scr = QApplication::desktop()->screenGeometry();
        analyseDialog->move(scr.center() - rect().center() );

        analyseDialog->show();
        analyseDialog->raise();
        analyseDialog->activateWindow();
    }
}

void MainWindow::onAboutPip3line()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::onHelpWithRegExp()
{
    if (!regexphelpDialog) {
        regexphelpDialog = new RegExpHelpDialog(this);
    }

    regexphelpDialog->show();
    regexphelpDialog->raise();
    regexphelpDialog->activateWindow();
}

void MainWindow::onSettingsDialogOpen()
{
    if (settingsDialog == 0) {
        settingsDialog = new SettingsDialog(guiHelper, this);
        connect(settingsDialog, SIGNAL(updateCheckRequested()), this, SLOT(checkForUpdates()));
    }

    settingsDialog->show();
    settingsDialog->raise();
    settingsDialog->activateWindow();
}

void MainWindow::checkForUpdates()
{
    QUrl resource(UPDATE_URL);
    DownloadManager * dm = new DownloadManager(resource, &networkManager);
    connect(dm,SIGNAL(error(QString,QString)), logger,SLOT(logError(QString,QString)));
    connect(dm, SIGNAL(warning(QString,QString)), logger, SLOT(logWarning(QString,QString)));
    connect(dm, SIGNAL(finished(DownloadManager*)), this, SLOT(processingCheckForUpdate(DownloadManager*)));

    dm->launch();
}

void MainWindow::processingCheckForUpdate(DownloadManager *dm)
{
    QString data = QString::fromUtf8(dm->getData());
    qDebug() << data;
    QString message = "Unknown";
    QRegExp versionRegExp("#define VERSION_STRING \"(.*)\"");
    if (data.isEmpty()) {
        message = tr("Empty network response, cannot check for new Pip3line version");
        logger->logWarning(message);
    } else {
        int pos = versionRegExp.indexIn(data,0);
        if (pos != -1) {
            float last = versionRegExp.cap(1).mid(0,3).toFloat();
            float current = QString(VERSION_STRING).toFloat();
            if (last > current) {
                message = tr("Current: %1\nLatest: %2\n\nNew version of Pip3line available at\n \nhttps://code.google.com/p/pip3line/downloads/list").arg(VERSION_STRING).arg(versionRegExp.cap(1));
                QMessageBox::warning(this,tr("New update"),message,QMessageBox::Ok);
            } else {
                message = tr("Latest version of Pip3line in use (%1), everything is fine").arg(VERSION_STRING);
                logger->logStatus(message);

            }
        } else {
            message = tr("Got a network response but cannot find the version string.");
            logger->logWarning(message,"Version Check");
        }
    }

    if (settingsDialog != 0)
        settingsDialog->setVersionUpdateMessage(message);

    delete dm;
}

void MainWindow::processingUrlDownload(DownloadManager *dm)
{
    QByteArray data = dm->getData();
    if (data.isEmpty()) {
        data = "No data from URL";
        mainTabs->newTabTransform(data);
    } else {
        mainTabs->newTabTransform(data);
    }
    delete dm;
    showWindow();
}

void MainWindow::on_actionNew_Tab_triggered()
{
    mainTabs->newTabTransform();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool minimize = settings->value(SETTINGS_MINIMIZE_TO_TRAY, true).toBool();
    if (minimize && trayIcon != 0) {
        if (quickView != 0) {
            quickViewWasVisible = quickView->isVisible();
            if (quickViewWasVisible)
                quickView->hide();
        }
        if (settingsDialog != 0) {
            settingsWasVisible = settingsDialog->isVisible();
            if (settingsWasVisible)
                settingsDialog->hide();
        }
        hide();
        event->ignore();
    } else {
        qApp->quit();
        event->accept();
    }
}

void MainWindow::createTrayIcon()
{
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        trayIconMenu = new QMenu(this);
        updateTrayIcon();
        connect(guiHelper, SIGNAL(importExportUpdated()), this, SLOT(updateTrayIcon()));
        trayIcon->show();
    }
}

void MainWindow::showWindow()
{
    showNormal();
    activateWindow();
    if (quickView != 0 && quickViewWasVisible)
        quickView->show();

    if (settingsDialog != 0 && settingsWasVisible)
        settingsDialog->show();

}

void MainWindow::updateTrayIcon()
{
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        trayIconMenu->clear();
        trayIconLabel = new QAction(tr("Import from clipboard as"), trayIconMenu);
        trayIconLabel->setDisabled(true);
        trayIconMenu->addAction(trayIconLabel);
        QAction * action = new QAction(GuiHelper::ACTION_UTF8_STRING, trayIconMenu);
        trayIconMenu->addAction(action);
        QStringList list = guiHelper->getImportExportFunctions();
        qSort(list);
        for (int i = 0; i < list.size(); i++) {
            action = new QAction(list.at(i), trayIconMenu);
            trayIconMenu->addAction(action);
        }
        connect(trayIconMenu, SIGNAL(triggered(QAction*)), this, SLOT(onImport(QAction*)), Qt::UniqueConnection);

        trayIconMenu->addSeparator();
        trayIconMenu->addAction(ui->actionGet_data_from_URL);
        connect(ui->actionGet_data_from_URL, SIGNAL(triggered()), this, SLOT(onDataFromURL()));
        trayIconMenu->addSeparator();

        trayIconMenu->addAction(ui->actionExit);
        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setIcon(QIcon(":/Images/icons/pip3line.png"));
        trayIcon->setContextMenu(trayIconMenu);
        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                     this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    }
}

void MainWindow::onDebug()
{
    qDebug() << "Debugging";
    DebugDialog * dialog = new DebugDialog(this);

    dialog->show();
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::MiddleClick:
    case QSystemTrayIcon::DoubleClick:
        showWindow();
        break;
    case QSystemTrayIcon::Context:
        {
            QString input = QApplication::clipboard()->text();
            QList<QAction *> actions = trayIconMenu->actions();
            actions.removeAll(trayIconLabel);
            actions.removeAll(ui->actionExit);
            for (int i = 0; i < actions.size(); i++) {
                actions.at(i)->setEnabled(!input.isEmpty());
            }
        }
        break;
    default:
        ;
    }
}

void MainWindow::onImport(QAction *action)
{
    if (action != ui->actionExit) {
        QClipboard *clipboard = QApplication::clipboard();
        QString input = clipboard->text();
        if (action->text() == GuiHelper::ACTION_UTF8_STRING) {
            mainTabs->newTabTransform(input.toUtf8());
        } else {
            TransformAbstract *ta  = guiHelper->getImportExportFunction(action->text());
            if (ta != 0) {
                ta->setWay(TransformAbstract::OUTBOUND);
                mainTabs->newTabTransform(ta->transform(input.toUtf8()));
            }
        }
        showWindow();
    }
}

void MainWindow::onDataFromURL()
{
    QString input = QApplication::clipboard()->text();
    QUrl resource(input);
    if (resource.isValid()) {
        DownloadManager * dm = new DownloadManager(resource, &networkManager);
        connect(dm,SIGNAL(error(QString,QString)), logger,SLOT(logError(QString,QString)));
        connect(dm, SIGNAL(warning(QString,QString)), logger, SLOT(logWarning(QString,QString)));
        connect(dm, SIGNAL(finished(DownloadManager*)), this, SLOT(processingUrlDownload(DownloadManager*)));
        dm->launch();
    }
}

void MainWindow::onQuickView()
{
    if (quickView == 0) {
        quickView = new QuickViewDialog(guiHelper, this);
        connect(guiHelper, SIGNAL(newSelection(QByteArray)), quickView, SLOT(receivingData(QByteArray)));
    }

    quickView->show();
    quickView->raise();
}

void MainWindow::on_actionLogs_triggered()
{
    mainTabs->showLogs();
}
