/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_auto_help.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_auto_help.h"
#include "ui_dlg_auto_help.h"

DlgAutoHelp::DlgAutoHelp(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgAutoHelp)
{
    ui->setupUi(this);
}

DlgAutoHelp::~DlgAutoHelp()
{
    delete ui;
}
