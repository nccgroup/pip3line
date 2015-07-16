/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "generictab.h"
#include "ui_generictab.h"
#include <QDebug>
#include "../shared/offsetgotowidget.h"
#include "../shared/searchwidget.h"
#include "../sources/bytesourceabstract.h"
#include "../views/hexview.h"
#include "../views/textview.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QKeyEvent>
#include <QAction>
#include <QTabWidget>
#include <QDropEvent>
#include <QMenu>
#include <QDragEnterEvent>
#include "../quickviewitemconfig.h"
#include <QUrl>
#include "../downloadmanager.h"
#include "../loggerwidget.h"
#include "../guihelper.h"
#include "../shared/readonlybutton.h"
#include "../shared/clearallmarkingsbutton.h"
#include "../shared/bytesourceguibutton.h"
#include "../shared/detachtabbutton.h"
#include "../shared/messagepanelwidget.h"
#include "../sources/intermediatesource.h"
#include "../shared/universalreceiverbutton.h"
#include "../shared/guiconst.h"
#include "../views/bytetableview.h"
#include <QScrollBar>

using namespace GuiConst;

const QString GenericTab::TEXT_TEXT = "Text";

GenericTab::GenericTab(ByteSourceAbstract *nbytesource, GuiHelper *guiHelper, QWidget *parent) :
    TabAbstract(guiHelper,parent),
    bytesource(nbytesource)
{
    ableToReceiveData = false;
    newDefaultTextViewAction = NULL;
    setName(bytesource->name());
    connect(bytesource,SIGNAL(nameChanged(QString)), SLOT(setName(QString)));

    hexView = new(std::nothrow) HexView(bytesource, guiHelper,this);
    if (hexView == NULL) {
        qFatal("Cannot allocate memory for HexView X{");
    }

    ui = new(std::nothrow) Ui::GenericTab();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::FileTab X{");
    }
    ui->setupUi(this);
    ui->tabWidget->addTab(hexView, tr("Hex"));
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), SLOT(onDeleteTab(int)));

    searchWidget = new(std::nothrow) SearchWidget(bytesource, guiHelper, this);
    if (searchWidget == NULL) {
        qFatal("Cannot allocate memory for SearchWidget X{");
    }
    searchWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    connect(searchWidget, SIGNAL(jumpTo(quint64,quint64)), hexView, SLOT(gotoSearch(quint64,quint64)));

    ui->mainLayout->insertWidget(1,searchWidget);
    connect(searchWidget, SIGNAL(searchRequest(QByteArray,QBitArray,bool)), SLOT(onSearch(QByteArray,QBitArray,bool)));

    hexView->installEventFilter(this);

    gotoWidget = new(std::nothrow) OffsetGotoWidget(guiHelper,this);
    if (gotoWidget == NULL) {
        qFatal("Cannot allocate memory for OffsetGotoWidget X{");
    }
    gotoWidget->setMaximumWidth(150);
    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(ui->loadPushButton) + 1, gotoWidget);
    connect(gotoWidget,SIGNAL(gotoRequest(quint64,bool,bool,bool)), SLOT(onGotoOffset(quint64,bool,bool,bool)));

    roButton = new(std::nothrow) ReadOnlyButton(bytesource,this);
    if (roButton == NULL) {
        qFatal("Cannot allocate memory for ReadOnlyButton X{");
    }
    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(gotoWidget) + 2,roButton);

    clearAllMarksButton = new(std::nothrow) ClearAllMarkingsButton(bytesource,this);
    if (clearAllMarksButton == NULL) {
        qFatal("Cannot allocate memory for clearAllMarkingsButton X{");
    }
    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(roButton) + 1, clearAllMarksButton);

    guiButton = new(std::nothrow) ByteSourceGuiButton(bytesource,guiHelper,this);
    if (guiButton == NULL) {
        qFatal("Cannot allocate memory for ByteSourceGuiButton X{");
    }

    ui->mainToolBarLayout->insertWidget(5, guiButton);

    integrateByteSource();

    detachButton = new(std::nothrow) DetachTabButton(this);
    if (detachButton == NULL) {
        qFatal("Cannot allocate memory for detachButton X{");
    }

    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(ui->loadPushButton) + 1,detachButton);

    messagePanel = new(std::nothrow) MessagePanelWidget(this);
    if (messagePanel == NULL) {
        qFatal("Cannot allocate memory for MessagePanelWidget X{");
    }


    ui->mainLayout->insertWidget(ui->mainLayout->indexOf(ui->tabWidget) + 1,messagePanel);
    connect(bytesource, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), messagePanel, SLOT(log(QString,QString,Pip3lineConst::LOGLEVEL)));
    //connect(byteSource, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), logger, SLOT(log(QString,QString,Pip3lineConst::LOGLEVEL)));

    // Checking if there are some additional buttons
    QWidget *gui = bytesource->getGui(this, ByteSourceAbstract::GUI_BUTTONS);
    if (gui != NULL) {
        ui->mainToolBarLayout->insertWidget(4, gui);
    }

    // Checking if there is an additional upper pane
    gui = bytesource->getGui(this, ByteSourceAbstract::GUI_UPPER_VIEW);
    if (gui != NULL) {
        ui->mainLayout->insertWidget(1,gui);
    }

    QMenu * newViewMenu = new(std::nothrow)QMenu(ui->addViewPushButton);
    if (newViewMenu == NULL) {
        qFatal("Cannot allocate memory for QMenu X{");
    }

    QAction * descAction = new(std::nothrow)QAction(tr("View type"),newViewMenu);
    if (descAction == NULL) {
        qFatal("Cannot allocate memory for QAction X{");
    }
    descAction->setDisabled(true);
    newViewMenu->addAction(descAction);

    newHexViewAction = new(std::nothrow)QAction(tr("Hexadecimal"),newViewMenu);
    if (newHexViewAction == NULL) {
        qFatal("Cannot allocate memory for QAction X{");
    }
    newViewMenu->addAction(newHexViewAction);

    newTextViewAction = new(std::nothrow)QAction(tr("Text"),newViewMenu);
    if (newTextViewAction == NULL) {
        qFatal("Cannot allocate memory for QAction X{");
    }
    newViewMenu->addAction(newTextViewAction);
    newViewMenu->addSeparator();

    descAction = new(std::nothrow)QAction(tr("Predefined view"),newViewMenu);
        if (descAction == NULL) {
            qFatal("Cannot allocate memory for QAction X{");
        }
    descAction->setDisabled(true);
    newViewMenu->addAction(descAction);

    newDefaultTextViewAction = new(std::nothrow)QAction(tr("Default Text view"),newViewMenu);
        if (newDefaultTextViewAction == NULL) {
            qFatal("Cannot allocate memory for newDefaultTextViewAction X{");
        }
    newViewMenu->addAction(newDefaultTextViewAction);

    ui->addViewPushButton->setMenu(newViewMenu);

    UniversalReceiverButton *urb = new(std::nothrow) UniversalReceiverButton(this, guiHelper);
    if (urb == NULL) {
        qFatal("Cannot allocate memory for UniversalReceiverButton X{");
    }

    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(gotoWidget) + 1,urb);

    connect(newViewMenu,SIGNAL(triggered(QAction*)), SLOT(onNewViewTab(QAction*)));
    connect(hexView,SIGNAL(askForFileLoad()), SLOT(fileLoadRequest()));
    connect(ui->loadPushButton, SIGNAL(clicked()), SLOT(fileLoadRequest()));
    connect(ui->historyUndoPushButton, SIGNAL(clicked()), SLOT(onHistoryBackward()));
    connect(ui->historyRedoPushButton, SIGNAL(clicked()), SLOT(onHistoryForward()));

    setAcceptDrops(true);
}

