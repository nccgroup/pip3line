/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TEXTINPUTDIALOG_H
#define TEXTINPUTDIALOG_H

#include <QDialog>
#include <QPixmap>

namespace Ui {
class textInputDialog;
}

class TextInputDialog : public QDialog
{
        Q_OBJECT
        
    public:
        explicit TextInputDialog(QWidget *parent = 0);
        ~TextInputDialog();
        void setTextLabel(QString text);
        void setPixLabel(QPixmap pix);
        QString getInputText() const;
        void setText(QString text);
        
    private:
        Q_DISABLE_COPY(TextInputDialog)
        Ui::textInputDialog *ui;
};

#endif // TEXTINPUTDIALOG_H
