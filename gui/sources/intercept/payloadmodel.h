/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PAYLOADMODEL_H
#define PAYLOADMODEL_H

#include "payload.h"
#include <QObject>
#include <QStringList>

class PayloadModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        static const int TIMESPTAMP_COLUMN;
        static const int DIRECTION_COLUMN;
        static const int PAYLOAD_COLUMN;
        explicit PayloadModel(QObject * parent = 0);
        ~PayloadModel();
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        void addPayload(Payload * payload);
        Payload *getPayload(int i);
        QStringList getColumnNames() const;
        void setColumnNames(const QStringList &value);
    public slots:
        void clear();
    private:
        QList<Payload *> payloadList;
        QStringList columnNames;

};

#endif // PAYLOADMODEL_H
