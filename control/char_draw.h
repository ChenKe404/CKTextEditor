/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	char_draw.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef CHAR_DRAW_H
#define CHAR_DRAW_H

struct CharDraw : public QObject
{
    inline CharDraw(QChar ch,QObject* parent = nullptr)
        : _ch(ch),QObject(parent)
    {}
    inline QChar ch() const {
        return _ch;
    }
    inline bool enabled() const {
        return _enabled;
    }
    inline void setEnable(bool yes){
        _enabled = yes;;
    }
    virtual void draw(QPainter& p,QRectF rc) = 0;
private:
    QChar _ch = ' ';
    bool _enabled = true;
};

class CharDrawSpace : public CharDraw
{
    using Super = CharDraw;
public:
    using Super::CharDraw;
    void draw(QPainter &p, QRectF rc) override;
};

class CharDrawLineFeed : public CharDraw
{
    using Super = CharDraw;
public:
    CharDrawLineFeed(QObject* parent);
    void draw(QPainter &p, QRectF rc) override;
private:
    QFont _font;
    QString _tag;
};

#endif // CHAR_DRAW_H
