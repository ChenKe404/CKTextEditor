/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_attr.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_ATTR_H
#define DLG_ATTR_H

#include <QDialog>

namespace Ui {
class DlgAttr;
}

class DlgAttr : public QDialog
{
    Q_OBJECT

public:
    explicit DlgAttr(QWidget *parent = nullptr);
    ~DlgAttr();

    void accept() override;

    QString name() const;
    const QVariant& value() const;

    void onType(int idx);
private:
    Ui::DlgAttr *ui;
    QVariant _value;
};

#endif // DLG_ATTR_H
