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
#include <QTimer>
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
#include <QFileDialog>
#include "tabs/generictab.h"
#include "tabs/randomaccesstab.h"
#include "sources/currentmemorysource.h"
#include "sources/largefile.h"
#include "sources/basicsource.h"
#include "../version.h"
#include "analysedialog.h"
#include "massprocessingdialog.h"
#include "regexphelpdialog.h"
#include "loggerwidget.h"
#include "settingsdialog.h"
#include "downloadmanager.h"
#include "guihelper.h"
#include "maintabs.h"
#include "quickviewdialog.h"
#include "comparisondialog.h"
#include "sources/tcpserverlistener.h"
#include "sources/tcplistener.h"
#include "sources/intercepsource.h"

using namespace Pip3lineConst;

const QString MainWindow::NEW_TRANSFORMTAB = "Transform tab";
const QString MainWindow::NEW_FILE = "File";
const QString MainWindow::NEW_CURRENTMEM = "Current memory";
const QString MainWindow::NEW_BASEHEX = "Hexeditor";
const QString MainWindow::NEW_INTERCEP = "Interceptor";

MainWindow::MainWindow(bool debug, QWidget *parent) :
    QMainWindow(parent)
{
    analyseDialog = NULL;
    regexphelpDialog = NULL;
    settingsDialog = NULL;
    trayIcon = NULL;
    quickView = NULL;
    comparisonView = NULL;
    debugDialog = NULL;
    trayIconLabel = NULL;
    newMenu = NULL;
    blockListener = NULL;
    trayIconMenu = NULL;

    settingsWasVisible = false;
    quickViewWasVisible = false;
    compareWasVisible = false;

    ui = new(std::nothrow) Ui::MainWindow();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::MainWindow X{");
    }

    qRegisterMetaType<Pip3lineConst::LOGLEVEL>("Pip3lineConst::LOGLEVEL");
    qRegisterMetaType<quintptr>("quintptr");
    qRegisterMetaType<Block>("Block");
    qRegisterMetaType<QHostAddress>("QHostAddress");
#if QT_VERSION >= 0x050000
    qRegisterMetaType<qintptr>("qintptr");
#endif
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

    buildToolBar();

    if (debug)
        ui->actionDebug_dialog->setVisible(true);
    else
        ui->actionDebug_dialog->setVisible(false);

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
    qApp->setStyleSheet("TextCell {background: white}");
   // blockListener = new(std::nothrow) UdpListener(this);
    blockListener = new(std::nothrow) TcpServerListener();
    if (blockListener == NULL) {
        qFatal("Cannot allocate memory for BlocksSource X{");
    }

    // by default everything is base64 encoded, to avoid parsing issues
    blockListener->setDecodeinput(true);

    connect(blockListener, SIGNAL(blockReceived(Block)), SLOT(onExternalBlockReceived(Block)),Qt::QueuedConnection);
    connect(blockListener,SIGNAL(error(QString,QString)), logger,SLOT(logError(QString,QString)));
    connect(blockListener, SIGNAL(status(QString,QString)), logger, SLOT(logStatus(QString,QString)));
    QTimer::singleShot(0,blockListener,SLOT(startListening()));

    connect(guiHelper,SIGNAL(raiseWindowRequest()), SLOT(showWindow()));

//  qApp->setStyleSheet("* {color : green; background: black}");
}

