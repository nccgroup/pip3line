/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef QUICKVIEWITEMCONFIG_H
#define QUICKVIEWITEMCONFIG_H

#include <QDialog>
#include <transformabstract.h>
#include <transformmgmt.h>
#include <QWidget>
#include <commonstrings.h>
#include "guihelper.h"
#include "infodialog.h"

using namespace Pip3lineConst;

namespace Ui {
class QuickViewItemConfig;
}

class QuickViewItemConfig : public QDialog
{
        Q_OBJECT
        
    public:
        explicit QuickViewItemConfig(GuiHelper *guiHelper, QWidget *parent = 0);
        ~QuickViewItemConfig();
        TransformAbstract * getTransform();
        void setTransform(TransformAbstract * transform);
        void setName(const QString &name);
        QString getName() const;
        OutputFormat getFormat() const;
        void setFormat(OutputFormat format);
        void setOutputType(OutputType type);
        OutputType getOutputType();
        void setWayBoxVisible(bool val);
        void setFormatVisible(bool val);
        void setOutputTypeVisible(bool val);
        void closeEvent(QCloseEvent *event);


    private slots:
        void onTransformSelect(QString name);
        void onInboundWayChange(bool checked);
        void onTextFormatToggled(bool checked);
        void onInfo();
        void integrateTransform();
        void onTransformDelete();
    private:
        Q_DISABLE_COPY(QuickViewItemConfig)
        static const QString LOGID;
        Ui::QuickViewItemConfig *ui;
        GuiHelper *guiHelper;
        TransformAbstract *currentTransform;
        TransformMgmt * transformFactory;
        QWidget *confGui;
        InfoDialog * infoDialog;
        OutputFormat format;
        bool wayBoxVisible;
        bool formatBoxVisible;
        bool outputTypeVisible;
};

#endif // QUICKVIEWITEMCONFIG_H
