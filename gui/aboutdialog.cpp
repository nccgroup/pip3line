/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "../version.h"
#include <commonstrings.h>
#include <QStyleFactory>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->versionLabel->setText(tr("%1 %2 (libtransform v%3)").arg(APPNAME).arg(VERSION_STRING).arg(LIB_TRANSFORM_VERSION));
    QString info;

    info.append(tr("<p>Compiled with <b>Qt %1</b> (currently running with Qt %2)</p>").arg(QT_VERSION_STR).arg(qVersion()));
    info.append(tr("<p>Style(s) available on this platform: <ul>"));
    QStringList stylelist = QStyleFactory::keys();
    for (int i = 0; i < stylelist.size(); i++) {
        info.append("<li>");
        info.append(stylelist.at(i));
        info.append("</li>");
    }
    info.append("</ul></p>");
    ui->infoTextEdit->append(info);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
