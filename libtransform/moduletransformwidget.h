/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MODULETRANSFORMWIDGET_H
#define MODULETRANSFORMWIDGET_H

#include "libtransform_global.h"
#include "scripttransformabstract.h"
#include <QWidget>

namespace Ui {
class ModuleTransformWidget;
}

class LIBTRANSFORMSHARED_EXPORT ModuleTransformWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit ModuleTransformWidget(ScriptTransformAbstract *transform, QWidget *parent = 0);
        ~ModuleTransformWidget();
    private slots:
        void onChooseFile();
        void onMakePersistent(bool checked);
    private:
        Ui::ModuleTransformWidget *ui;
        ScriptTransformAbstract *transform;
};

#endif // MODULETRANSFORMWIDGET_H