GenericTab::~GenericTab()
{
    delete gotoWidget;
    delete searchWidget;
    delete detachButton;
    delete guiButton;
    delete ui;
    delete bytesource;
    bytesource = NULL;
}

void GenericTab::loadFromFile(QString fileName)
{
    if (fileName.isEmpty()) {
        logger->logError("Empty file name, ignoring",metaObject()->className());
        return;
    }

    if (bytesource->hasCapability(ByteSourceAbstract::CAP_LOADFILE)) {
        bytesource->fromLocalFile(fileName);
        integrateByteSource();
    }  else {
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_LOADFILE capability, ignoring").arg(((QObject *)bytesource)->metaObject()->className()),QMessageBox::Ok);
    }
}

int GenericTab::getBlockCount() const
{
    return 1;
}

ByteSourceAbstract *GenericTab::getSource(int)
{
    return bytesource;
}

ByteTableView *GenericTab::getHexTableView(int)
{
    return hexView->getHexTableView();
}

void GenericTab::setData(const QByteArray &data)
{
    if (bytesource->isReadonly()) {
        QMessageBox::critical(this, tr("Read only"), tr("The byte source is readonly"),QMessageBox::Ok);
    } else if (bytesource->hasCapability(ByteSourceAbstract::CAP_RESET)) {
        bytesource->setData(data);
    } else {
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_RESET capability").arg(((QObject *)bytesource)->metaObject()->className()),QMessageBox::Ok);
    }
}

