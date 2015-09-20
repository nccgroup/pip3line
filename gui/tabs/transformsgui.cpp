/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "transformsgui.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "../sources/bytesourceabstract.h"
#include "../textinputdialog.h"
#include "../loggerwidget.h"
#include "../guihelper.h"
#include "../massprocessingdialog.h"
#include "../transformwidget.h"
#include "ui_transformsgui.h"
#include "../shared/detachtabbutton.h"
#include "../shared/universalreceiverbutton.h"
#include "../state/closingstate.h"
#include "../views/foldedview.h"
#include <QVBoxLayout>

TransformsGui::TransformsGui(GuiHelper *guiHelper, QWidget *parent) :
    TabAbstract(guiHelper,parent)
{
    ui = new(std::nothrow) Ui::TransformsGui();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::TransformsGui X{");
    }

    transformFactory = guiHelper->getTransformFactory();
    ui->setupUi(this);

    spacer = new(std::nothrow) QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    if (spacer == NULL) {
        qFatal("Cannot allocate memory for QSpacerItem X{");
    }

    firstTransformWidget = NULL;
    massProcessingDialog = NULL;

    // need to be before the transformwidget initialization
    ui->autoCopyLastPushButton->setChecked(guiHelper->isAutoCopyTextTransformGui());

    firstTransformWidget = new(std::nothrow) TransformWidget(guiHelper, this);

    if (firstTransformWidget == NULL) {
        qFatal("Cannot allocate memory for firstTransformWidget X{");
    }

    addWidget(firstTransformWidget);

    detachButton = new(std::nothrow) DetachTabButton(this);
    if (detachButton == NULL) {
        qFatal("Cannot allocate memory for detachButton X{");
    }

    ui->toolbarLayout->insertWidget(ui->toolbarLayout->indexOf(ui->massProcessingPushButton) + 1,detachButton);

    UniversalReceiverButton *urb = new(std::nothrow) UniversalReceiverButton(this, guiHelper);
    if (urb == NULL) {
        qFatal("Cannot allocate memory for UniversalReceiverButton X{");
    }

    ui->toolbarLayout->insertWidget(ui->toolbarLayout->indexOf(ui->registerPushButton),urb);

    ui->savedComboBox->installEventFilter(guiHelper);
    buildSavedCombo();



    connect(ui->savedComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(onSavedSelected(QString)));
    connect(ui->loadPushButton, SIGNAL(clicked()), SLOT(onLoadState()));
    connect(ui->savePushButton, SIGNAL(clicked()), SLOT(onSaveState()));
    connect(ui->resetPushButton, SIGNAL(clicked()), SLOT(resetAll()));
    connect(ui->registerPushButton,SIGNAL(clicked()), SLOT(onRegisterChain()));
    connect(ui->massProcessingPushButton, SIGNAL(clicked()), SLOT(onMassProcessing()));
    connect(transformFactory, SIGNAL(savedUpdated()), SLOT(buildSavedCombo()));
    connect(ui->autoCopyLastPushButton, SIGNAL(toggled(bool)), this, SLOT(onAutoCopychanged(bool)));
}

TransformsGui::~TransformsGui()
{
 //   qDebug() << "Destroying " << this;
    delete massProcessingDialog;

    while (transformWidgetList.size() > 0) {
        delete transformWidgetList.takeLast();
    }
    //delete spacer;
    delete detachButton;
    logger = NULL;
    guiHelper = NULL;
    delete ui;
}

QString TransformsGui::getCurrentChainConf()
{
//    this->setEnabled(false);

    TransformChain list = getCurrentTransformChain();
    QString ret;
    QXmlStreamWriter streamin(&ret);

    transformFactory->saveConfToXML(list, &streamin);

//    this->setEnabled(true);

    return ret;
}

void TransformsGui::setCurrentChainConf(const QString &conf, bool ignoreErrors)
{
    if (conf.isEmpty())
        return;

    MessageDialog errorDialog(guiHelper);
    connect(transformFactory, SIGNAL(error(QString,QString)), &errorDialog, SLOT(logError(QString)));
    QXmlStreamReader reader(conf);

    TransformChain talist = transformFactory->loadConfFromXML(&reader);
    if (talist.isEmpty()) {
        if (!ignoreErrors) {
            QMessageBox::critical(this,tr("Error"),tr("The loaded chain is empty. Check the logs."),QMessageBox::Ok);
        }
    }
    else {
        if (errorDialog.hasMessages() && !ignoreErrors) {
            errorDialog.setWindowTitle(tr("Error(s) while loading the configuration"));
            if (errorDialog.exec() == QDialog::Rejected) {
                while (!talist.isEmpty())
                    delete talist.takeLast();
                return;
            }
        }

        setCurrentTransformChain(talist);
        emit chainChanged(conf);
    }

}

