/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_replace.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_REPLACE_H
#define DLG_REPLACE_H

#include <QDialog>

namespace Ui {
class DlgReplace;
}

class MainWindow;
class QStandardItem;
class TextTableWidget;
class DlgReplace : public QDialog
{
    Q_OBJECT

public:
    explicit DlgReplace(QWidget *parent = nullptr);
    ~DlgReplace();

    void attach(MainWindow*);
protected:
    void showEvent(QShowEvent *event) override;
private:
    void locate(bool prev);
    void locate_replace(bool prev);
    void onAll();
    bool prevGroup();
    bool nextGroup();
private:
    Ui::DlgReplace *ui;
    MainWindow* _mw;

    bool _last_is_prev; // 上一次点击的是上一个
    int _last_row;
    int _pos,_len;  // 当前条目中匹配的内容位置
    QStandardItem* _cur_item;
};

#endif // DLG_REPLACE_H
