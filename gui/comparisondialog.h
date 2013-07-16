#ifndef COMPARISONDIALOG_H
#define COMPARISONDIALOG_H

#include <QDialog>
#include "guihelper.h"
#include "transformsgui.h"
#include <QList>
#include <QComboBox>
#include <QColor>

namespace Ui {
class ComparisonDialog;
}

class ComparisonDialog : public QDialog
{
        Q_OBJECT
        
    public:
        explicit ComparisonDialog(GuiHelper *guiHelper ,QWidget *parent = 0);
        ~ComparisonDialog();
    private slots:
        void onTabSelection(int index);
        void onCompare();
        void loadTabs();
        void oncolorChange();
    private:
        static const QColor DEFAULT_MARKING_COLOR;
        void refreshEntries(QComboBox *entryBox, int count);
        void refreshTabs(QComboBox *tabBox);
        void changeIconColor(QColor color);
        Ui::ComparisonDialog *ui;
        GuiHelper *guiHelper;
        QList<TransformsGui *> tabs;
        QColor marksColor;
};

#endif // COMPARISONDIALOG_H
