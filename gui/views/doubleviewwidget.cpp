/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "doubleviewwidget.h"
#include "ui_doubleviewwidget.h"

DoubleViewWidget::DoubleViewWidget(ByteSourceAbstract *byteSource1, ByteSourceAbstract *byteSource2, GuiHelper *nguiHelper, QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::DoubleViewWidget)
{
    guiHelper = nguiHelper;
    ui->setupUi(this);
    view1 = new(std::nothrow) HexView(byteSource1, guiHelper, parent);
    if (view1 == NULL) {
        qFatal("Cannot allocate memory for view1 X{");
    }

    ui->mainWidget->layout()->addWidget(view1);

    view2 = new(std::nothrow) HexView(byteSource2, guiHelper, parent);
    if (view2 == NULL) {
        qFatal("Cannot allocate memory for view2 X{");
    }
    ui->mainWidget->layout()->addWidget(view2);
}

DoubleViewWidget::~DoubleViewWidget()
{
    delete ui;
}
