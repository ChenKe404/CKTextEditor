/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	text_tablewidget.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_about.h"
#include "ui_dlg_about.h"

DlgAbout::DlgAbout(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgAbout)
{
    ui->setupUi(this);

    if(strlen(PRJ_ARCH) == 0)
        ui->lab_name->setText("CKTextEditor");
    else
        ui->lab_name->setText("CKTextEditor (" PRJ_ARCH ")");

    auto fmt = ui->lab_version->text();
    ui->lab_version->setText(fmt.arg(PRJ_VERSION));

    fmt = ui->lab_build->text();
    QString str("Qt %1.%2.%3 at %4");
    str =  str.arg(QT_VERSION_MAJOR)
              .arg(QT_VERSION_MINOR)
              .arg(QT_VERSION_PATCH)
              .arg(QString(__DATE__)+" "+__TIME__);
    ui->lab_build->setText(fmt.arg(str));

    fmt=ui->lab_licence->text();
    ui->lab_licence->setText(fmt.arg(PRJ_LICENCE));

    ui->lab_repos->setOpenExternalLinks(true);
    str.clear();
    auto sp = QString(PRJ_REPOS).split('|');
    for(auto& it : sp)
    {
        if(it.isEmpty())
            continue;
        str.append(QString("<a href=\"%1\">%1</a><br/>").arg(it));
    }
    if(!str.isEmpty())
    {
        str.remove(str.size()-5,5);
    }
    ui->lab_repos->setText(str);
}

DlgAbout::~DlgAbout()
{
    delete ui;
}