TransformChain TransformsGui::getCurrentTransformChain()
{
   // this->setEnabled(false);

    TransformChain list;
    for (int i = 0; i < transformWidgetList.size() - 1; i++) {
        if (transformWidgetList.at(i)->getTransform() != 0 )
            list.append(transformWidgetList.at(i)->getTransform());
    }
    list.setName(name);

 //   this->setEnabled(true);
    return list;
}


void TransformsGui::setCurrentTransformChain(TransformChain talist)
{
    name = talist.getName();
    emit nameChanged();
    // At this point nothing should go wrong, we can clean.

    while (transformWidgetList.size() > 0) {
        delete transformWidgetList.takeLast();
    }

    QList<TransformWidget *> widgetList;
    TransformWidget *twa = NULL;
    for (int i = 0; i < talist.size(); i++) {
        twa = new(std::nothrow) TransformWidget(guiHelper, this);
        if (twa == NULL) {
            qFatal("Cannot allocate memory for TransformWidget X{");
            return;
        } else {
            twa->setTransform(talist.at(i));
            widgetList.append(twa);
        }
    }
    twa = new(std::nothrow) TransformWidget(guiHelper, this);
    if (twa == NULL) {
        qFatal("Cannot allocate memory for TransformWidget X{");
        return;
    }

    widgetList.append(twa);

    // setting the first transform widget
    firstTransformWidget = widgetList.at(0);

    // adding the rest
    for (int i = 0; i < widgetList.size(); i++) {
        addWidget(widgetList.at(i));
    }
}

void TransformsGui::setData(const QByteArray &data)
{
    firstTransformWidget->input(data);
}

void TransformsGui::loadFromFile(QString fileName)
{
    if (transformWidgetList.size() > 0) {
        TransformWidget * tw = transformWidgetList.at(0);
        tw->fromLocalFile(fileName);
    }
}

int TransformsGui::getBlockCount() const
{
    return transformWidgetList.size();
}

ByteSourceAbstract *TransformsGui::getSource(int blockIndex)
{
    if (blockIndex < 0 || blockIndex >= transformWidgetList.size()) {
        logger->logError(tr("[TransformsGui::getSource] index out-of-bounds: %1").arg(blockIndex));
        return NULL;
    }

    return transformWidgetList.at(blockIndex)->getSource();
}

ByteTableView *TransformsGui::getHexTableView(int blockIndex)
{
    if (blockIndex < 0 || blockIndex >= transformWidgetList.size()) {
        logger->logError(tr("[TransformsGui::getHexTableView] index out-of-bounds %1").arg(blockIndex));
        return NULL;
    }

    return transformWidgetList.at(blockIndex)->getHexTableView();
}

BaseStateAbstract *TransformsGui::getStateMngtObj()
{
    TransformGuiStateObj *stateObj = new(std::nothrow) TransformGuiStateObj(this);
    if (stateObj == NULL) {
        qFatal("Cannot allocate memory for TransformGuiStateObj X{");
    }
    return stateObj;
}


void TransformsGui::processNewTransformation(TransformWidget *transformWidget)
{
    int pos;
    TransformWidget * ntw = new(std::nothrow) TransformWidget(guiHelper, this);
    if (ntw == NULL) {
        qFatal("Cannot allocate memory for TransformWidget ntw X{");
        return;
    }

    pos = transformWidgetList.indexOf(transformWidget);

    // cleaning transformWidget located after the modified widget
    while (transformWidgetList.size() > pos + 1)
        delete transformWidgetList.takeLast();

    //ntw->input(transformWidget->output()); // testing
    addWidget(ntw);

}

