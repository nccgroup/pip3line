/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef ANALYSE_H
#define ANALYSE_H

#include <QDialog>
#include <QMutex>
#include <QMenu>

namespace Ui {
class AnalyseDialog;
}

class AnalyseDialog : public QDialog
{
        Q_OBJECT
        
    public:
        explicit AnalyseDialog(QWidget *parent = 0);
        ~AnalyseDialog();

    signals:
        void results(QString);
private slots:
        void gatheringResults(QString);
        void cleaning();
        void onInputRightClick(QPoint pos);
        void on_actionImportFile_triggered();
        void onCharsetAnalysis();
        void charSetAnalysis();
        void onCharFrequencyAnalysis();
        void charFrequencyAnalysis();
        void onHashGuess();
        void hashGuess();

    private:
        Q_DISABLE_COPY(AnalyseDialog)
        QString potentialHashes(int length);
        QList<QByteArray> getInput();
        Ui::AnalyseDialog *ui;
        QMutex lock;
        QMenu * plainTextContextMenu;
};

#endif // ANALYSE_H
