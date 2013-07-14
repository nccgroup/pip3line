/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MASSPROCESSINGDIALOG_H
#define MASSPROCESSINGDIALOG_H

#include <QDialog>
#include <transformabstract.h>
#include <QMutex>
#include <QIODevice>
#include <QTextEdit>
#include "messagedialog.h"
#include "transformsgui.h"
#include "../tools/processor.h"
#include "../tools/tcpserver.h"
#include "../tools/pipeserver.h"
#include <transformmgmt.h>
#include <transformchain.h>
#include "screeniodevice.h"
#include "loggerwidget.h"
#include <QTimer>
#include <QSettings>
#include "guihelper.h"
#include "../tools/serverabstract.h"

class TransformsGui;
class GuiHelper;

namespace Ui {
class MassProcessingDialog;
}

class MassProcessingDialog : public QDialog
{
        Q_OBJECT
        
    public:
        explicit MassProcessingDialog(GuiHelper * helper,TransformsGui *tGui);
        ~MassProcessingDialog();
    public slots:
        void setTranformChain(const QString &chainConf);
    signals:
        void finished();
    private slots:
        void selectInputFile();
        void selectOutputFile();
        void on_processingPushButton_clicked();
        void releasingThread();
        void cleaningMem();
        void stats();
        void onInputChanged(int index);
        void onOutputChanged(int index);
        void onSeparatorChanged(QString hexSep);
        void refreshTransformConf();
        void onUseSocketForOutput(bool checked);
        void clearOnScreenOutput();
        void restartCurrentServer();
        void stopCurrentServer();
        void refreshIPs();
        void onKeepSynchronize(bool checked);

    private:
        Q_DISABLE_COPY(MassProcessingDialog)
        static const QString SETTINGS_MASS_PROCESSING_GROUP;
        enum ReadingTypes {LINES = 0, BLOCK};
        void deleteCurrentServer();
        QIODevice * getInput();
        QIODevice * getOutput();

        Ui::MassProcessingDialog *ui;
        QString transformConf;
        Processor * processor;
        MessageDialog errorDialog;
        QMutex threadMutex;
        QIODevice *currentInput;
        QIODevice *currentOutput;
        TransformMgmt * transformFactory;
        QByteArray tempManualInput;
        TransformsGui * tGui;
        LoggerWidget * logger;
        QTimer statTimer;
        GuiHelper *guiHelper;
        ServerAbstract *server;
};

#endif // MASSPROCESSINGDIALOG_H
