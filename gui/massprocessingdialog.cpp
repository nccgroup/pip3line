/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "massprocessingdialog.h"
#include "ui_massprocessingdialog.h"
#include "../tools/textprocessor.h"
#include "../tools/binaryprocessor.h"
#include "tabs/transformsgui.h"
#include "../tools/processor.h"
#include "../tools/tcpserver.h"
#include "../tools/pipeserver.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QMutexLocker>
#include <QFileInfo>
#include <QThreadPool>
#include <QIODevice>
#include <QDebug>
#include <QHostAddress>
#include <QBuffer>
#include <QNetworkInterface>
#include <commonstrings.h>
#include <transformabstract.h>
#include <transformmgmt.h>
#include "guihelper.h"
#include "../tools/serverabstract.h"
#include "screeniodevice.h"
#include <QIODevice>
#include "shared/guiconst.h"
using namespace GuiConst;

MassProcessingDialog::MassProcessingDialog(GuiHelper *helper, TransformsGui *ntGui) :
    AppDialog(helper, ntGui),
    errorDialog(helper),
    statTimer(this)
{
    ui = new(std::nothrow) Ui::MassProcessingDialog();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::MassProcessingDialog X{");
    }
    ui->setupUi(this);
    transformFactory = guiHelper->getTransformFactory();
    processor = NULL;
    currentInput = NULL;
    currentOutput = NULL;
    tGui = ntGui;

    errorDialog.setJustShowMessages(true);
    errorDialog.setWindowTitle(tr("Errors found while processing"));
    refreshIPs();

    ui->serverTypeComboBox->addItem(TCP_SERVER);
    ui->serverTypeComboBox->addItem(PIPE_SERVER);
    ui->localSocketGroupBox->setTitle(PIPE_SERVER);

    ui->portSpinBox->setValue(guiHelper->getDefaultPort());
    ui->decodeCheckBox->setChecked(guiHelper->getDefaultServerDecode());
    ui->encodeCheckBox->setChecked(guiHelper->getDefaultServerEncode());
    ui->separatorLineEdit->setText(QString::fromUtf8(QByteArray(1,guiHelper->getDefaultServerSeparator()).toHex()));
    ui->pipeNameLineEdit->setText(guiHelper->getDefaultServerPipeName());

    onInputChanged(ui->inputComboBox->currentIndex());

    server = NULL;

    ui->keepSynchronizedCheckBox->setChecked(true);
    ui->refreshConfPushButton->setDisabled(true);

    ui->restartPushButton->setEnabled(false);
    ui->stopPushButton->setEnabled(false);



    connect(&statTimer, SIGNAL(timeout()), SLOT(stats()));
    connect(ui->inputComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onInputChanged(int)));
    connect(ui->outputComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onOutputChanged(int)));
    connect(ui->refreshIPsPushButton, SIGNAL(clicked()), SLOT(refreshIPs()));
    connect(ui->clearPushButton, SIGNAL(clicked()), SLOT(clearOnScreenOutput()));
    connect(ui->inputFilePushButton, SIGNAL(clicked()), SLOT(selectInputFile()));
    connect(ui->outputFileButton, SIGNAL(clicked()), SLOT(selectOutputFile()));
    connect(ui->stopPushButton, SIGNAL(clicked()), SLOT(stopCurrentServer()));
    connect(ui->restartPushButton, SIGNAL(clicked()), SLOT(restartCurrentServer()));
    connect(ui->separatorLineEdit, SIGNAL(textChanged(QString)), SLOT(onSeparatorChanged(QString)));
    connect(ui->useSocketForOutputcheckBox, SIGNAL(toggled(bool)), SLOT(onUseSocketForOutput(bool)));
    connect(ui->keepSynchronizedCheckBox, SIGNAL(toggled(bool)), SLOT(onKeepSynchronize(bool)));
    connect(ui->refreshConfPushButton, SIGNAL(clicked()), SLOT(refreshTransformConf()));
    connect(ui->logsCheckBox, SIGNAL(toggled(bool)), SLOT(onLogsEnabled(bool)));

    connect(tGui, SIGNAL(chainChanged(QString)), this, SLOT(setTranformChain(QString)), Qt::QueuedConnection);

  //  ui->outputComboBox->setCurrentIndex(0);
  //  ui->outputFileLineEdit->setText("/dev/null");
}

MassProcessingDialog::~MassProcessingDialog()
{
    logger = NULL;
    guiHelper = NULL;
    delete ui;
    cleaningMem();
}

void MassProcessingDialog::setTranformChain(const QString &chainConf)
{
    //qDebug() << "Updating transform configuration" << chainConf;
    transformConf = chainConf;
    if (server != NULL) {
        emit newServerTransformChain(transformConf);
    }
}