MainWindow::~MainWindow()
{
    //qDebug() << "Destroying main window";
    if (blockListener != NULL) {
        blockListener->stopListening();
        delete blockListener;
    }
    delete debugDialog;
    delete newMenu;
    delete quickView;
    delete comparisonView;// these two first to avoid weird bugs
    delete mainTabs;
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
    QPushButton * newPushButton = new(std::nothrow) QPushButton(QIcon(":/Images/icons/document-new-3.png"),"",this);
    if (newPushButton == NULL) {
        qFatal("Cannot allocate memory for newPushButton X{");
    }

    newPushButton->setToolTip(tr("Special bytes sources"));

    newMenu = new(std::nothrow) QMenu();
    if (newMenu == NULL) {
        qFatal("Cannot allocate memory for newPushButton X{");
    }

    connect(ui->actionNewDefault, SIGNAL(triggered()), SLOT(onNewDefault()));

    QAction * action = new(std::nothrow) QAction(NEW_TRANSFORMTAB, newMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for QAction NEW_DEFAULT X{");
    }
    newMenu->addAction(action);

    action = new(std::nothrow) QAction(NEW_BASEHEX, newMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for QAction NEW_BASEHEX X{");
    }
    newMenu->addAction(action);

    action = new(std::nothrow) QAction(NEW_FILE, newMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for QAction NEW_FILE X{");
    }
    newMenu->addAction(action);

    action = new(std::nothrow) QAction(NEW_CURRENTMEM, newMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for QAction NEW_CURRENTMEM X{");
    }
    newMenu->addAction(action);

//    action = new(std::nothrow) QAction(NEW_INTERCEP, newMenu);
//    if (action == NULL) {
//    qFatal("Cannot allocate memory for QAction NEW_INTERCEP X{");
//    }
//    newMenu->addAction(action);

    newPushButton->setMenu(newMenu);

    newPushButton->setFlat(true);
    connect(newMenu, SIGNAL(triggered(QAction*)), SLOT(onNewAction(QAction*)));

    ui->mainToolBar->insertWidget(ui->mainToolBar->actions().at(1),newPushButton );
   // ui->mainToolBar->insertSeparator(ui->action_Analyse);

    connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));

    connect(ui->actionHelp_with_RegExp, SIGNAL(triggered()), this, SLOT(onHelpWithRegExp()));
    connect(ui->actionAbout_Pip3line, SIGNAL(triggered()), this, SLOT(onAboutPip3line()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionPip3line_settings, SIGNAL(toggled(bool)), this, SLOT(onSettingsDialogOpen(bool)));
    connect(ui->actionMagic, SIGNAL(toggled(bool)), this, SLOT(onQuickView(bool)));
    connect(ui->actionDebug_dialog, SIGNAL(triggered()), this, SLOT(onDebug()));
    connect(ui->actionCompare, SIGNAL(toggled(bool)), this, SLOT(onCompare(bool)));
    connect(ui->action_Analyse, SIGNAL(toggled(bool)), SLOT(onAnalyse(bool)));

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
    qDebug() << "loading file" << fileName;
    newFileTab(fileName);
    //mainTabs->loadFile(fileName);
}

void MainWindow::onAboutPip3line()
{
    AboutDialog dialog(guiHelper, this);
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

void MainWindow::onSettingsDialogOpen(bool checked)
{
    if (checked) {
        if (settingsDialog == NULL) {
            settingsDialog = new(std::nothrow) SettingsDialog(guiHelper, this);
            if (settingsDialog == NULL) {
                qFatal("Cannot allocate memory for settingsDialog X{");
                return;
            }
            connect(settingsDialog, SIGNAL(updateCheckRequested()), this, SLOT(checkForUpdates()));
            settingsDialog->attachAction(ui->actionPip3line_settings);
        }

        settingsDialog->show();
        settingsDialog->raise();
    } else if (settingsDialog != NULL) {
        settingsDialog->hide();
    }
}

void MainWindow::checkForUpdates()
{
    QUrl resource(UPDATE_URL);
    DownloadManager * dm = new(std::nothrow) DownloadManager(resource, guiHelper);
    if (dm == NULL) {
        qFatal("Cannot allocate memory for DownloadManager (updates) X{");
        return;
    }

    connect(dm, SIGNAL(finished(QByteArray)), this, SLOT(processingCheckForUpdate(QByteArray)));
    guiHelper->requestDownload(resource, NULL,dm);
}

void MainWindow::processingCheckForUpdate(QByteArray bdata)
{
    QString data = QString::fromUtf8(bdata.constData(),bdata.size());
    QString message = "Unknown";

    QRegExp versionRegExp("^([0-9]{1,2})\\.([0-9]{1,2})\\.?([0-9]{0,2})");
    if (data.isEmpty()) {
        message = tr("Empty network response, cannot check for new Pip3line version");
        logger->logWarning(message);
    } else {

        int pos = versionRegExp.indexIn(data,0);

        if (pos != -1) {
            uint major = versionRegExp.cap(1).toUInt();
            uint minor = versionRegExp.cap(2).toUInt();
            uint rev = 0;
            if (versionRegExp.captureCount() > 2)
                rev = versionRegExp.cap(3).toUInt();

            qDebug() << tr("Remote Version %1.%2.%3").arg(major).arg(minor).arg(rev);
            qDebug() << tr("Current Version %1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_REV);

            if (major > VERSION_MAJOR ||
                    (major == VERSION_MAJOR && minor > VERSION_MINOR) ||
                    (major == VERSION_MAJOR && minor == VERSION_MINOR && rev > VERSION_REV)) {
                message = tr("Current: %1\nLatest: %2.%3.%4\n\nNew version of Pip3line available at\n \nhttps://code.google.com/p/pip3line/downloads/list").arg(QString(VERSION_STRING)).arg(major).arg(minor).arg(rev);
                QMessageBox::warning(this,tr("New update"),message,QMessageBox::Ok);
            } else {
                message = tr("Latest version of Pip3line in use (%1), everything is fine").arg(QString(VERSION_STRING));
                logger->logStatus(message);

            }
        } else {
            message = tr("Got a network response but cannot find the version string.");
            logger->logWarning(message,"Version Check");
        }
    }

    if (settingsDialog != NULL)
        settingsDialog->setVersionUpdateMessage(message);
}

void MainWindow::processingUrlDownload(QByteArray data)
{
    if (data.isEmpty()) {
        data = "No data from URL";
        mainTabs->newTabTransform(data);
    } else {
        mainTabs->newTabTransform(data);
    }
    showWindow();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool minimize = settings->value(SETTINGS_MINIMIZE_TO_TRAY, true).toBool();
    if (minimize && trayIcon != NULL) {
        guiHelper->goIntoHidding();
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
        QApplication::setQuitOnLastWindowClosed(false);
        trayIconMenu = new(std::nothrow) QMenu(this);
        if (trayIconMenu == NULL) {
            qFatal("Cannot allocate memory for trayIconMenu X{");
            return;
        }
        updateTrayIcon();
        trayIcon = new(std::nothrow) QSystemTrayIcon(this);
        if (trayIcon == NULL) {
            qFatal("Cannot allocate memory for trayIcon X{");
            return;
        }
        trayIcon->setIcon(QIcon(":/Images/icons/pip3line.png"));
        trayIcon->setContextMenu(trayIconMenu);
        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                     this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
        connect(guiHelper, SIGNAL(importExportUpdated()), this, SLOT(updateTrayIcon()));
        trayIcon->show();
    } else {
        qCritical() << tr("No System tray seems to be available, disabling tray Icon menu functionality");
    }
}

void MainWindow::showWindow()
{
    if (!isActiveWindow()) {
        showNormal();
        activateWindow();
    }

}

void MainWindow::onExternalBlockReceived(const Block &block)
{
    guiHelper->routeExternalDataBlock(block.data);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    savedPos = pos();
    guiHelper->goIntoHidding();
    QMainWindow::hideEvent(event);
}

void MainWindow::showEvent(QShowEvent *event)
{
    if (!savedPos.isNull())
        move(savedPos);
    guiHelper->isRising();
    QMainWindow::showEvent(event);
}

void MainWindow::newFileTab(QString fileName)
{
    LargeFile *fs = new(std::nothrow) LargeFile();
    if (fs == NULL) {
        qFatal("Cannot allocate memory for FileSource X{");
    }
    connect(fs,SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), logger,SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)),Qt::QueuedConnection);
    if (!fileName.isEmpty()) {
        fs->fromLocalFile(fileName);
    }
    TabAbstract *newTab = new(std::nothrow) RandomAccessTab(fs, guiHelper,this);
    if (newTab == NULL) {
        qFatal("Cannot allocate memory for RandomAccessTab X{");
    }
    if (!fileName.isEmpty()) {
        newTab->setName(QFileInfo(fileName).fileName());
    }

    mainTabs->integrateTab(newTab);
}

void MainWindow::updateTrayIcon()
{
    if (trayIconMenu != NULL) {
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

    }
}

void MainWindow::onDebug()
{
    qDebug() << "Debugging";
    if (debugDialog == NULL) {
        debugDialog = new(std::nothrow) DebugDialog(guiHelper, this);
        if (debugDialog == NULL) {
            qFatal("Cannot allocate memory for DebugDialog X{");
            return;
        }
        connect(debugDialog,SIGNAL(destroyed()), SLOT(onDebugDestroyed()));
    }

    debugDialog->show();
}

void MainWindow::onDebugDestroyed()
{
    debugDialog = NULL;
}

void MainWindow::onNewAction(QAction *action)
{
    if (action->text() == NEW_FILE) {
        QString fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"));
        if (!fileName.isEmpty()) {
            newFileTab(fileName);
        }
    } else if (action->text() == NEW_TRANSFORMTAB) {
        mainTabs->newTabTransform();
    } else if (action->text() == NEW_CURRENTMEM) {
        CurrentMemorysource *source = new(std::nothrow) CurrentMemorysource();
        if (source == NULL) {
            qFatal("Cannot allocate memory for CurrentMemorysource X{");
        }
        connect(source,SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), logger,SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)),Qt::QueuedConnection);

        RandomAccessTab *raTab = new(std::nothrow) RandomAccessTab(source,guiHelper,mainTabs);
        if (raTab == NULL) {
            qFatal("Cannot allocate memory for RandomAccessTab X{");
        }
        raTab->setName("Current memory");

        mainTabs->integrateTab(raTab);
    } else if (action->text() == NEW_BASEHEX) {
        BasicSource * bs = new(std::nothrow) BasicSource();
        if (bs == NULL) {
            qFatal("Cannot allocate memory for BasicSearch X{");
        }
        connect(bs,SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), logger,SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)),Qt::QueuedConnection);

        GenericTab *tab = new(std::nothrow) GenericTab(bs,guiHelper,mainTabs);
        if (tab == NULL) {
            qFatal("Cannot allocate memory for GenericTab X{");
        }
        tab->setName("");

        mainTabs->integrateTab(tab);

    } else if (action->text() == NEW_INTERCEP) {
        IntercepSource * is = new(std::nothrow) IntercepSource();
        if (is == NULL) {
            qFatal("Cannot allocate memory for IntercepSource X{");
        }
        connect(is,SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), logger,SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)),Qt::QueuedConnection);

        GenericTab *tab = new(std::nothrow) GenericTab(is,guiHelper,mainTabs);
        if (tab == NULL) {
            qFatal("Cannot allocate memory for GenericTab X{");
        }
        tab->setName("");

        mainTabs->integrateTab(tab);
    }
}

