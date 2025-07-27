/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_auto_help.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_AUTO_HELP_H
#define DLG_AUTO_HELP_H

#include <QDialog>

namespace Ui {
class DlgAutoHelp;
}

class DlgAutoHelp : public QDialog
{
    Q_OBJECT

public:
    explicit DlgAutoHelp(QWidget *parent = nullptr);
    ~DlgAutoHelp();

private:
    Ui::DlgAutoHelp *ui;
};

#endif // DLG_AUTO_HELP_H
