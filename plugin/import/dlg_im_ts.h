/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_im_ts.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_IM_TS_H
#define DLG_IM_TS_H

#include <QDialog>

namespace ck
{
class Text;
}

namespace Ui {
class DlgImTS;
}

class DlgImTS : public QDialog
{
    Q_OBJECT

public:
    explicit DlgImTS(ck::Text*,QWidget *parent = nullptr);
    ~DlgImTS();

    void accept() override;
private:
    void onGameLanguage();
    void onIniAdd();
    void onIniRemove();
    void onIniUp();
    void onIniDown();
    void handleDLL(const QString&);
    void handleINI(const QStringList&);
private:
    Ui::DlgImTS *ui;
    ck::Text* _text;
};

#endif // DLG_IM_TS_H
