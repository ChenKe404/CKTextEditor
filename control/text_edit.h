/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	text_edit.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef TEXT_EDIT_H
#define TEXT_EDIT_H

#include <QPlainTextEdit>
#include "char_draw.h"

class TextEdit : public QPlainTextEdit
{
    using Super = QPlainTextEdit;
public:
    explicit TextEdit(QWidget *parent = nullptr);
    void setCharDraw(CharDraw* cd);
protected:
    void paintEvent(QPaintEvent *event) override;
    // void keyPressEvent(QKeyEvent *event) override;

    std::map<QChar,CharDraw*> _cds;
};

#endif // TEXT_EDIT_H
