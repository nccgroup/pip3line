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
#include <QMutex>
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
        void processMessages();
        QString getXmlConf();
        void processData(const QByteArray &data);
        bool isConfigured();
    private slots:
        void internalProcess();
        void renderData(const QByteArray &data);
        void logError(const QString &mess, const QString &);
        void logWarning(const QString &mess, const QString &);
        void mouseDoubleClickEvent (QMouseEvent *event);
    signals:
        void askForDeletion(QuickViewItem *);
        void updateData(QByteArray data);
    private:
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
        QMutex dataMutex;
        QMutex messMutex;
};

#endif // QUICKVIEWITEM_H
