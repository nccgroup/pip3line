/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "tababstract.h"
#include <QSettings>
#include <QState>
#include <QDebug>
#include "../loggerwidget.h"
#include "../guihelper.h"
#include "../sources/bytesourceabstract.h"
#include "../shared/guiconst.h"
#include "../state/closingstate.h"

using namespace GuiConst;

const int TabAbstract::WINDOWED_TAB = 1;

TabAbstract::TabAbstract(GuiHelper *nguiHelper, QWidget *parent):
    QWidget(parent),
    guiHelper(nguiHelper)
{
    logger = guiHelper->getLogger();
    preTabType = INVALID_PRETAB;
}

TabAbstract::~TabAbstract()
{

}

void TabAbstract::setName(const QString &nname)
{
    if (nname != name) {
        name = nname;
        QWidget * parent = parentWidget();
        if (parent != NULL && QString(parent->metaObject()->className()) == "FloatingDialog")
            parentWidget()->setWindowTitle(name);
        emit nameChanged();
    }
}

QString TabAbstract::getName() const
{
    return name;
}


void TabAbstract::bringFront()
{
    emit askBringFront();
}

bool TabAbstract::canReceiveData()
{
    return true;
}

void TabAbstract::onDetach()
{
    emit askWindowTabSwitch();
}
GuiConst::AVAILABLE_PRETABS TabAbstract::getPreTabType() const
{
    return preTabType;
}

void TabAbstract::setPreTabType(const GuiConst::AVAILABLE_PRETABS &value)
{
    preTabType = value;
}

GuiHelper *TabAbstract::getHelper()
{
    return guiHelper;
}

TabStateObj::TabStateObj(TabAbstract *tab) :
    tab(tab)
{
    setName(tab->getName());
    isWindowed = false;
}

TabStateObj::~TabStateObj()
{

}

void TabStateObj::run()
{
    BaseStateAbstract *tempState = new(std::nothrow) ClosingState(); // this will close the current tab element
    if (tempState == NULL) {
        qFatal("Cannot allocate memory for ClosingState X{");
    }

    emit addNewState(tempState);

    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        writer->writeStartElement(GuiConst::STATE_TAB);
        writer->writeAttribute(GuiConst::STATE_PRETAB_TYPE, write((int)tab->getPreTabType()));
        writer->writeAttribute(GuiConst::STATE_TAB_NAME, tab->getName());
        if (isWindowed) {
            writer->writeAttribute(GuiConst::STATE_WINDOWED, QString::number(TabAbstract::WINDOWED_TAB));
            writer->writeAttribute(GuiConst::STATE_WIDGET_GEOM, write(windowState));
        }
    } else {
        QXmlStreamAttributes attrList = reader->attributes();
        if (attrList.hasAttribute(GuiConst::STATE_TAB_NAME)) {
            QString name = attrList.value(GuiConst::STATE_TAB_NAME).toString();
            if (!name.isEmpty()) {
                tab->setName(name);
            }
        }
    }
}
bool TabStateObj::getIsWindowed() const
{
    return isWindowed;
}

void TabStateObj::setIsWindowed(bool value)
{
    isWindowed = value;
}
QByteArray TabStateObj::getWindowState() const
{
    return windowState;
}

void TabStateObj::setWindowState(const QByteArray &value)
{
    windowState = value;
}


