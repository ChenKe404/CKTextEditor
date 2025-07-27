/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_decide_kick.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_DECIDE_KICK_H
#define DLG_DECIDE_KICK_H

#include <QDialog>

namespace Ui {
class DlgDecideKick;
}

class DlgDecideKick : public QDialog
{
    Q_OBJECT

public:
    explicit DlgDecideKick(QWidget *parent = nullptr);
    ~DlgDecideKick();

    // 设置组名
    void setName(const QString& retain,const QString& kick);
    // 设置原文和译文
    void set(const std::string& src,const std::string& trs_retain,const std::string& trs_kick);
    // 是否使用保留组的
    bool useRetain() const;
    // 是否之后的也都这么做
    bool following() const;
private:
    Ui::DlgDecideKick *ui;
    QString _fmt_retain, _fmt_kick;
    bool _use_retain;
};

#endif // DLG_DECIDE_KICK_H
