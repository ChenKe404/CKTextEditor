/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_edit.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_edit.h"
#include "ui_dlg_edit.h"

DlgEdit::DlgEdit(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::DlgEdit),
    _cb(nullptr)
{
    ui->setupUi(this);
    connect(ui->btn_ok,&QPushButton::clicked,this,[this](){
        auto text = ui->edt_text->toPlainText();
        if(!_can_empty)
        {
            if(text.isEmpty())
            {
                Warning(tr("不能为空!"));
                return;
            }
        }
        if(_cb && !_cb(text))
            return;
        QDialog::accept();
    });
}

DlgEdit::~DlgEdit()
{
    delete ui;
}

void DlgEdit::setCanEmpty(bool yes)
{
    _can_empty = yes;
}

void DlgEdit::setText(const QString &text)
{
    ui->edt_text->setPlainText(text);
    ui->edt_text->moveCursor(QTextCursor::End);
}

QString DlgEdit::text() const
{
    return ui->edt_text->toPlainText();
}

void DlgEdit::setCharDraw(CharDraw *cd)
{
    ui->edt_text->setCharDraw(cd);
}

void DlgEdit::onCheck(const CBCheck &cb)
{
    _cb = cb;
}