void TransformsGui::processDeletionRequest(TransformWidget *transformWidget)
{
    if (transformWidgetList.size() < 2) {
        firstTransformWidget->reset();
        return;
    }

    int i = transformWidgetList.indexOf(transformWidget);
    if (i == -1) {
        QMessageBox::critical(this,tr("Radioactive error"), tr("Widget object not found T_T"),QMessageBox::Ok);
        qWarning() << tr("[TransformsGui] Widget object not found T_T");
        return;
    }

    if (i == 0) {
        firstTransformWidget = transformWidgetList.at(i + 1);
        transformWidgetList.takeAt(i);
        delete transformWidget;
    } else if (i < transformWidgetList.size() - 1) {
        TransformWidget *prev = transformWidgetList.at(i - 1);
        TransformWidget *suiv = transformWidgetList.at(i + 1);
        transformWidgetList.takeAt(i);
        delete transformWidget;
        connect(prev,SIGNAL(updated()),suiv,SLOT(updatingFrom()));
        prev->forceUpdating();
    }

    emit entriesChanged();
}

void TransformsGui::onMassProcessing()
{
    if (transformWidgetList.size() == 1) {
        QMessageBox::critical(this,tr("Nothing to be processed"), tr("Please, select a transformation before."),QMessageBox::Ok);
        return;
    }
    if (massProcessingDialog == NULL) {
        massProcessingDialog = new(std::nothrow) MassProcessingDialog(guiHelper, this);
        if (massProcessingDialog == NULL) {
            qFatal("Cannot allocate memory for MassProcessingDialog X{");
            return;
        }
        massProcessingDialog->setTranformChain(getCurrentChainConf());
        massProcessingDialog->setWindowTitle(tr("Mass processing for %1").arg(name));
    }

    massProcessingDialog->show();
}

void TransformsGui::addWidget(TransformWidget *transformWidget)
{
    if (!transformWidgetList.isEmpty()) { // only if there is already another TransformWidget
        TransformWidget *previousTw = transformWidgetList.last();
        previousTw->setAutoCopyTextToClipboard(false);
        connect(previousTw,SIGNAL(updated()),transformWidget,SLOT(updatingFrom()));
    }

    transformWidget->setAutoCopyTextToClipboard(ui->autoCopyLastPushButton->isChecked());
    transformWidgetList.append(transformWidget); // updating the list

    // Adding the widget to the gui
    ui->scrollAreaWidgetContents->layout()->addWidget(transformWidget);
    connect(transformWidget, SIGNAL(error(QString,QString)),logger, SLOT(logError(QString,QString)));
    connect(transformWidget, SIGNAL(warning(QString,QString)),logger, SLOT(logWarning(QString,QString)));
    connect(transformWidget, SIGNAL(status(QString,QString)),logger, SLOT(logStatus(QString,QString)));
    connect(transformWidget,SIGNAL(transfoRequest(TransformWidget*)),this,SLOT(processNewTransformation(TransformWidget*)));
    connect(transformWidget,SIGNAL(deletionRequest(TransformWidget*)), this, SLOT(processDeletionRequest(TransformWidget*)));
    connect(transformWidget, SIGNAL(transformChanged()), this, SLOT(onTransformChanged()),Qt::QueuedConnection);
    connect(transformWidget, SIGNAL(tryNewName(QString)), this, SLOT(onNameChangeRequest(QString)));
    connect(transformWidget, SIGNAL(foldRequest()), this, SLOT(onFoldRequest()));

    emit entriesChanged();
}

void TransformsGui::onSaveState()
{
    if (transformWidgetList.size() == 1) {
        QMessageBox::critical(this,tr("Error"),tr("No transformation selected, nothing to save!"),QMessageBox::Ok);
        return;
    }

    MessageDialog errorDialog(guiHelper);
    errorDialog.setJustShowMessages(true);
    connect(transformFactory, SIGNAL(error(QString,QString)), &errorDialog, SLOT(logError(QString)));

    TransformChain transformList = getCurrentTransformChain();

    QString fileName = QFileDialog::getSaveFileName(this,tr("Choose file to save"), "",tr("XML documents (*.xml);; All (*)"));

    if (fileName.isEmpty())
        return;


    transformFactory->saveConfToFile(fileName,transformList);

    if (errorDialog.hasMessages()) {
        errorDialog.setWindowTitle(tr("Error(s) while saving the state file:"));
        errorDialog.exec();
    } else {
        logger->logStatus(tr("Saved transformation chain to %1").arg(fileName));
    }
}

