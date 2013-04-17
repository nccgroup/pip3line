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
using namespace Pip3lineConst;

const QString MassProcessingDialog::SETTINGS_MASS_PROCESSING_GROUP = "MassProcessing";

MassProcessingDialog::MassProcessingDialog(GuiHelper *helper, TransformsGui *ntGui) :
    QDialog(ntGui),
    ui(new Ui::MassProcessingDialog),
    statTimer(this)
{
    guiHelper = helper;
    ui->setupUi(this);
    transformFactory = guiHelper->getTransformFactory();
    processor = 0;
    currentInput = 0;
    currentOutput = 0;
    tGui = ntGui;
    logger = guiHelper->getLogger();


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

    server = 0;

    ui->keepSynchronizedCheckBox->setChecked(true);
    ui->refreshConfPushButton->setDisabled(true);

    ui->restartPushButton->setEnabled(false);
    ui->stopPushButton->setEnabled(false);

    connect(&statTimer, SIGNAL(timeout()), this, SLOT(stats()));
    connect(ui->inputComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onInputChanged(int)));
    connect(ui->outputComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onOutputChanged(int)));
    connect(ui->refreshIPsPushButton, SIGNAL(clicked()), this, SLOT(refreshIPs()));
    connect(ui->clearPushButton, SIGNAL(clicked()), this, SLOT(clearOnScreenOutput()));
    connect(ui->inputFilePushButton, SIGNAL(clicked()), this, SLOT(selectInputFile()));
    connect(ui->outputFileButton, SIGNAL(clicked()), this, SLOT(selectOutputFile()));
    connect(ui->stopPushButton, SIGNAL(clicked()), this, SLOT(stopCurrentServer()));
    connect(ui->restartPushButton, SIGNAL(clicked()), this, SLOT(restartCurrentServer()));
    connect(ui->separatorLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onSeparatorChanged(QString)));
    connect(ui->useSocketForOutputcheckBox, SIGNAL(toggled(bool)), this, SLOT(onUseSocketForOutput(bool)));
    connect(ui->keepSynchronizedCheckBox, SIGNAL(toggled(bool)), this, SLOT(onKeepSynchronize(bool)));
    connect(ui->refreshConfPushButton, SIGNAL(clicked()), this, SLOT(refreshTransformConf()));

    connect(tGui, SIGNAL(chainChanged(QString)), this, SLOT(setTranformChain(QString)));
}

MassProcessingDialog::~MassProcessingDialog()
{
    delete ui;
    cleaningMem();
}

void MassProcessingDialog::setTranformChain(const QString &chainConf)
{
    transformConf = chainConf;
    if (server != 0)
        server->setTransformations(transformConf);
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

    QIODevice * output = 0;
    if (ui->inputComboBox->currentIndex() == 0 || ui->inputComboBox->currentIndex() == 1) { // i.e. file or manual input
        QIODevice * input = 0;
        ui->inputStackedWidget->setEnabled(false);

        if (processor != 0) {
            QMessageBox::critical(this,tr("Error"),tr("Already processing."),QMessageBox::Ok);
            return;
        }

        input  = getInput();
        if (input == 0) {
            massMutex.unlock();
            releasingThread();
            return;
        }

        output = getOutput();

        if (output == 0) {
            delete input;
            massMutex.unlock();
            releasingThread();
            return;
        }

        errorDialog.clearMessages();
        processor = new TextProcessor(transformFactory);
        processor->setInput(input);
        processor->setOutput(output);
        processor->setTransformsChain(transformConf);
        processor->setDecoding(ui->decodeCheckBox->isChecked());
        processor->setEncoding(ui->encodeCheckBox->isChecked());

        connect(processor, SIGNAL(error(QString,QString)), logger, SLOT(logError(QString,QString)));
        connect(processor, SIGNAL(status(QString,QString)), logger, SLOT(logStatus(QString,QString)));
        connect(processor,SIGNAL(finished()),this,SLOT(releasingThread()));

        processor->start();
    } else { // Servers

        if (!ui->useSocketForOutputcheckBox->isChecked()) {
            output = getOutput();
            if (output == 0) {
                massMutex.unlock();
                releasingThread();
                return;
            }
        }
        deleteCurrentServer();
        if (ui->serverTypeComboBox->currentText() == TCP_SERVER) {
            TcpServer * tcpServer = new TcpServer(transformFactory,this);
            tcpServer->setIP(ui->ipsComboBox->currentText());
            tcpServer->setPort(ui->portSpinBox->value());
            connect(ui->portSpinBox, SIGNAL(valueChanged(int)), tcpServer,SLOT(setPort(int)));
            connect(ui->ipsComboBox, SIGNAL(currentIndexChanged(QString)), tcpServer, SLOT(setIP(QString)));

            server = tcpServer;
        } else if (ui->serverTypeComboBox->currentText() == PIPE_SERVER) {
            PipeServer * pipeServer = new PipeServer(transformFactory,this);
            pipeServer->setPipeName(ui->pipeNameLineEdit->text());
            connect(ui->pipeNameLineEdit, SIGNAL(textChanged(QString)), pipeServer, SLOT(setPipeName(QString)));
            server = pipeServer;
        }

        connect(server, SIGNAL(error(QString,QString)), logger,SLOT(logError(QString,QString)));
        connect(server, SIGNAL(status(QString,QString)), logger,SLOT(logStatus(QString,QString)));
        connect(ui->encodeCheckBox, SIGNAL(toggled(bool)), server,SLOT(setEncoding(bool)));
        connect(ui->decodeCheckBox, SIGNAL(toggled(bool)), server,SLOT(setDecoding(bool)));

        server->setOutput(output);
        server->setTransformations(tGui->getCurrentChainConf());
        server->setDecoding(ui->decodeCheckBox->isChecked());
        server->setEncoding(ui->encodeCheckBox->isChecked());
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

    }
    statTimer.start(1000);
}


