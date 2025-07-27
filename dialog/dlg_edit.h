/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_edit.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_EDIT_H
#define DLG_EDIT_H

#include <QDialog>
#include <control/char_draw.h>

namespace Ui {
class DlgEdit;
}

class DlgEdit : public QDialog
{
    Q_OBJECT
    using CBCheck = std::function<bool(const QString&)>;
public:
    explicit DlgEdit(QWidget *parent = nullptr);
    ~DlgEdit();

    void setCanEmpty(bool yes);
    void setText(const QString&);
    QString text() const;

    void setCharDraw(CharDraw*);

    void onCheck(const CBCheck& cb);
private:
    Ui::DlgEdit *ui;
    CBCheck _cb;
    bool _can_empty = true;
};

#endif // DLG_EDIT_H
