/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "moduletransformwidget.h"
#include "ui_moduletransformwidget.h"
#include "modulesmanagement.h"
#include <QFileDialog>

ModuleTransformWidget::ModuleTransformWidget(ScriptTransformAbstract *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::ModuleTransformWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::ModuleTransformWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->fileLineEdit->setText(transform->getModuleFileName());
    if (transform->getType() == ModulesManagement::PERSISTENT) {
        ui->stackedWidget->setCurrentIndex(0);
        ui->makePersistentCheckBox->setChecked(true);
    } else if (transform->getType() == ModulesManagement::AUTO) {
        ui->stackedWidget->setCurrentIndex(1);
    } else {
        ui->stackedWidget->setCurrentIndex(0);
        ui->makePersistentCheckBox->setChecked(false);
    }
    ui->groupBox->setTitle(tr("%1 script").arg(transform->getScriptDescr()));
    connect(ui->choosePushButton, SIGNAL(clicked()), this, SLOT(onChooseFile()));
    connect(ui->makePersistentCheckBox, SIGNAL(toggled(bool)), this, SLOT(onMakePersistent(bool)));
}

ModuleTransformWidget::~ModuleTransformWidget()
{
    delete ui;
}

void ModuleTransformWidget::onChooseFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose %1 module file").arg(transform->getScriptDescr()),"", tr("Python files (*.py)"));
    if (!fileName.isEmpty()) {
        if (transform->setModuleFile(fileName)) {
            ui->fileLineEdit->setText(fileName);
        }
    }
}

void ModuleTransformWidget::onMakePersistent(bool checked)
{
    if (checked)
        transform->setType(ModulesManagement::PERSISTENT);
    else
        transform->setType(ModulesManagement::TRANSIENT);
}
