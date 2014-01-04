/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "detachtabbutton.h"
#include "../tabs/tababstract.h"

DetachTabButton::DetachTabButton(TabAbstract * tab) :
    QPushButton(tab)
{
    curtab = tab;
    setIcon(QIcon(":/Images/icons/tab-duplicate-2.png"));
    setFlat(true);
    setMaximumWidth(25);
    setToolTip(tr("Detach/attach tab"));
    connect(this, SIGNAL(clicked()), curtab, SLOT(onDetach()));
}
