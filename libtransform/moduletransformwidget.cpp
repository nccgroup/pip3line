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
#include <QStringList>
#include <QKeyEvent>
#include <QHashIterator>
#include <QMessageBox>
#include <QDebug>

ModuleTransformWidget::ModuleTransformWidget(ScriptTransformAbstract *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::ModuleTransformWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::ModuleTransformWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    model  = new(std::nothrow) ParametersItemModel();
    if (model == NULL) {
        qFatal("Cannot allocate memory for ParametersItemModel X{");
    }
    QAbstractItemModel * oldModel = ui->parameterstableView->model();
    ui->parameterstableView->setModel(model);
    oldModel->deleteLater();
    connect(model, SIGNAL(parametersChanged()), this, SLOT(onParametersUpdated()));
    connect(transform, SIGNAL(confUpdated()), this, SLOT(reloadParameters()));

    ui->parameterstableView->installEventFilter(this);
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
    ui->tabWidget->setTabText(0,tr("%1 script").arg(transform->getScriptDescr()));

    connect(ui->choosePushButton, SIGNAL(clicked()), this, SLOT(onChooseFile()));
    connect(ui->makePersistentCheckBox, SIGNAL(toggled(bool)), this, SLOT(onMakePersistent(bool)));
    connect(ui->addParamPushButton, SIGNAL(clicked()), this, SLOT(onAddParameter()));
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

void ModuleTransformWidget::onAddParameter()
{
    model->addBlankRow();
}

void ModuleTransformWidget::onParametersUpdated()
{
    transform->setParameters(model->getParameters());
}

void ModuleTransformWidget::reloadParameters()
{
    model->setParameters(transform->getParameters());
}

bool ModuleTransformWidget::eventFilter(QObject *sender, QEvent *event)
{
    if (sender == ui->parameterstableView && event->type() == QEvent::KeyPress) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key())
        {
            case Qt::Key_Delete:
                {
                    qDebug() << "TableView eventFilter DEL key pressed";
                    QModelIndexList list = ui->parameterstableView->selectionModel()->selectedRows();
                    if (list.size() >= 1) {
                        model->removeRows(list.at(0).row(),1, list.at(0).parent());
                    }
                }
               break;
            default:
                return false;
        }
    }
    return true;
}

ParametersItemModel::ParametersItemModel(QObject *parent):
    QAbstractTableModel(parent)
{

}

ParametersItemModel::~ParametersItemModel()
{

}

int ParametersItemModel::columnCount(const QModelIndex &) const
{
    return 2;
}

int ParametersItemModel::rowCount(const QModelIndex &) const
{
    return parametersNames.size();
}

QVariant ParametersItemModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= parametersNames.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        if (index.row() < parametersNames.size()) {
            if (index.column() == 0) {
                return parametersNames.at(index.row());
            } else if (index.row() < parametersValues.size()) {
                return parametersValues.at(index.row());
            }
        }
    }

    return QVariant();
}

QVariant ParametersItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            return section == 0 ? QString("Name") : QString("Value");
        }
    } else if (orientation == Qt::Vertical) {
        if (role == Qt::DisplayRole) {
            return QString::number(section);
        }
    }

    return QVariant();
}

Qt::ItemFlags ParametersItemModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool ParametersItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() >= parametersNames.size()) {
        return false;
    }

    if (role == Qt::EditRole) {
        if (index.column() == 0) {
            if (parametersNames.contains(value.toString())) {
                QMessageBox::critical(NULL,tr("Error"), tr("Parameter name already exists"),QMessageBox::Ok);
                return false;
            }
            if (parametersNames.at(index.row()) != value.toString()) {
                parametersNames.replace(index.row(), value.toString());
                emit parametersChanged();
                return true;
            }
        } else {
            if (parametersValues.at(index.row()) != value.toString()) {
                parametersValues.replace(index.row(), value.toString());
                emit parametersChanged();
                return true;
            }
        }
    }

    return false;
}

void ParametersItemModel::addBlankRow()
{
    QString empty;
    if (parametersNames.contains(empty)) {
        QMessageBox::critical(NULL,tr("Error"), tr("Empty parameter is available, use this one first"),QMessageBox::Ok);
        return;
    }
    beginResetModel();

    parametersNames.append(empty);
    parametersValues.append(empty);
    endResetModel();
}

bool ParametersItemModel::removeRows(int row, int count, const QModelIndex &)
{
    if (row >= parametersNames.size()) {
            return false;
    }

    beginResetModel();

    if (count > 1) {
        qWarning() << "More than one row selected for deletion, removing only the first one.";
    }

    parametersNames.removeAt(row);
    parametersValues.removeAt(row);

    endResetModel();

    return true;
}

void ParametersItemModel::setParameters(QHash<QByteArray, QByteArray> newParameters)
{
    beginResetModel();
    parametersNames.clear();
    parametersValues.clear();
    QHashIterator<QByteArray, QByteArray> i(newParameters);
     while (i.hasNext()) {
         i.next();
         parametersNames.append(QString::fromUtf8(i.key()));
         parametersValues.append(QString::fromUtf8(i.value()));
     }
     endResetModel();
}

QHash<QByteArray, QByteArray> ParametersItemModel::getParameters()
{

    QHash<QByteArray, QByteArray> finalParams;
    for (int i = 0; i < parametersNames.size(); i++) {
        finalParams.insert(parametersNames.at(i).toUtf8(), parametersValues.at(i).toUtf8());
    }

    return finalParams;
}