void MassProcessingDialog::deleteCurrentServer()
{
    if (server!= 0) {
        server->stopServer();
        delete server;
        server = 0;
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
    if (processor != 0) {
        delete processor;
        processor = 0;
    }
    if (currentInput != 0) {
        delete currentInput;
        currentInput = 0;
    }
    if (currentOutput != 0) {
        delete currentOutput;
        currentOutput = 0;
    }
    deleteCurrentServer();
}


void MassProcessingDialog::stats()
{
    QMutexLocker locked(&threadMutex);
    if (processor != 0)
        ui->statsLabel->setText(QString("Output blocks written: %1").arg(processor->getStatsOut()));
    else if (server != 0)
        ui->statsLabel->setText(QString("Output blocks written: %1").arg(server->getStatsOut()));

}

QIODevice *MassProcessingDialog::getInput()
{
    QIODevice * input = 0;
    QBuffer *buffer = 0;

    switch (ui->inputComboBox->currentIndex())
    {
        case 0:
            if (ui->inputFileLineEdit->text().isEmpty())
            {
                QMessageBox::critical(this,tr("Error"),tr("No input file defined"),QMessageBox::Ok);
                return 0;
            }

            input = new QFile(ui->inputFileLineEdit->text());
            if (!input->open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QMessageBox::critical(this,tr("Error"),tr("Error while opening \"%1\" for reading:\n%2").arg(ui->inputFileLineEdit->text()).arg(input->errorString()),QMessageBox::Ok);
                delete input;
                input = 0 ;
            }
            break;
        case 1:
            tempManualInput = ui->manuaInputPlainTextEdit->toPlainText().toUtf8();
            if (tempManualInput.isEmpty()) {
                break;
            }
            buffer = new QBuffer(&tempManualInput);
            buffer->open(QBuffer::ReadOnly);
            input = buffer;
            break;
        default:
            break;
    }

    if (!currentInput == 0) {
        delete currentInput;
    }
    currentInput = input;
    return input;
}

QIODevice *MassProcessingDialog::getOutput()
{
    QIODevice * output = 0;
    QIODevice::OpenMode writingMode;
    switch (ui->outputComboBox->currentIndex())
    {
        case 0:
            if (ui->outputFileLineEdit->text().isEmpty()){
                QMessageBox::critical(this,tr("Error"),tr("No output file defined"),QMessageBox::Ok);
                return 0;
            }

            output = new QFile(ui->outputFileLineEdit->text());

            if (ui->noOverWriteCheckBox->isChecked())
                writingMode = QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append;
            else
                writingMode = QIODevice::WriteOnly | QIODevice::Text;

            if (!output->open(writingMode)) {
                QMessageBox::critical(this,tr("Error"),tr("Error while opening \"%1\" for writing:\n%2").arg(ui->outputFileLineEdit->text()).arg(output->errorString()),QMessageBox::Ok);
                delete output;
                output = 0;
            }
            break;
        case 1:
            output = new ScreenIODevice(ui->outputPlainTextEdit);
            output->open(QIODevice::WriteOnly);
            break;
        default:
            break;
    }
    if (!currentOutput == 0) {
        delete currentOutput;
    }
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
        connect(tGui, SIGNAL(chainChanged(QString)), this, SLOT(setTranformChain(QString)), Qt::UniqueConnection);
        ui->refreshConfPushButton->setEnabled(false);
    } else {
        disconnect(tGui, SIGNAL(chainChanged(QString)), this, SLOT(setTranformChain(QString)));
        ui->refreshConfPushButton->setEnabled(true);
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
    if (server != 0  && !hexSep.isEmpty()) {
        QByteArray sep = QByteArray::fromHex(hexSep.toUtf8());
        if (sep.size() > 0)
            server->setSeparator(sep.at(0));
    }
}

void MassProcessingDialog::refreshTransformConf()
{
    if (server != 0) {
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
    if (server != 0) {
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
    if (server != 0)  {
        server->stopServer();
    }

    releasingThread();
}

