/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef DOUBLEVIEWWIDGET_H
#define DOUBLEVIEWWIDGET_H

#include <QDockWidget>
#include "hexview.h"
#include "../guihelper.h"
#include "../sources/bytesourceabstract.h"

namespace Ui {
class DoubleViewWidget;
}

class DoubleViewWidget : public QDockWidget
{
    Q_OBJECT
    
    public:
        explicit DoubleViewWidget(ByteSourceAbstract *byteSource1, ByteSourceAbstract *byteSource2, GuiHelper *guiHelper,QWidget *parent = 0);
        ~DoubleViewWidget();

    private:
        Q_DISABLE_COPY(DoubleViewWidget)
        Ui::DoubleViewWidget *ui;
        HexView *view1;
        HexView *view2;
        GuiHelper *guiHelper;
};

#endif // DOUBLEVIEWWIDGET_H
