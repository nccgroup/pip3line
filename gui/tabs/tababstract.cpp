/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "tababstract.h"
#include "../loggerwidget.h"
#include "../guihelper.h"

TabAbstract::TabAbstract(GuiHelper *nguiHelper, QWidget *parent):
    QWidget(parent)
{
    guiHelper = nguiHelper;
    logger = guiHelper->getLogger();
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