void MassProcessingDialog::selectInputFile()
{
    QString init = ui->inputFileLineEdit->text();
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select input file"),init);
    if (!fileName.isEmpty()) {
        ui->inputFileLineEdit->setText(fileName);
    }
}

void MassProcessingDialog::selectOutputFile()
{
    QString init = ui->outputFileLineEdit->text();
    QString fileName = QFileDialog::getSaveFileName(this,tr("Choose output file"),init);
    if (!fileName.isEmpty()) {
        ui->outputFileLineEdit->setText(fileName);
    }
}

void MassProcessingDialog::on_processingPushButton_clicked()
{
    QMutexLocker massMutex(&threadMutex);
    ui->processingPushButton->setEnabled(false);
    ui->outputComboBox->setEnabled(false);
    ui->inputComboBox->setEnabled(false);

    QIODevice * output = NULL;
    if (ui->inputComboBox->currentIndex() == 0 || ui->inputComboBox->currentIndex() == 1) { // i.e. file or manual input
        QIODevice * input = NULL;
        ui->inputStackedWidget->setEnabled(false);

        if (processor != NULL) {
            QMessageBox::critical(this,tr("Error"),tr("Already processing."),QMessageBox::Ok);
            return;
        }

        input  = getInput();
        if (input == NULL) {
            massMutex.unlock();
            releasingThread();
            return;
        }

        output = getOutput();

        if (output == NULL) {
            delete input;
            input = NULL;
            massMutex.unlock();
            releasingThread();
            return;
        }

        errorDialog.clearMessages();
        processor = new(std::nothrow) TextProcessor(transformFactory);
       // qDebug() << "new processor instanciated" << transformConf;
        if (processor != NULL) {
            processor->setInput(input);
            processor->setOutput(output);
            processor->setTransformsChain(transformConf);
            processor->setDecoding(ui->decodeCheckBox->isChecked());
            processor->setEncoding(ui->encodeCheckBox->isChecked());

            connect(processor, SIGNAL(error(QString,QString)), logger, SLOT(logError(QString,QString)),Qt::QueuedConnection);
            connect(processor, SIGNAL(status(QString,QString)), logger, SLOT(logStatus(QString,QString)),Qt::QueuedConnection);
            connect(processor,SIGNAL(finished()),this,SLOT(releasingThread()));

            processor->start();
            statTimer.start(1000);
        } else {
            qFatal("Cannot allocate memory for TextProcessor X{");
        }
    } else { // Servers

        if (!ui->useSocketForOutputcheckBox->isChecked()) {
            output = getOutput();
            if (output == NULL) {
                massMutex.unlock();
                releasingThread();
                return;
            }
        }
        deleteCurrentServer();
        if (ui->serverTypeComboBox->currentText() == TCP_SERVER) {
            TcpServer * tcpServer = new(std::nothrow) TcpServer(transformFactory,this);
            if (tcpServer != NULL) {
                tcpServer->setIP(ui->ipsComboBox->currentText());
                tcpServer->setPort(ui->portSpinBox->value());
                connect(ui->portSpinBox, SIGNAL(valueChanged(int)), tcpServer,SLOT(setPort(int)));
                connect(ui->ipsComboBox, SIGNAL(currentIndexChanged(QString)), tcpServer, SLOT(setIP(QString)));
            } else {
                qFatal("Cannot allocate memory for tcpServer X{");
            }

            server = tcpServer;
        } else if (ui->serverTypeComboBox->currentText() == PIPE_SERVER) {
            PipeServer * pipeServer = new(std::nothrow) PipeServer(transformFactory,this);
            if (pipeServer != NULL) {
                pipeServer->setPipeName(ui->pipeNameLineEdit->text());
                connect(ui->pipeNameLineEdit, SIGNAL(textChanged(QString)), pipeServer, SLOT(setPipeName(QString)));
                server = pipeServer;
            } else {
                qFatal("Cannot allocate memory for pipeServer X{");
            }
        }

        if (server != NULL) {
            connect(server, SIGNAL(error(QString,QString)), logger,SLOT(logError(QString,QString)),Qt::QueuedConnection);
            connect(server, SIGNAL(status(QString,QString)), logger,SLOT(logStatus(QString,QString)),Qt::QueuedConnection);
            connect(ui->encodeCheckBox, SIGNAL(toggled(bool)), server,SLOT(setEncoding(bool)));
            connect(ui->decodeCheckBox, SIGNAL(toggled(bool)), server,SLOT(setDecoding(bool)));
            connect(this, SIGNAL(newServerTransformChain(QString)), server,SLOT(setTransformations(QString)), Qt::QueuedConnection);

            server->setOutput(output);
            server->setTransformations(tGui->getCurrentChainConf());
            server->setDecoding(ui->decodeCheckBox->isChecked());
            server->setEncoding(ui->encodeCheckBox->isChecked());
            server->setAllowForwardingLogs(ui->logsCheckBox->isCheckable());
            onSeparatorChanged(ui->separatorLineEdit->text());

            ui->serverTypeComboBox->setEnabled(false);
            ui->useSocketForOutputcheckBox->setEnabled(false);

            if (!server->startServer()) {
                massMutex.unlock();
                QMessageBox::critical(this,tr("Error"),tr("Could not start the %1:\n%2").arg(server->getServerType()).arg(server->getLastError()),QMessageBox::Ok);
                releasingThread();
                return;
            }
            ui->restartPushButton->setEnabled(true);
            ui->stopPushButton->setEnabled(true);
            statTimer.start(1000);
        }

    }

}


