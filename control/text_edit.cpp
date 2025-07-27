/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	text_edit.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "text_edit.h"
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>

TextEdit::TextEdit(QWidget *parent)
    : Super(parent)
{}

void TextEdit::setCharDraw(CharDraw *cd)
{
    if(!cd) return;
    auto iter = _cds.find(cd->ch());
    if(iter != _cds.end())
        iter->second->deleteLater();
    if(!cd->parent())
        cd->setParent(this);
    _cds[cd->ch()] = cd;
}

void TextEdit::paintEvent(QPaintEvent *e)
{
    QPlainTextEdit::paintEvent(e);  // 保留默认绘制
    const int vph = viewport()->height();
    const auto sbv = verticalScrollBar()->value();
    auto doc = document();
    auto docLayout = doc->documentLayout();

    QPainter p(viewport());
    p.setRenderHint(QPainter::Antialiasing);

    auto block = firstVisibleBlock();
    while (block.isValid()) {
        const auto rect = docLayout->blockBoundingRect(block);
        const auto offset = contentOffset();    // 滚动偏移

        if (rect.top() - sbv > vph)
            break;

        auto blockRc = blockBoundingGeometry(block);
        // 块的起始位置
        const auto bx = blockRc.x();
        const auto by = blockRc.y();

        auto next = block.next();

        const auto blockPos = block.position(); // 块的起始文本位置
        auto layout = block.layout();
        const auto lineCount = layout->lineCount();
        for (int n = 0; n < lineCount; ++n)
        {
            auto line = layout->lineAt(n);
            if(!line.isValid()) continue;

            const auto docPos = blockPos + line.textStart();    // 行在文档的起始位置
            const auto textLen = line.textLength();
            for (int i = 0; i <= textLen; ++i)
            {
                auto idx = i + docPos;
                auto ch = doc->characterAt(idx);

                // 是否需要处理换行符
                if(i == textLen)
                {
                    // 不是最后一块, 并且必须是块的最后一行
                    if(!next.isValid() || n != lineCount - 1)
                        continue;
                    ch = '\n';
                }
                else if(ch == QChar::Tabulation)
                    ch = '\t';

                auto iter = _cds.find(ch);
                if(iter == _cds.end() || !iter->second->enabled())
                    continue;

                idx -= blockPos;    // 相对于块的字符索引
                qreal x = line.cursorToX(idx) + bx;
                qreal y = line.y() + (line.height() - line.ascent()) * 0.5 + by;
                qreal w = line.cursorToX(idx,QTextLine::Trailing) - x;
                if(ch == '\t' && w < 1)  // 制表符可能得不到宽度
                {
                    if(idx != textLen - 1)
                        w = line.cursorToX(idx + 1) - x;
                    else
                        w = 6;
                }
                QRectF rc {
                    x + offset.x(),
                    y + offset.y(),
                    w,
                    line.ascent()
                };

                iter->second->draw(p,rc);
            }
        }

        block = next;
    }
}

/*
void TextEdit::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Tab)
    {
        QTextCursor cursor = textCursor();
        int pos = cursor.position();
        if(pos >= 2)
        {
            QString text = toPlainText();
            const auto len = text.length();
            auto last = text[pos - 1];
            auto slash = text[pos - 2];
            if(slash == '\\' && last.isLetter())
            {
                char c = 0;
                switch (last.toLatin1()) {
                case 'f':   //翻页
                    c = '\f';
                case 'r':   //回车
                    if(!c) c = '\r';
                case 'n':   //换行
                    if(!c) c = '\n';
                case 't':   //水平制表
                    if(!c) c = '\t';
                    pos -= 2;
                    text.remove(pos,2);
                    text.insert(pos,c);
                    setPlainText(text);
                    cursor.setPosition(pos);
                    setTextCursor(cursor);
                    return;
                default: break;
                }
            }
        }
    }
    QPlainTextEdit::keyPressEvent(e);
}
*/