void MainWindow::onNewDefault()
{
    mainTabs->newTabTransform();
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
        DownloadManager * dm = new(std::nothrow) DownloadManager(resource, guiHelper);
        if (dm == NULL) {
            qFatal("Cannot allocate memory for DownloadManager (url) X{");
            return;
        }
        connect(dm, SIGNAL(finished(QByteArray)), this, SLOT(processingUrlDownload(QByteArray)));
        guiHelper->requestDownload(resource, NULL,dm);
    }
}

void MainWindow::onQuickView(bool checked)
{
    if (checked) {
        if (quickView == NULL) {
            quickView = new(std::nothrow) QuickViewDialog(guiHelper, this);
            if (quickView == NULL) {
                qFatal("Cannot allocate memory for QuickViewDialog X{");
                return;
            }
            connect(guiHelper, SIGNAL(newSelection(QByteArray)), quickView, SLOT(receivingData(QByteArray)));
            quickView->attachAction(ui->actionMagic);
        }

        quickView->show();
        quickView->raise();
    } else if (quickView != NULL) {
        quickView->hide();
    }
}

void MainWindow::onCompare(bool checked)
{
    if (checked) {
        if (comparisonView == NULL) {
            comparisonView= new(std::nothrow) ComparisonDialog(guiHelper, this);
            if (comparisonView == NULL) {
                qFatal("Cannot allocate memory for ComparisonDialog X{");
                return;
            }
            comparisonView->attachAction(ui->actionCompare);
            comparisonView->adjustSize();
        }
        comparisonView->show();
        comparisonView->raise();
    } else if (comparisonView != NULL) {
        comparisonView->hide();
    }
}

void MainWindow::onAnalyse(bool checked)
{
    if (checked) {
        if (!analyseDialog) {
            analyseDialog = new(std::nothrow) AnalyseDialog(guiHelper,this);
            if (analyseDialog == NULL) {
                qFatal("Cannot allocate memory for analyseDialog X{");
                return;
            }
            analyseDialog->attachAction(ui->action_Analyse);
        }

        analyseDialog->show();
        analyseDialog->raise();
    } else if (analyseDialog != NULL) {
        analyseDialog->hide();
    }
}

void MainWindow::on_actionLogs_triggered()
{
    mainTabs->showLogs();
}
