/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_decide.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_DECIDE_H
#define DLG_DECIDE_H

#include <QDialog>

namespace Ui {
class DlgDecide;
}

class DlgDecide : public QDialog
{
    Q_OBJECT

public:
    explicit DlgDecide(QWidget *parent = nullptr);
    ~DlgDecide();

    // 设置组名
    void setName(const QString& merged,const QString& merging);
    void set(const std::string& src,const std::string& merged,const std::string& merging);
    // 是否使用已合并的
    bool useMerged() const;
    // 之后所有相同的译文是否也这么做
    bool following() const;
private:
    Ui::DlgDecide *ui;
    QString _fmt_merged, _fmt_merging;
    bool _use_merged;
};

#endif // DLG_DECIDE_H
