#ifndef FOLDEDWIDGET_H
#define FOLDEDWIDGET_H

#include <QWidget>
#include "transformwidget.h"

class QMouseEvent;

namespace Ui {
    class FoldedWidget;
}

class FoldedWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit FoldedWidget(TransformWidget * transformWidget, QWidget *parent = NULL);
        ~FoldedWidget();
        TransformWidget *getTransformWidget() const;

    signals:
        void unfoldRequested();
    private:
        void mouseDoubleClickEvent ( QMouseEvent * event );
        Ui::FoldedWidget * ui;
        TransformWidget * transformWidget;
};

#endif // FOLDEDWIDGET_H