void TransformsGui::onLoadState()
{
    MessageDialog errorDialog(guiHelper);
    connect(transformFactory, SIGNAL(error(QString,QString)), &errorDialog, SLOT(logError(QString)));

    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose state file to load from"),"",tr("XML documents (*.xml);; All (*)"));

    if (fileName.isEmpty())
        return;

    TransformChain talist = transformFactory->loadConfFromFile(fileName);
    if (talist.isEmpty())
        QMessageBox::critical(this,tr("Error"),tr("The loaded chain is empty. Check the logs."),QMessageBox::Ok);
    else {
        if (errorDialog.hasMessages()) {
            errorDialog.setWindowTitle(tr("Error(s) while loading the state file"));
            if (errorDialog.exec() == QDialog::Rejected) {
                while (!talist.isEmpty())
                    delete talist.takeLast();
                return;
            }
        } else {
            logger->logStatus(tr("File %1 loaded").arg(fileName));
        }

        setCurrentTransformChain(talist);
        // not really efficient
        QString conf;
        QXmlStreamWriter writer(&conf);
        transformFactory->saveConfToXML(talist, &writer);
        emit chainChanged(conf);
    }
}

void TransformsGui::onRegisterChain()
{
    if (transformWidgetList.size() == 1) {
        QMessageBox::critical(this,tr("Error"),tr("No transformation selected, nothing to register!"),QMessageBox::Ok);
    } else {

        TextInputDialog * dia = guiHelper->getNameDialog(this, name);
        if (dia != NULL) {
            int ret = dia->exec();
            if (ret == QDialog::Accepted) {
                QString newName = dia->getInputText();
                if (newName.isEmpty())
                    newName = name;
                setName(newName);

                transformFactory->registerChainConf(getCurrentTransformChain(),true);

            }
            delete dia;
        }
    }
}

void TransformsGui::onSavedSelected(QString name)
{
    if (name.isEmpty()) {
        QMessageBox::critical(this,tr("Error"),tr("The selected name is empty T_T"),QMessageBox::Ok);
        qWarning() << tr("[TransformsGui] The selected name is empty T_T");
        return;
    }
    TransformChain tc = transformFactory->loadChainFromSaved(name);
    if (tc.isEmpty()) {
        QMessageBox::critical(this,tr("Error"),tr("The returned chain is empty. Check the logs."),QMessageBox::Ok);
    } else {
        resetAll();
        setCurrentTransformChain(tc);
        emit chainChanged(transformFactory->getSavedConfs().value(name));
    }
}

void TransformsGui::buildSavedCombo()
{
    ui->savedComboBox->blockSignals(true);
    ui->savedComboBox->clear();
    int row = 0;
    // first inactive element
    ui->savedComboBox->addItem(QString("User's chains"));
    QStandardItem * item = qobject_cast<QStandardItemModel *>(ui->savedComboBox->model())->item( row );
    item->setEnabled( false );
    item->setTextAlignment(Qt::AlignCenter);
    item->setBackground(Qt::darkGray);
    item->setForeground(Qt::white);
    // then the rest
    QHash<QString, QString> hash = transformFactory->getSavedConfs();
    QStringList list = hash.keys();
    if (list.isEmpty()) {
        ui->savedComboBox->setEnabled(false);
    } else {
        ui->savedComboBox->addItems(list);
        ui->savedComboBox->setEnabled(true);
    }

    ui->savedComboBox->blockSignals(false);
}

void TransformsGui::resetAll()
{
    while (transformWidgetList.size() > 1) { // we want to keep the first one
        delete transformWidgetList.takeLast();
    }

    // just reset first one
    firstTransformWidget->reset();

    ui->savedComboBox->blockSignals(true);
    ui->savedComboBox->setCurrentIndex(0);
    ui->savedComboBox->blockSignals(false);
}

void TransformsGui::onAutoCopychanged(bool val)
{
    if (!transformWidgetList.isEmpty()) { // only if there is a TransformWidget
        TransformWidget *last = transformWidgetList.last();
        last->setAutoCopyTextToClipboard(val);
    } else {
        qCritical() << tr("No TransformWidget in the list T_T");
    }
}

