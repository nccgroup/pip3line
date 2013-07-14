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
    QMainWindow(parent)
{
    analyseDialog = NULL;
    regexphelpDialog = NULL;
    settingsDialog = NULL;
    trayIcon = NULL;
    quickView = NULL;
    trayIconLabel = NULL;

    ui = new(std::nothrow) Ui::MainWindow();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::MainWindow X{");
    }
    qApp->setOrganizationName(APPNAME);
    qApp->setApplicationName(APPNAME);

    ui->setupUi(this);

    logger = new(std::nothrow) LoggerWidget();
    if (logger == NULL) {
        qFatal("Cannot allocate memory for LoggerWidget X{");
    }

    initializeLibTransform();

    guiHelper = new(std::nothrow) GuiHelper(transformFactory,&networkManager, logger);
    if (guiHelper == NULL) {
        qFatal("Cannot allocate memory for GuiHelper X{");
    }

    setStyleSheet("QWidget {} ");


    buildToolBar();

    if (debug)
        ui->actionDebug_dialog->setVisible(true);
    else
        ui->actionDebug_dialog->setVisible(false);

//    networkProxy.setType(QNetworkProxy::HttpProxy);
//    networkProxy.setHostName("127.0.0.1");
//    networkProxy.setPort(8080);
//    networkManager.setProxy(networkProxy);





    mainTabs = new(std::nothrow) MainTabs(guiHelper, this);
    if (mainTabs == NULL) {
        qFatal("Cannot allocate memory for MainTabs X{");
    }
    ui->centralWidget->layout()->addWidget(mainTabs);

    quickViewWasVisible = false;
    settingsWasVisible = false;

    createTrayIcon();

    settings = transformFactory->getSettingsObj();
    if (settings->value(SETTINGS_AUTO_UPDATE, true).toBool()) {
        checkForUpdates();
    }
}

MainWindow::~MainWindow()
{
    delete mainTabs;
    delete quickView;
    delete trayIconMenu;
    logger = NULL; // no need to delete, already done by the main tab gui
    delete analyseDialog;
    delete regexphelpDialog;
    delete trayIcon;
    delete settings;
    delete guiHelper;
    delete transformFactory;
    delete settingsDialog;

    delete ui;

}

void MainWindow::buildToolBar()
{
//    QPushButton * newPushButton = new(std::nothrow) QPushButton("New",this);
//    if (newPushButton == NULL) {
//        qFatal("Cannot allocate memory for newPushButton X{");
//    }

//   // newPushButton->setMenu();

//    ui->mainToolBar->insertWidget(ui->actionNew_Tab,newPushButton);


 
    QComboBox * filterComboBox = new(std::nothrow) QComboBox();
    if (filterComboBox == NULL) {
        qFatal("Cannot allocate memory for filterComboBox X{");
    }

    filterComboBox->setFrame(false);

    filterComboBox->installEventFilter(guiHelper);
    guiHelper->buildFilterComboBox(filterComboBox);
    ui->mainToolBar->addWidget(filterComboBox);

    connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));

    connect(ui->actionHelp_with_RegExp, SIGNAL(triggered()), this, SLOT(onHelpWithRegExp()));
    connect(ui->actionAbout_Pip3line, SIGNAL(triggered()), this, SLOT(onAboutPip3line()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionPip3line_settings, SIGNAL(triggered()), this, SLOT(onSettingsDialogOpen()));
    connect(ui->actionMagic, SIGNAL(triggered()), this, SLOT(onQuickView()));
    connect(ui->actionDebug_dialog, SIGNAL(triggered()), this, SLOT(onDebug()));

}

