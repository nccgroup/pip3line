/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TRANSFORMSGUI_H
#define TRANSFORMSGUI_H

#include <QWidget>
#include <transformmgmt.h>
#include "massprocessingdialog.h"
#include "transformwidget.h"
#include "loggerwidget.h"
#include "guihelper.h"
#include "transformchain.h"
#include "sources/bytesourceabstract.h"
#include "views/byteitemmodel.h"

namespace Ui {
class TransformsGui;
}

class MassProcessingDialog;
class TransformWidget;

class TransformsGui : public QWidget
{
        Q_OBJECT
        
    public:
        explicit TransformsGui(GuiHelper *guiHelper , QWidget *parent = 0);
        ~TransformsGui();
        QString getCurrentChainConf();
        void setCurrentChainConf(const QString &conf);
        void setData(const QByteArray &data);
        void setName(const QString & name);
        QString getName() const ;
        void bringFront();
        void loadFromFile(QString fileName);
        int getBlockCount() const;
        ByteItemModel * getBytesModelFrom(int blockIndex);

    signals:
        void nameChanged(TransformsGui *tab);
        void chainChanged(QString newConf);
        void askWindowTabSwitch(TransformsGui *tab);
        void askBringFront(TransformsGui *tab);
    public slots:
        void resetAll();

    private slots:
        void processNewTransformation(TransformWidget * transformWidget);
        void processDeletionRequest(TransformWidget * transformWidget);
        void onMassProcessing();
        void onSaveState();
        void onLoadState();
        void onSaveToMemory();
        void onSavedSelected(QString name);
        void buildSavedCombo();
        void onSwitchWindowTab();
        void onTransformChanged();
        void onNameChangeRequest(QString name);

    private:
        Q_DISABLE_COPY(TransformsGui)
        TransformChain getCurrentTransformChain();
        void setCurrentTransformChain(TransformChain list);
        void addWidget(TransformWidget * transformWidget);
        Ui::TransformsGui *ui;
        MassProcessingDialog * massProcessingDialog;
        TransformMgmt *transformFactory;
        TransformWidget *firstTransformWidget;
        QList<TransformWidget *> transformWidgetList;
        LoggerWidget *logger;
        GuiHelper * guiHelper;
        QString name;
};

#endif // TRANSFORMSGUI_H
