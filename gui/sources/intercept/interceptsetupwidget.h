#ifndef INTERCEPTSETUPWIDGET_H
#define INTERCEPTSETUPWIDGET_H

#include <QWidget>
#include <QStringList>
#include "../blocksources/blockssource.h"
#include "interceptorchestratorabstract.h"

namespace Ui {
    class InterceptSetUpWidget;
}

class InterceptSetUpWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit InterceptSetUpWidget(QWidget *parent = 0);
        ~InterceptSetUpWidget();
        QStringList getUnaryList(bool includeReflexive = true);

    private:
        Ui::InterceptSetUpWidget *ui;
        static const QStringList TypeList;
        QStringList UnaryList;
        InterceptOrchestratorAbstract *leftSource;
};

#endif // INTERCEPTSETUPWIDGET_H
