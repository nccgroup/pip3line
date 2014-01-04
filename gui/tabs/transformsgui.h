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
#include "transformchain.h"
#include "tababstract.h"

namespace Ui {
class TransformsGui;
}

class MassProcessingDialog;
class TransformWidget;
class TransformWidget;
class ByteSourceAbstract;
class DetachTabButton;
class ByteTableView;

class TransformsGui : public TabAbstract
{
        Q_OBJECT
        
    public:
        explicit TransformsGui(GuiHelper *guiHelper , QWidget *parent = 0);
        ~TransformsGui();
        QString getCurrentChainConf();
        void setCurrentChainConf(const QString &conf);
        void setData(const QByteArray &data);
        void loadFromFile(QString fileName);
        int getBlockCount() const;
        ByteSourceAbstract *getSource(int blockIndex);
        ByteTableView *getHexTableView(int blockIndex);
    signals:
        void chainChanged(QString newConf);

    private slots:
        void processNewTransformation(TransformWidget * transformWidget);
        void processDeletionRequest(TransformWidget * transformWidget);
        void onMassProcessing();
        void onSaveState();
        void onLoadState();
        void onSaveToMemory();
        void onSavedSelected(QString name);
        void buildSavedCombo();
        void onTransformChanged();
        void onNameChangeRequest(QString name);
        void resetAll();

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
        DetachTabButton *detachButton;
};

#endif // TRANSFORMSGUI_H
