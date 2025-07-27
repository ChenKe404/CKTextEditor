/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_find.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_find.h"
#include "ui_dlg_find.h"
#include "../control/text_tablewidget.h"

DlgFind::DlgFind(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::DlgFind),
    _table(nullptr)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    connect(ui->btn_find,&QPushButton::clicked,this,[this](){
        if(!_table) return;
        _table->filter(ui->edt_content->text(),flags() | TextTableWidget::ONCE);
    });
    connect(ui->btn_filter,&QPushButton::clicked,this,[this](){
        if(!_table) return;
        _table->filter(ui->edt_content->text(),flags());
    });
    connect(ui->chk_src,&QCheckBox::checkStateChanged,this,[this](Qt::CheckState st){
        if(st == Qt::Unchecked && !ui->chk_trans->isChecked())
            ui->chk_trans->setChecked(true);
    });
    connect(ui->chk_trans,&QCheckBox::checkStateChanged,this,[this](Qt::CheckState st){
        if(st == Qt::Unchecked && !ui->chk_src->isChecked())
            ui->chk_src->setChecked(true);
    });
}

DlgFind::~DlgFind()
{
    delete ui;
}

void DlgFind::attach(TextTableWidget *table)
{
    _table = table;
}

uint32_t DlgFind::flags() const
{
    uint32_t ret = 0;
    if(ui->chk_src->isChecked())
        ret |= TextTableWidget::SRC;
    if(ui->chk_trans->isChecked())
        ret |= TextTableWidget::TRS;
    if(ui->chk_ignore_case->isChecked())
        ret |= TextTableWidget::IGNORE_CASE;
    return ret;
}
