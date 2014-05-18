/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "singleviewabstract.h"
#include "../loggerwidget.h"
#include "../guihelper.h"
#include "../sources/bytesourceabstract.h"
#include <transformabstract.h>
#include <transformmgmt.h>
#include <threadedprocessor.h>

SingleViewAbstract::SingleViewAbstract(ByteSourceAbstract *dataModel,GuiHelper *nguiHelper, QWidget *parent) :
    QWidget(parent)
{
    byteSource = dataModel;
    guiHelper = nguiHelper;
    logger = guiHelper->getLogger();
}

SingleViewAbstract::~SingleViewAbstract()
{
    byteSource = NULL;
    logger = NULL;
}

void SingleViewAbstract::searchAgain()
{
    return search(previousSearch,previousMask);
}
ByteSourceAbstract *SingleViewAbstract::getByteSource() const
{
    return byteSource;
}