void MassProcessingDialog::deleteCurrentServer()
{
    if (server!= NULL) {
        server->stopServer();
        delete server;
        server = NULL;
    }
}

void MassProcessingDialog::releasingThread() {
    statTimer.stop();
    stats();
    QMutexLocker locked(&threadMutex);
    cleaningMem();

    ui->inputComboBox->setEnabled(true);
    ui->processingPushButton->setEnabled(true);
    ui->inputStackedWidget->setEnabled(true);
    ui->outputComboBox->setEnabled(true);
    ui->serverTypeComboBox->setEnabled(true);
    ui->useSocketForOutputcheckBox->setEnabled(true);
    ui->processingPushButton->setEnabled(true);
    ui->stopPushButton->setEnabled(false);
    ui->restartPushButton->setEnabled(false);

    if (errorDialog.hasMessages() && !errorDialog.isVisible())
        errorDialog.exec();
    emit finished();
}

void MassProcessingDialog::cleaningMem()
{
    delete processor;
    processor = NULL;

    delete currentInput;
    currentInput = NULL;

    delete currentOutput;
    currentOutput = NULL;

    deleteCurrentServer();
}


void MassProcessingDialog::stats()
{
    QMutexLocker locked(&threadMutex);
    QString bstring;
    ProcessingStats stats;
    if (processor != NULL)
        stats = processor->getStats();
    else if (server != NULL)
        stats = server->getStats();

    bstring.append(tr("Blocks read: %1 Blocks written: %2").arg(stats.getInBlocks()).arg(stats.getOutBlocks()));

    if (stats.getErrorsCount() != 0)
        bstring.append(tr(" Errors: %1").arg(stats.getErrorsCount()));

    if (stats.getWarningsCount() != 0)
        bstring.append(tr(" Warnings: %1").arg(stats.getWarningsCount()));

    if (stats.getStatusCount() != 0)
        bstring.append(tr(" Info: %1").arg(stats.getStatusCount()));

    ui->statsLabel->setText(bstring);

}

QIODevice *MassProcessingDialog::getInput()
{
    QIODevice * input = NULL;
    QBuffer *buffer = NULL;

    switch (ui->inputComboBox->currentIndex())
    {
        case 0:
            if (ui->inputFileLineEdit->text().isEmpty())
            {
                QMessageBox::critical(this,tr("Error"),tr("No input file defined"),QMessageBox::Ok);
                return NULL;
            }

            input = new(std::nothrow) QFile(ui->inputFileLineEdit->text());
            if (input == NULL) {
                 qFatal("Cannot allocate memory for input QFile X{");
            } else if (!input->open(QIODevice::ReadOnly | QIODevice::Text)) {
                QMessageBox::critical(this,tr("Error"),tr("Error while opening \"%1\" for reading:\n%2").arg(ui->inputFileLineEdit->text()).arg(input->errorString()),QMessageBox::Ok);
                delete input;
                input = NULL ;
            }
            break;
        case 1:
            tempManualInput = ui->manuaInputPlainTextEdit->toPlainText().toUtf8();
            if (tempManualInput.isEmpty()) {
                break;
            }
            buffer = new(std::nothrow) QBuffer(&tempManualInput);
            if (buffer != NULL) {
                buffer->open(QBuffer::ReadOnly);
            } else {
               qFatal("Cannot allocate memory for input QBuffer X{");
            }

            input = buffer;
            break;
        default:
            break;
    }

    delete currentInput;
    currentInput = input;
    return input;
}