void MainWindow::initializeLibTransform()
{
    transformFactory = new(std::nothrow) TransformMgmt();
    if (transformFactory == NULL) {
        qFatal("Cannot allocate memory for the Transform Factory X{");
    }
    connect(transformFactory,SIGNAL(error(QString, QString)),logger,SLOT(logError(QString,QString)));
    connect(transformFactory,SIGNAL(warning(QString,QString)),logger,SLOT(logWarning(QString,QString)));
    connect(transformFactory,SIGNAL(status(QString,QString)),logger,SLOT(logStatus(QString,QString)));

    transformFactory->initialize(QCoreApplication::applicationDirPath());
}

void MainWindow::loadFile(QString fileName)
{
    mainTabs->loadFile(fileName);
}

void MainWindow::onAnalyse()
{
    if (!analyseDialog) {
        analyseDialog = new(std::nothrow) AnalyseDialog(this);
        if (analyseDialog == NULL) {
            qFatal("Cannot allocate memory for analyseDialog X{");
            return;
        }
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
        regexphelpDialog = new(std::nothrow) RegExpHelpDialog(this);
        if (regexphelpDialog == NULL) {
            qFatal("Cannot allocate memory for regexphelpDialog X{");
            return;
        }
    }

    regexphelpDialog->show();
    regexphelpDialog->raise();
    regexphelpDialog->activateWindow();
}

void MainWindow::onSettingsDialogOpen()
{
    if (settingsDialog == NULL) {
        settingsDialog = new(std::nothrow) SettingsDialog(guiHelper, this);
        if (settingsDialog == NULL) {
            qFatal("Cannot allocate memory for settingsDialog X{");
            return;
        }
        else
            connect(settingsDialog, SIGNAL(updateCheckRequested()), this, SLOT(checkForUpdates()));
    }

    settingsDialog->show();
    settingsDialog->raise();
    settingsDialog->activateWindow();
}

void MainWindow::checkForUpdates()
{
    QUrl resource(UPDATE_URL);
    DownloadManager * dm = new(std::nothrow) DownloadManager(resource, &networkManager);
    if (dm == NULL) {
        qFatal("Cannot allocate memory for DownloadManager (updates) X{");
        return;
    }
    connect(dm,SIGNAL(error(QString,QString)), logger,SLOT(logError(QString,QString)));
    connect(dm, SIGNAL(warning(QString,QString)), logger, SLOT(logWarning(QString,QString)));
    connect(dm, SIGNAL(finished(DownloadManager*)), this, SLOT(processingCheckForUpdate(DownloadManager*)));

    dm->launch();
}

