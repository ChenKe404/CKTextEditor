/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_find.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_FIND_H
#define DLG_FIND_H

#include <QDialog>

namespace Ui {
class DlgFind;
}

class TextTableWidget;
class DlgFind : public QDialog
{
    Q_OBJECT

public:
    explicit DlgFind(QWidget *parent = nullptr);
    ~DlgFind();

    void attach(TextTableWidget*);
private:
    uint32_t flags() const;
private:
    Ui::DlgFind *ui;
    TextTableWidget* _table;
};

#endif // DLG_FIND_H
