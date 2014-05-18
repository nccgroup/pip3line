/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "readonlybutton.h"
#include "../sources/bytesourceabstract.h"
#include "../guihelper.h"
#include <QMessageBox>
#include <QToolTip>

const QString ReadOnlyButton::ReadWrite = "RW";
const QString ReadOnlyButton::ReadOnly = "RO";
const QString ReadOnlyButton::ButtonToolTipRW = "Current state is read/write, click here if you want to switch to readonly";
const QString ReadOnlyButton::ButtonToolTipRO = "Current state is read-only, click here if you want to enable write";
const QString ReadOnlyButton::CAP_WRITE_ToolTip = "Disabled: the source cannot be modified";

ReadOnlyButton::ReadOnlyButton(ByteSourceAbstract *bytesource, QWidget *parent) :
    QPushButton(parent)
{
    byteSource = bytesource;
    setCheckable(true);
    setMaximumWidth(50);
    setFlat(true);
    refreshStateValue();
    connect(this, SIGNAL(toggled(bool)), SLOT(onToggle(bool)));
}

void ReadOnlyButton::refreshStateValue()
{
    blockSignals(true);
    if (byteSource->isReadonly()) {
        setStyleSheet(GuiStyles::PushButtonReadonly);
        setChecked(true);
        setToolTip(ButtonToolTipRO);
        setText(ReadOnly);
    } else {
        setStyleSheet(QString());
        setChecked(false);
        setToolTip(ButtonToolTipRW);
        setText(ReadWrite);
    }


    if (!byteSource->hasCapability(ByteSourceAbstract::CAP_WRITE)) {
        setDisabled(true);
        setToolTip(CAP_WRITE_ToolTip);
    }
    blockSignals(false);
}

void ReadOnlyButton::onToggle(bool val)
{
    QString mess;
    if (byteSource->hasCapability(ByteSourceAbstract::CAP_WRITE)) {
        if (byteSource->setReadOnly(val)) {
            if (val) {
                setStyleSheet(GuiStyles::PushButtonReadonly);
                QToolTip::showText( this->mapToGlobal( QPoint( 0, 0 ) ), ButtonToolTipRO );
                setToolTip(ButtonToolTipRO);
                setText(ReadOnly);
            } else {
                setStyleSheet(QString());
                QToolTip::showText( this->mapToGlobal( QPoint( 0, 0 ) ), ButtonToolTipRW );
                setToolTip(ButtonToolTipRW);
                setText(ReadWrite);
            }
            return;
        } else {
            mess = tr("Could not set readonly property on %1").arg(((QObject *)byteSource)->metaObject()->className());
        }
    } else {
        mess = tr("%1 does not have the CAP_WRITE capability").arg(((QObject *)byteSource)->metaObject()->className());
        blockSignals(true);
        setStyleSheet(GuiStyles::PushButtonReadonly);
        setChecked(true);
        setText(ReadOnly);
        setToolTip(CAP_WRITE_ToolTip);
        blockSignals(false);
    }
    emit logError(mess);
    QMessageBox::critical(this,tr("Error"), mess ,QMessageBox::Ok);
}