void MainWindow::processingCheckForUpdate(DownloadManager *dm)
{
    QString data = QString::fromUtf8(dm->getData());

    QString message = "Unknown";
    QRegExp versionRegExp("#define VERSION_STRING \"([0-9]\\.[0-9]\\.?[0-9]?)\"");
    if (data.isEmpty()) {
        message = tr("Empty network response, cannot check for new Pip3line version");
        logger->logWarning(message);
    } else {

        int pos = versionRegExp.indexIn(data,0);
        qDebug() << "Version? " << versionRegExp.cap(1);
        if (pos != -1) {
            float last = versionRegExp.cap(1).mid(0,3).toFloat();
            float current = QString(VERSION_STRING).mid(0,3).toFloat();
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

    if (settingsDialog != NULL)
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
    if (minimize && trayIcon != NULL) {
        if (quickView != NULL) {
            quickViewWasVisible = quickView->isVisible();
            if (quickViewWasVisible)
                quickView->hide();
        }
        if (settingsDialog != NULL) {
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
        trayIconMenu = new(std::nothrow) QMenu(this);
        if (trayIconMenu == NULL) {
            qFatal("Cannot allocate memory for trayIconMenu X{");
            return;
        }
        updateTrayIcon();
        connect(guiHelper, SIGNAL(importExportUpdated()), this, SLOT(updateTrayIcon()));
        trayIcon->show();
    }
}

void MainWindow::showWindow()
{
    showNormal();
    activateWindow();
    if (quickView != NULL && quickViewWasVisible)
        quickView->show();

    if (settingsDialog != NULL && settingsWasVisible)
        settingsDialog->show();

}

void MainWindow::updateTrayIcon()
{
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        trayIconMenu->clear();
        trayIconLabel = new(std::nothrow) QAction(tr("Import from clipboard as"), trayIconMenu);
        if (trayIconLabel == NULL) {
            qFatal("Cannot allocate memory for trayIconLabel X{");
            return;
        }
        trayIconLabel->setDisabled(true);
        trayIconMenu->addAction(trayIconLabel);
        QAction * action = new(std::nothrow) QAction(GuiHelper::UTF8_STRING_ACTION, trayIconMenu);
        if (action == NULL) {
            qFatal("Cannot allocate memory for QAction in mainwindow 1 X{");
            return;
        }

        trayIconMenu->addAction(action);
        QStringList list = guiHelper->getImportExportFunctions();
        qSort(list);
        for (int i = 0; i < list.size(); i++) {
            action = new(std::nothrow) QAction(list.at(i), trayIconMenu);
            if (action == NULL) {
                qFatal("Cannot allocate memory for QAction in mainwindow 2 X{");
                return;
            }
            trayIconMenu->addAction(action);
        }
        connect(trayIconMenu, SIGNAL(triggered(QAction*)), this, SLOT(onImport(QAction*)), Qt::UniqueConnection);

        trayIconMenu->addSeparator();
        trayIconMenu->addAction(ui->actionGet_data_from_URL);
        connect(ui->actionGet_data_from_URL, SIGNAL(triggered()), this, SLOT(onDataFromURL()));
        trayIconMenu->addSeparator();

        trayIconMenu->addAction(ui->actionExit);
        trayIcon = new(std::nothrow) QSystemTrayIcon(this);
        if (trayIcon == NULL) {
            qFatal("Cannot allocate memory for trayIcon X{");
            return;
        }
        trayIcon->setIcon(QIcon(":/Images/icons/pip3line.png"));
        trayIcon->setContextMenu(trayIconMenu);
        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                     this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    }
}

void MainWindow::onDebug()
{
    qDebug() << "Debugging";
    DebugDialog * dialog = new(std::nothrow) DebugDialog(this);
    if (dialog == NULL) {
        qFatal("Cannot allocate memory for DebugDialog X{");
        return;
    }

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
        qWarning() << tr("Unmanaged Sytem tray activation reason: %1").arg(reason);
    }
}

void MainWindow::onImport(QAction *action)
{
    if (action != ui->actionExit) {
        QClipboard *clipboard = QApplication::clipboard();
        QString input = clipboard->text();
        if (action->text() == GuiHelper::UTF8_STRING_ACTION) {
            mainTabs->newTabTransform(input.toUtf8());
        } else {
            TransformAbstract *ta  = guiHelper->getImportExportFunction(action->text());
            if (ta != NULL) {
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
        DownloadManager * dm = new(std::nothrow) DownloadManager(resource, &networkManager);
        if (dm == NULL) {
            qFatal("Cannot allocate memory for DownloadManager (url) X{");
            return;
        }
        connect(dm,SIGNAL(error(QString,QString)), logger,SLOT(logError(QString,QString)));
        connect(dm, SIGNAL(warning(QString,QString)), logger, SLOT(logWarning(QString,QString)));
        connect(dm, SIGNAL(finished(DownloadManager*)), this, SLOT(processingUrlDownload(DownloadManager*)));
        dm->launch();
    }
}

void MainWindow::onQuickView()
{
    if (quickView == NULL) {
        quickView = new(std::nothrow) QuickViewDialog(guiHelper, this);
        if (quickView == NULL) {
            qFatal("Cannot allocate memory for QuickViewDialog X{");
            return;
        }
        connect(guiHelper, SIGNAL(newSelection(QByteArray)), quickView, SLOT(receivingData(QByteArray)));
    }

    quickView->show();
    quickView->raise();
}

void MainWindow::on_actionLogs_triggered()
{
    mainTabs->showLogs();
}