void TransformsGui::onFoldRequest()
{
    TransformWidget * requester = static_cast<TransformWidget *>(sender());
    if (requester == NULL) {
        qFatal("Cannot cast sender() to TransformWidget X{");
    }

    int index = ui->mainLayout->indexOf(requester);
    if (index == -1) {
        logger->logError(tr("Invalid index when folding T_T"), metaObject()->className());
    } else {
        ui->mainLayout->removeWidget(requester);
        requester->hide();
        requester->setFolded(true);
        FoldedView *fv = new(std::nothrow) FoldedView(requester,this);
        if (fv == NULL) {
            qFatal("Cannot allocate memory for FoldedWidget X{");
        }

        connect(fv, SIGNAL(unfoldRequested()), this, SLOT(onUnfoldRequest()));

        ui->mainLayout->insertWidget(index,fv,0, Qt::AlignTop);
        bool allFolded = true;
        for (int i = 0; i < transformWidgetList.size(); i++) {
            allFolded = allFolded && transformWidgetList.at(i)->isFolded();
        }

        if (allFolded) {
            ui->mainLayout->addSpacerItem(spacer);
        }
    }
}

void TransformsGui::onUnfoldRequest()
{
    FoldedView *fv = static_cast<FoldedView *>(sender());
    if (fv == NULL) {
        qFatal("Cannot cast sender() to FoldedWidget X{");
    }

    int index = ui->mainLayout->indexOf(fv);
    if (index == -1) {
        logger->logError(tr("Invalid index when unfolding T_T"), metaObject()->className());
    } else {
        TransformWidget * tw = fv->getTransformWidget();
        ui->mainLayout->removeWidget(fv);
        delete fv;
        ui->mainLayout->removeItem(spacer);
        ui->mainLayout->insertWidget(index,tw);
        tw->setFolded(false);
        tw->show();
    }
}

void TransformsGui::onTransformChanged()
{
    emit chainChanged(getCurrentChainConf());
}

void TransformsGui::onNameChangeRequest(QString name)
{
    QObject * obj = sender();

    if (obj == firstTransformWidget  && !name.isEmpty()) {
        setName(name);
    }
}



TransformGuiStateObj::TransformGuiStateObj(TransformsGui *tg) :
    TabStateObj(tg)
{
    name = tg->metaObject()->className();
}

TransformGuiStateObj::~TransformGuiStateObj()
{

}

void TransformGuiStateObj::run()
{
    TabStateObj::run();
    QString conf;
    int size = 0;

    TransformsGui *tgtab = dynamic_cast<TransformsGui *>(tab);

    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        conf = tgtab->getCurrentChainConf();

        writer->writeAttribute(GuiConst::STATE_TRANSFORM_CONF, write(conf,true));
        size = tgtab->getBlockCount();

        // first saving the transformWidget states so that they are saved last.
        for (int i = size - 1  ; i > -1; i--) { // need to reverse the order due to the stack behaviour
            BaseStateAbstract *state = tgtab->transformWidgetList.at(i)->getStateMngtObj();
            emit addNewState(state);
        }

        writer->writeAttribute(GuiConst::STATE_SIZE, write(size));
        for (int i = size - 1  ; i > -1; i--) { // need to reverse the order due to the stack behaviour
            BaseStateAbstract *state = tgtab->getSource(i)->getStateMngtObj();
            emit addNewState(state);
        }


    } else {
        QXmlStreamAttributes attrList = reader->attributes();
        conf = readString(attrList.value(GuiConst::STATE_TRANSFORM_CONF));

        if (!conf.isEmpty()) {
            tgtab->setCurrentChainConf(conf, true);
        }
        int bsize = tgtab->getBlockCount();
        bool ok = false;
        size = readInt(attrList.value(GuiConst::STATE_SIZE),&ok);
        if (!ok) {
            emit log(tr("Error while parsing the number of blocks from the saved state, stopping restore."), tr("TransformGuiLoader"), Pip3lineConst::LERROR);
            return;
        }

        if (bsize != size) {
            emit log(tr("The number of transform blocks (%1) is different from the array size. Taking the smaller number."), tr("TransformGuiLoader"), Pip3lineConst::LWARNING);
            size = qMin(bsize, size);
        }

        // first restoring the transformWidget states (so that they are executed last)
        for (int i = size - 1  ; i > -1; i--) { // need to reverse the order due to the stack behaviour
            BaseStateAbstract *state = tgtab->transformWidgetList.at(i)->getStateMngtObj();
            emit addNewState(state);
        }

        // then the data (executed first)
        for (int i = size - 1  ; i > -1; i--) { // need to reverse the order due to the stack behaviour
            BaseStateAbstract *tempState = tgtab->getSource(i)->getStateMngtObj();
            emit addNewState(tempState);
        }
    }
}
