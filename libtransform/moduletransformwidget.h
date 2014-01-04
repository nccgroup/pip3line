/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MODULETRANSFORMWIDGET_H
#define MODULETRANSFORMWIDGET_H

#include "libtransform_global.h"
#include <QWidget>
#include <QList>

namespace Ui {
class ModuleTransformWidget;
}
class ScriptTransformAbstract;

#include <QAbstractTableModel>
#include <QHash>


class ParametersItemModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        explicit ParametersItemModel( QObject *parent = 0);
        ~ParametersItemModel();
        int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
        void addBlankRow();
        bool removeRows(int row, int count, const QModelIndex &);
        void setParameters(QHash<QByteArray, QByteArray> newParameters);
        QHash<QByteArray, QByteArray> getParameters();
    signals:
        void parametersChanged();
    private:
        Q_DISABLE_COPY(ParametersItemModel)
        QList<QString> parametersNames;
        QList<QString> parametersValues;

};

class LIBTRANSFORMSHARED_EXPORT ModuleTransformWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit ModuleTransformWidget(ScriptTransformAbstract *transform, QWidget *parent = 0);
        ~ModuleTransformWidget();
    public slots:
        void reloadParameters();
    private slots:
        void onChooseFile();
        void onMakePersistent(bool checked);
        void onAddParameter();
        void onParametersUpdated();
        void onAutoReload(bool val);
    private:
        Q_DISABLE_COPY(ModuleTransformWidget)
        bool eventFilter(QObject *sender, QEvent *event);
        Ui::ModuleTransformWidget *ui;
        ScriptTransformAbstract *transform;
        ParametersItemModel *model;
};

#endif // MODULETRANSFORMWIDGET_H