bool GenericTab::canReceiveData()
{
    return (bytesource != NULL && bytesource->hasCapability(ByteSourceAbstract::CAP_RESET));
}

BaseStateAbstract *GenericTab::getStateMngtObj()
{
    GenericTabStateObj *stateObj = new(std::nothrow) GenericTabStateObj(this);
    if (stateObj == NULL) {
        qFatal("Cannot allocate memory for GenericTabStateObj X{");
    }

    return stateObj;
}

void GenericTab::fileLoadRequest()
{
    QString fileName;
    if (bytesource->hasCapability(ByteSourceAbstract::CAP_LOADFILE)) {
        fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"));
        if (!fileName.isEmpty()) {
            bytesource->fromLocalFile(fileName);
            integrateByteSource();
            setName(QFileInfo(fileName).fileName());
        }
    } else {
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_LOADFILE capability, ignoring").arg(((QObject *)bytesource)->metaObject()->className()),QMessageBox::Ok);
    }
}

void GenericTab::onGotoOffset(quint64 offset, bool absolute, bool negative, bool select)
{
    if (!hexView->goTo(offset,absolute,negative, select)) {
        gotoWidget->setStyleSheet(GuiStyles::LineEditError);
    } else {
        gotoWidget->setStyleSheet(qApp->styleSheet());
    }
}

void GenericTab::onHistoryBackward()
{
    bytesource->historyBackward();
}

void GenericTab::onHistoryForward()
{
    bytesource->historyForward();
}

void GenericTab::onNewViewTab(QAction *action)
{
    ViewTab vt;
    if (action == newDefaultTextViewAction) {  // usual Text View (no input transform)
        vt.type = DEFAULTTEXT;
        vt.transform = NULL;
        vt.tabName = TEXT_TEXT;
    } else {
        QuickViewItemConfig *itemConfig = new(std::nothrow) QuickViewItemConfig(guiHelper, this);
        if (itemConfig == NULL) {
            qFatal("Cannot allocate memory for QuickViewItemConfig X{");
        }
        itemConfig->setWayBoxVisible(true);
        itemConfig->setFormatVisible(false);
        itemConfig->setOutputTypeVisible(false);
        int ret = itemConfig->exec();
        if (ret == QDialog::Accepted) {
            TransformAbstract * ta = itemConfig->getTransform();
            vt.transform = ta;
            vt.tabName = itemConfig->getName();
            if (action == newHexViewAction) {
                vt.type = HEXVIEW;
            } else if (action == newTextViewAction) {
                vt.type = TEXTVIEW;
            } else {
                qWarning("New View Action not managed T_T");
                vt.type = UNDEFINED;
            }

            delete itemConfig;
        } else { // action cancelled
            delete itemConfig;
            return;
        }
    }
    addViewTab(vt);
}

