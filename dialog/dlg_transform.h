/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_transform.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_TRANSFORM_H
#define DLG_TRANSFORM_H

#include <QDialog>

namespace Ui {
class DlgTransform;
}

class DlgTransform : public QDialog
{
    Q_OBJECT

public:
    explicit DlgTransform(ck::Text* text, QWidget *parent = nullptr);
    ~DlgTransform();

    void attach(ck::Text* text);

    void accept() override;
private:
    void onFile();
private:
    Ui::DlgTransform *ui;
    ck::Text _text;
    ck::Text* _text_to;
};

#endif // DLG_TRANSFORM_H
