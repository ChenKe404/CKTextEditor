/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_decide_kick.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_decide_kick.h"
#include "ui_dlg_decide_kick.h"

DlgDecideKick::DlgDecideKick(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::DlgDecideKick),
    _use_retain(true)
{
    ui->setupUi(this);
    _fmt_retain = ui->lab_retain->text();
    _fmt_kick = ui->lab_kick->text();

    connect(ui->btn_retain,&QPushButton::clicked,this,[this](){
        _use_retain = true;
        accept();
    });
    connect(ui->btn_kick,&QPushButton::clicked,this,[this](){
        _use_retain = false;
        accept();
    });
}

DlgDecideKick::~DlgDecideKick()
{
    delete ui;
}

void DlgDecideKick::setName(const QString &retain, const QString &kick)
{
    ui->lab_retain->setText(_fmt_retain.arg(retain));
    ui->lab_kick->setText(_fmt_kick.arg(kick));
}

void DlgDecideKick::set(const std::string &src, const std::string &trs_retain, const std::string &trs_kick)
{
    ui->edt_src->setPlainText(src.c_str());
    ui->edt_trs_retain->setPlainText(trs_retain.c_str());
    ui->edt_trs_kick->setPlainText(trs_kick.c_str());
}

bool DlgDecideKick::useRetain() const
{
    return _use_retain;
}

bool DlgDecideKick::following() const
{
    return ui->chb_following->isChecked();
}
