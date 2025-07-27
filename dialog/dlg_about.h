/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_about.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_ABOUT_H
#define DLG_ABOUT_H

#include <QDialog>

namespace Ui {
class DlgAbout;
}

class DlgAbout : public QDialog
{
    Q_OBJECT

public:
    explicit DlgAbout(QWidget *parent = nullptr);
    ~DlgAbout();

private:
    Ui::DlgAbout *ui;
};

#endif // DLG_ABOUT_H
