/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef QUICKVIEWITEM_H
#define QUICKVIEWITEM_H

#include <QWidget>
#include <transformabstract.h>
#include <commonstrings.h>
#include "guihelper.h"
#include "quickviewitemconfig.h"

using namespace Pip3lineConst;

namespace Ui {
class QuickViewItem;
}

class QuickViewItem : public QWidget
{
        Q_OBJECT
        
    public:
        explicit QuickViewItem(GuiHelper * guiHelper, QWidget *parent = 0, const QString &guiConfig = QString());
        ~QuickViewItem();
        bool configure();
        QString getXmlConf();
        void processData(const QByteArray &data);
        bool isConfigured();
    private slots:
        void processingFinished(QByteArray output, Messages messages);
        void mouseDoubleClickEvent (QMouseEvent *event);
    signals:
        void askForDeletion(QuickViewItem *);
    private:
        Q_DISABLE_COPY(QuickViewItem)
        bool setXmlConf(const QString &conf);
        static const QString LOGID;
        Ui::QuickViewItem *ui;
        QuickViewItemConfig * guiConfig;
        GuiHelper * guiHelper;
        TransformAbstract *currentTransform;
        QString toolTipMess;
        bool noError;
        QByteArray currentData;
        OutputFormat format;
};

#endif // QUICKVIEWITEM_H
