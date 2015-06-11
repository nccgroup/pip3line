/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "appdialog.h"
#include "guihelper.h"
#include "loggerwidget.h"
#include "shared/guiconst.h"
#include <QDebug>
#include <QSettings>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>

using namespace GuiConst;


AppStateObj::AppStateObj(AppDialog *diag) :
    dialog(diag)
{
    setName(dialog->metaObject()->className());
}

AppStateObj::~AppStateObj()
{

}

void AppStateObj::run()
{
    qDebug() << "Save/load" << name;
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        if (writer != NULL)
            writer->writeStartElement(name);
        else {
            emit log(tr("Writer is null T_T"),this->metaObject()->className(), Pip3lineConst::LERROR);
            return;
        }
    } else if (!readNextStart()) // the check for reader == NULL is already done in the function
        return;


    QByteArray sdata;
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        if (flags & GuiConst::STATE_LOADSAVE_DIALOG_POS) {
            writer->writeAttribute(GuiConst::STATE_WIDGET_GEOM, write(dialog->saveGeometry()));
            writer->writeAttribute(GuiConst::STATE_DIALOG_ISVISIBLE, write(dialog->isVisible()));
        }
    } else {
        if (flags & GuiConst::STATE_LOADSAVE_DIALOG_POS &&
                reader->name() == name) {
            QXmlStreamAttributes attrList = reader->attributes();
            sdata = readByteArray(attrList.value(GuiConst::STATE_WIDGET_GEOM));
            if (!sdata.isEmpty())
                dialog->restoreGeometry(sdata);

            if (attrList.hasAttribute(GuiConst::STATE_DIALOG_ISVISIBLE))
                dialog->setVisible(readBool(attrList.value(GuiConst::STATE_DIALOG_ISVISIBLE)));
        }
    }

    internalRun();

    genCloseElement();
}

void AppStateObj::internalRun()
{
//    if (reader != NULL)
//        qDebug() << "AppStateObj:" << reader->name();
}


AppDialog::AppDialog(GuiHelper * nguiHelper, QWidget *parent) :
    QDialog(parent)
{
    guiHelper = nguiHelper;
    logger = guiHelper->getLogger();
    savedVisibleState = isVisible();
    attachedAction = NULL;
    connect(guiHelper, SIGNAL(appGoesIntoHidding()), SLOT(onMainHiding()));
    connect(guiHelper, SIGNAL(appIsRising()), SLOT(onMainRising()));
    //qDebug() << "created" << this;
}

void AppDialog::attachAction(QAction *action)
{
    if (action->isCheckable())
        attachedAction = action;
    else
        logger->logError(tr("Action is not checkable, ignoring"),this->metaObject()->className());
}

BaseStateAbstract *AppDialog::getStateMngtObj()
{
    BaseStateAbstract *stateObj = new(std::nothrow) AppStateObj(this);
    if (stateObj == NULL) {
        qFatal("Cannot allocate memory for AppStateObj X{");
    }

    return stateObj;
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
