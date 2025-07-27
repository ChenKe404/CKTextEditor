/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_decide.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_decide.h"
#include "ui_dlg_decide.h"

DlgDecide::DlgDecide(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::DlgDecide),
    _use_merged(true)
{
    ui->setupUi(this);
    _fmt_merged = ui->lab_merged->text();
    _fmt_merging = ui->lab_merging->text();

    connect(ui->btn_merged,&QPushButton::clicked,this,[this](){
        _use_merged = true;
        accept();
    });
    connect(ui->btn_merging,&QPushButton::clicked,this,[this](){
        _use_merged = false;
        accept();
    });
}

DlgDecide::~DlgDecide()
{
    delete ui;
}

void DlgDecide::setName(const QString &merged, const QString &merging)
{
    ui->lab_merged->setText(_fmt_merged.arg(merged));
    ui->lab_merging->setText(_fmt_merging.arg(merging));
}

void DlgDecide::set(const std::string &src, const std::string &merged, const std::string &merging)
{
    ui->edt_src->setPlainText(src.c_str());
    ui->edt_merged->setPlainText(merged.c_str());
    ui->edt_merging->setPlainText(merging.c_str());
}

bool DlgDecide::useMerged() const
{
    return _use_merged;
}

bool DlgDecide::following() const
{
    return ui->chb_following->isChecked();
}
