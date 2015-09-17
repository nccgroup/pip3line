#include "foldedview.h"
#include "ui_foldedview.h"

FoldedView::FoldedView(TransformWidget * transformWidget, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FoldedView),
    transformWidget(transformWidget)
{
    ui->setupUi(this);
    ui->descLabel->setText(transformWidget->getDescription());
}

FoldedView::~FoldedView()
{
    delete ui;
    transformWidget = NULL;
}

TransformWidget *FoldedView::getTransformWidget() const
{
    return transformWidget;
}

void FoldedView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit unfoldRequested();
    }
}
