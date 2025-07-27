/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	highlighter.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>

class Highlighter : public QSyntaxHighlighter
{
    using Super = QSyntaxHighlighter;
public:
    using Super::QSyntaxHighlighter;
protected:
    void highlightBlock(const QString &text) override;
};

#endif // HIGHLIGHTER_H
