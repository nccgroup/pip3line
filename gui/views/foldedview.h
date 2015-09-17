#ifndef FOLDEDVIEW_H
#define FOLDEDVIEW_H

#include <QFrame>
#include "transformwidget.h"

class QMouseEvent;

namespace Ui {
    class FoldedView;
}

class FoldedView : public QFrame
{
        Q_OBJECT

    public:
        explicit FoldedView(TransformWidget * transformWidget, QWidget *parent = 0);
        ~FoldedView();
        TransformWidget *getTransformWidget() const;
    signals:
        void unfoldRequested();
    private:
        void mouseDoubleClickEvent(QMouseEvent * event);
        Ui::FoldedView *ui;
        TransformWidget * transformWidget;
};

#endif // FOLDEDVIEW_H