void GenericTab::onDeleteTab(int index)
{
    if (ui->tabWidget->indexOf(hexView) == index) {
        logger->logError(tr("Cannot close the main hexadecimal tab"),metaObject()->className());
    } else { // here we trust the index as it comes from the QTabWidget
        SingleViewAbstract * sva = static_cast<SingleViewAbstract *>(ui->tabWidget->widget(index));
        ViewTab vt;
        index--; // need to reduce the index to match the tabData index
        if (index < 0 || index >= tabData.size()) {
            qCritical() << tr("The index for Tabdata is out-of-bound T_T");
            return;
        } else {
            vt = tabData.takeAt(index);
        }

        delete sva;
    }
}

void GenericTab::onSearch(QByteArray item, QBitArray mask, bool)
{
    hexView->search(item, mask);
}

bool GenericTab::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) { // ctrl-[key]
            if (keyEvent->key() == Qt::Key_N)  {
                searchWidget->nextFind(hexView->getLowPos());
                return true;
            } else if (keyEvent->key() == Qt::Key_G) {
                gotoWidget->setFocus();
                return true;
            } else if (keyEvent->key() == Qt::Key_F) {
                searchWidget->setFocus();
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void GenericTab::dragEnterEvent(QDragEnterEvent *event)
{
    guiHelper->processDragEnter(event, bytesource);
}

void GenericTab::dropEvent(QDropEvent *event)
{
    guiHelper->processDropEvent(event, bytesource);
}

void GenericTab::integrateByteSource()
{
    if (bytesource != NULL) {
        roButton->refreshStateValue();

        ui->historyRedoPushButton->setEnabled(bytesource->hasCapability(ByteSourceAbstract::CAP_HISTORY));
        ui->historyUndoPushButton->setEnabled(bytesource->hasCapability(ByteSourceAbstract::CAP_HISTORY));
    }
}

void GenericTab::addViewTab(GenericTab::ViewTab data)
{
    SingleViewAbstract * newView = NULL;
    QTabWidget * thetabwiget = ui->tabWidget;
    switch (data.type) {
        case (GenericTab::HEXVIEW) : {
                IntermediateSource * is = new(std::nothrow) IntermediateSource(guiHelper,bytesource,data.transform);
                if (is == NULL) {
                    qFatal("Cannot allocate memory for IntermediateSource X{");
                }

                newView = new(std::nothrow) HexView(is,guiHelper,thetabwiget,true);
                if (newView == NULL) {
                    qFatal("Cannot allocate memory for HexView X{");
                }
            }
            break;
        case (GenericTab::TEXTVIEW) : {
                IntermediateSource * is = new(std::nothrow) IntermediateSource(guiHelper,bytesource,data.transform);
                if (is == NULL) {
                    qFatal("Cannot allocate memory for IntermediateSource X{");
                }

                newView = new(std::nothrow) TextView(is,guiHelper,thetabwiget,true);
                if (newView == NULL) {
                    qFatal("Cannot allocate memory for TextView X{");
                }
            }
            break;
        case (GenericTab::DEFAULTTEXT) : {
                newView = new(std::nothrow) TextView(bytesource,guiHelper,thetabwiget);
                if (newView == NULL) {
                    qFatal("Cannot allocate memory for TextView X{");
                }
            }
            break;
        default: {
                logger->logError(tr("View Type undefined"));
            }
    }

    if (newView != NULL) {
        thetabwiget->addTab(newView,data.tabName);
        tabData.append(data);
    }
}

MessagePanelWidget *GenericTab::getMessagePanel() const
{
    return messagePanel;
}

SearchWidget *GenericTab::getSearchWidget() const
{
    return searchWidget;
}

OffsetGotoWidget *GenericTab::getGotoWidget() const
{
    return gotoWidget;
}

GenericTabStateObj::GenericTabStateObj(GenericTab *tab) :
    TabStateObj(tab)
{
    setName(metaObject()->className());
}

GenericTabStateObj::~GenericTabStateObj()
{

}

void GenericTabStateObj::run()
{
    GenericTab * gTab = dynamic_cast<GenericTab *> (tab);
    TabStateObj::run();
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        writer->writeAttribute(GuiConst::STATE_SEARCH_WIDGET, write(gTab->getSearchWidget()->text(),true));
        writer->writeAttribute(GuiConst::STATE_GOTOOFFSET_WIDGET, write(gTab->getGotoWidget()->text()));
        writer->writeAttribute(GuiConst::STATE_MESSAGE_PANEL, write(gTab->getMessagePanel()->toHTML(),true));
        writer->writeAttribute(GuiConst::STATE_MESSAGE_PANEL_VISIBLE, write(gTab->getMessagePanel()->isVisible()));
        writer->writeAttribute(GuiConst::STATE_SCROLL_INDEX, write(gTab->hexView->getHexTableView()->verticalScrollBar()->value()));
        writer->writeAttribute(GuiConst::STATE_CURRENT_INDEX, write(gTab->ui->tabWidget->currentIndex()));
        QList<GenericTab::ViewTab> tabs = gTab->tabData;
        int size = tabs.size();
        writer->writeStartElement(GuiConst::STATE_TABVIEWLIST);
        writer->writeAttribute(GuiConst::STATE_SIZE, write(tabs.size()));
        for (int i = 0; i < size ; i++) {
            writer->writeStartElement(GuiConst::STATE_TABVIEW);
            writer->writeAttribute(GuiConst::STATE_TYPE, write((int)tabs.at(i).type));
            // saving configuration
            QString conf;
            TransformChain list;
            if (tabs.at(i).transform != NULL) {
                list.append(tabs.at(i).transform);
            }
            list.setName(tabs.at(i).tabName);
            QXmlStreamWriter streamin(&conf);
            tab->getHelper()->getTransformFactory()->saveConfToXML(list, &streamin);
            writer->writeAttribute(GuiConst::STATE_CONF, write(conf));
            writer->writeEndElement();
            list.clear(); // the transforms are not own by us, don't delete them
        }
        writer->writeEndElement();
    } else {
        QXmlStreamAttributes attrList = reader->attributes();
        if (attrList.hasAttribute(GuiConst::STATE_SEARCH_WIDGET)) {
            gTab->getSearchWidget()->setText(readString(attrList.value(GuiConst::STATE_SEARCH_WIDGET)));
        }

        if (attrList.hasAttribute(GuiConst::STATE_GOTOOFFSET_WIDGET)) {
            gTab->getGotoWidget()->setText(readString(attrList.value(GuiConst::STATE_GOTOOFFSET_WIDGET)));
        }

        if (attrList.hasAttribute(GuiConst::STATE_MESSAGE_PANEL)) {
            gTab->getMessagePanel()->setHTML(readString(attrList.value(GuiConst::STATE_MESSAGE_PANEL)));
        }

        if (attrList.hasAttribute(GuiConst::STATE_MESSAGE_PANEL_VISIBLE)) {
            gTab->getMessagePanel()->setVisible(readBool(attrList.value(GuiConst::STATE_MESSAGE_PANEL_VISIBLE)));
        }

        bool ok = false;
        int index = -1;
        GenericTabClosingStateObj *tempState = NULL;
        if (attrList.hasAttribute(GuiConst::STATE_SCROLL_INDEX)) {
            index = readInt(attrList.value(GuiConst::STATE_SCROLL_INDEX), &ok);
            if (ok) {
                tempState = new(std::nothrow) GenericTabClosingStateObj(gTab);
                if (tempState == NULL) {
                    qFatal("Cannot allocate memory for GenericTabClosingStateObj X{");
                }
                tempState->setScrollIndex(index);
            }
        }

        if (attrList.hasAttribute(GuiConst::STATE_CURRENT_INDEX)) {
            index = readInt(attrList.value(GuiConst::STATE_CURRENT_INDEX), &ok);
            if (ok) {
                if (tempState == NULL) {
                    tempState = new(std::nothrow) GenericTabClosingStateObj(gTab);
                    if (tempState == NULL) {
                        qFatal("Cannot allocate memory for GenericTabClosingStateObj X{");
                    }
                }
                tempState->setCurrentIndex(index);
            }
        }

        if (tempState != NULL)
            emit addNewState(tempState);

        if (readNextStart(GuiConst::STATE_TABVIEWLIST)) {
            attrList = reader->attributes();
            if (attrList.hasAttribute(GuiConst::STATE_SIZE)) {
                ok = false;
                int size = readInt(attrList.value(GuiConst::STATE_SIZE),&ok);
                if (ok) {
                    for (int i = 0; i < size ; i++) {
                        GenericTab::ViewTab vt;
                        vt.transform = NULL; // just initialising in case of screw up
                        if (readNextStart(GuiConst::STATE_TABVIEW)) {
                            attrList = reader->attributes();
                            readEndElement(GuiConst::STATE_TABVIEW); // closing now, because there is no child defined anyway
                            if (attrList.hasAttribute(GuiConst::STATE_TYPE)) {
                                int type = readInt(attrList.value(GuiConst::STATE_TYPE),&ok);
                                if (ok && (type == 0 || type == 1 || type == 2 || type == 3)) {
                                    vt.type = (GenericTab::ViewType) type;
                                } else {
                                    qWarning() << "Invalid state type for this view, skipping";
                                    continue;
                                }
                            } else {
                                qWarning() << "no state type for this view, skipping";
                                continue;
                            }
                            if (vt.type != GenericTab::DEFAULTTEXT) {
                                if (attrList.hasAttribute(GuiConst::STATE_CONF)) {
                                    QString conf = readString(attrList.value(GuiConst::STATE_CONF));
                                    if (!conf.isEmpty()) {
                                        TransformChain list = tab->getHelper()->getTransformFactory()->loadConfFromXML(conf);
                                        if (!list.isEmpty()) {
                                            vt.transform = list.at(0);
                                            vt.tabName = list.getName();
                                        } else {
                                            qWarning() << "Resulting transform list empty for this view, skipping";
                                            continue;
                                        }
                                    } else {
                                        qWarning() << "Empty conf for this view, skipping";
                                        continue;
                                    }
                                } else {
                                    qWarning() << "no conf for this view, skipping";
                                    continue;
                                }
                            } else {
                                vt.tabName = GenericTab::TEXT_TEXT;
                            }
                            gTab->addViewTab(vt);
                            // no deleting vt.transform here, the pointer is now owned by the tab.
                        }
                    }
                } else {
                    qCritical() << "Size value is not a number, giving up";
                }
            } else {
                 qCritical() << "No size value for the view size, giving up";
            }
        }
        readEndElement(GuiConst::STATE_TABVIEWLIST);
    }

    BaseStateAbstract *state = gTab->getSource(0)->getStateMngtObj();
    emit addNewState(state);
}



GenericTabClosingStateObj::GenericTabClosingStateObj(GenericTab *tab) :
    tab(tab)
{
    scrollIndex = 0;
    currentIndex = 0;
    name = metaObject()->className();
}

GenericTabClosingStateObj::~GenericTabClosingStateObj()
{

}

void GenericTabClosingStateObj::run()
{
    tab->hexView->getHexTableView()->verticalScrollBar()->setValue(scrollIndex);
    tab->ui->tabWidget->setCurrentIndex(currentIndex);
}

void GenericTabClosingStateObj::setScrollIndex(int value)
{
    scrollIndex = value;
}

void GenericTabClosingStateObj::setCurrentIndex(int value)
{
    currentIndex = value;
}