QIODevice *MassProcessingDialog::getOutput()
{
    QIODevice * output = NULL;
    QIODevice::OpenMode writingMode;
    switch (ui->outputComboBox->currentIndex())
    {
        case 0:
            if (ui->outputFileLineEdit->text().isEmpty()){
                QMessageBox::critical(this,tr("Error"),tr("No output file defined"),QMessageBox::Ok);
                return NULL;
            }

            output = new(std::nothrow) QFile(ui->outputFileLineEdit->text());
            if (output == NULL) {
                 qFatal("Cannot allocate memory for output QFile X{");
            } else  {
                if (ui->noOverWriteCheckBox->isChecked())
                    writingMode = QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append;
                else
                    writingMode = QIODevice::WriteOnly | QIODevice::Text;

                if (!output->open(writingMode)) {
                    QMessageBox::critical(this,tr("Error"),tr("Error while opening \"%1\" for writing:\n%2").arg(ui->outputFileLineEdit->text()).arg(output->errorString()),QMessageBox::Ok);
                    delete output;
                    output = NULL;
                }
            }
            break;
        case 1:
            output = new(std::nothrow) ScreenIODevice(ui->outputPlainTextEdit);
            if (output != NULL)
                output->open(QIODevice::WriteOnly);
            else
                qFatal("Cannot allocate memory for output ScreenIODevice X{");

            break;
        default:
            break;
    }

    delete currentOutput;
    currentOutput = output;
    return output;
}

void MassProcessingDialog::refreshIPs()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    QList<QString> slist;

    for (int i = 0; i < list.size(); i++) {
        slist.append(list.at(i).toString());
    }
    int index = slist.indexOf("127.0.0.1");

    ui->ipsComboBox->clear();
    ui->ipsComboBox->addItems(slist);
    if (index != -1)
        ui->ipsComboBox->setCurrentIndex(index);
}

void MassProcessingDialog::onKeepSynchronize(bool checked)
{
    if (checked) {
        refreshTransformConf();
        connect(tGui, SIGNAL(chainChanged(QString)), this, SLOT(setTranformChain(QString)), Qt::QueuedConnection);
        ui->refreshConfPushButton->setEnabled(false);
    } else {
        disconnect(tGui, SIGNAL(chainChanged(QString)), this, SLOT(setTranformChain(QString)));
        ui->refreshConfPushButton->setEnabled(true);
    }
}

void MassProcessingDialog::onLogsEnabled(bool checked)
{
    if (processor != NULL) {

    } else if (server != NULL) {
        server->setAllowForwardingLogs(checked);
    }
}

void MassProcessingDialog::onInputChanged(int index)
{
    switch (index) {
        case 0:
            ui->inputStackedWidget->setCurrentWidget(ui->textFileInputPage);
            ui->processingPushButton->setText(tr("Process file"));
            break;
        case 1:
            ui->inputStackedWidget->setCurrentWidget(ui->manualInputPage);
            ui->processingPushButton->setText(tr("Process input"));
            break;
        case 2:
            ui->inputStackedWidget->setCurrentWidget(ui->serverPage);
            refreshIPs();
            ui->processingPushButton->setText(tr("Start server"));
            break;
    }
}

void MassProcessingDialog::onOutputChanged(int index)
{
    switch (index) {
        case 0:
            ui->outputStackedWidget->setCurrentWidget(ui->textFileOutputPage);
            break;
        case 1:
            ui->outputStackedWidget->setCurrentWidget(ui->screenOutPage);
            break;
    }
}

void MassProcessingDialog::onSeparatorChanged(QString hexSep)
{
    if (server != NULL  && !hexSep.isEmpty()) {
        QByteArray sep = QByteArray::fromHex(hexSep.toUtf8());
        if (sep.size() > 0)
            server->setSeparator(sep.at(0));
    }
}

void MassProcessingDialog::refreshTransformConf()
{
    if (server != NULL) {
        server->setTransformations(tGui->getCurrentChainConf());
    }
}

void MassProcessingDialog::onUseSocketForOutput(bool checked)
{
    if (checked)
        ui->outputGroupBox->setEnabled(false);
    else
        ui->outputGroupBox->setEnabled(true);
}

void MassProcessingDialog::clearOnScreenOutput()
{
    ui->outputPlainTextEdit->clear();
}

void MassProcessingDialog::restartCurrentServer()
{
    ui->stopPushButton->setEnabled(false);
    ui->restartPushButton->setEnabled(false);
    if (server != NULL) {
        server->stopServer();
        if (!server->startServer()) {
            QMessageBox::critical(this,tr("Error"),tr("Could not restart the %1:\n%2").arg(server->getServerType()).arg(server->getLastError()),QMessageBox::Ok);
            releasingThread();
        } else {
            ui->stopPushButton->setEnabled(true);
            ui->restartPushButton->setEnabled(true);
        }
    }
}

void MassProcessingDialog::stopCurrentServer()
{
    ui->stopPushButton->setEnabled(false);
    ui->restartPushButton->setEnabled(false);
    if (server != NULL)  {
        server->stopServer();
    }

    releasingThread();
}

