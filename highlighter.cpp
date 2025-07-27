/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	highlighter.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "highlighter.h"

void Highlighter::highlightBlock(const QString &text)
{
    int num = 0;
    for(auto& c : text)
    {
        if(c == ' ')
        {
            QTextCharFormat fmt;
            // fmt.setAnchorHref("//");
            // fmt.setAnchorHref()
            fmt.setTextOutline(QColor(0,0,0));
            fmt.setBackground(QColor(255,220,0));
            setFormat(num,1,fmt);
        }
        else if(c == '\r')
        {}
        else if(c == '\n')
        {}
        ++num;
    }
}
