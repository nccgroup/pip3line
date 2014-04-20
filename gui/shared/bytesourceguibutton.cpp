/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "bytesourceguibutton.h"
#include "../floatingdialog.h"
#include "../sources/bytesourceabstract.h"
#include "../guihelper.h"
#include <QDebug>
#include <QIcon>
#include <QAction>

ByteSourceGuiButton::ByteSourceGuiButton(ByteSourceAbstract *bytesource, GuiHelper *nguiHelper, QWidget *parent) :
    QPushButton(parent)
{
    byteSource = bytesource;
    guiHelper = nguiHelper;

    guidia = NULL;
    localAction = NULL;


    localAction = new(std::nothrow) QAction(tr(""), this);
    if (localAction == NULL) {
        qFatal("Cannot allocate memory for QAction X{");
    }
    localAction->setCheckable(true);
    setCheckable(true);
    setToolTip(tr("Configuration panel"));
    setIcon(QIcon(":/Images/icons/configure-5.png"));
    setMaximumWidth(25);
    //setDefaultAction(localAction);

    setFlat(true);
    gui = byteSource->getGui();
    if (gui != NULL) {
        connect(localAction, SIGNAL(toggled(bool)), SLOT(onToggle(bool)),Qt::UniqueConnection);
        connect(this, SIGNAL(toggled(bool)), SLOT(onToggle(bool)),Qt::UniqueConnection);
    } else {
        localAction->setToolTip(tr("No settings available for this source"));
        setDisabled(true);
    }
}

ByteSourceGuiButton::~ByteSourceGuiButton()
{
    delete guidia;
}

void ByteSourceGuiButton::refreshState()
{
    gui = byteSource->getGui();

    if (gui != NULL) {
        setDisabled(false);
        connect(localAction, SIGNAL(toggled(bool)), SLOT(onToggle(bool)),Qt::UniqueConnection);
        connect(this, SIGNAL(toggled(bool)), SLOT(onToggle(bool)),Qt::UniqueConnection);
    } else {
        qDebug() << tr("No gui available for this source %1").arg(byteSource->metaObject()->className());
        setDisabled(true);
    }

}

void ByteSourceGuiButton::onGuiDelete()
{
    gui = NULL;
    localAction->setChecked(false);
}

void ByteSourceGuiButton::onToggle(bool enabled)
{
    if (enabled) {
        if (guidia == NULL) {
            if (gui != NULL) {
                guidia = new(std::nothrow) FloatingDialog(guiHelper, gui);
                if (guidia == NULL) {
                    qFatal("Cannot allocate memory for FloatingDialog X{");
                }
                gui->setParent(guidia);

                guidia->attachAction(localAction);
                guidia->setWindowTitle(byteSource->description());
                guidia->setAllowReject(true);
                guidia->resize(gui->sizeHint());
                guidia->raise();
                guidia->show();
                connect(gui, SIGNAL(destroyed()), SLOT(onGuiDelete()));
                connect(guidia, SIGNAL(hiding()), SLOT(onGuiHiding()));
            }
        } else {
            guidia->raise();
            guidia->show();
        }
    } else if (guidia != NULL) {
        guidia->hide();
    }
}

void ByteSourceGuiButton::onGuiHiding()
{
    setChecked(false);
    localAction->setChecked(false);
}
