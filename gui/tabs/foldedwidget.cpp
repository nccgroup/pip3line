#include "foldedwidget.h"
#include "ui_foldedwidget.h"

FoldedWidget::FoldedWidget(TransformWidget *transformWidget, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FoldedWidget),
    transformWidget(transformWidget)
{
    ui->setupUi(this);
    ui->descLabel->setText(transformWidget->getDescription());
    connect(ui->unFoldPushButton, SIGNAL(clicked()), this, SIGNAL(unfoldRequested()));
}

FoldedWidget::~FoldedWidget()
{
    delete ui;
    transformWidget = NULL;
}
TransformWidget *FoldedWidget::getTransformWidget() const
{
    return transformWidget;
}

