/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "appdialog.h"
#include "guihelper.h"
#include "loggerwidget.h"
#include <QDebug>

AppDialog::AppDialog(GuiHelper * nguiHelper, QWidget *parent) :
    QDialog(parent)
{
    guiHelper = nguiHelper;
    logger = guiHelper->getLogger();
    savedVisibleState = isVisible();
    attachedAction = NULL;
    connect(guiHelper, SIGNAL(appGoesIntoHidding()), SLOT(onMainHiding()));
    connect(guiHelper, SIGNAL(appIsRising()), SLOT(onMainRising()));
}

void AppDialog::attachAction(QAction *action)
{
    if (action->isCheckable())
        attachedAction = action;
    else
        logger->logError(tr("Action is not checkable, ignoring"),this->metaObject()->className());
}

void AppDialog::onMainHiding()
{
    savedVisibleState = isVisible();
    if (savedVisibleState)
        hide();
}

void AppDialog::onMainRising()
{
    if (savedVisibleState) {
        show();
    }
}

void AppDialog::showEvent(QShowEvent *event)
{
    if (!savedPos.isNull())
        move(savedPos);

    if (attachedAction != NULL)
        attachedAction->setChecked(true);
    QDialog::showEvent(event);
}

void AppDialog::hideEvent ( QHideEvent * event)
{
    savedPos = pos();
    if (attachedAction != NULL)
        attachedAction->setChecked(false);
    QDialog::hideEvent(event);
}
