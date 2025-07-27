/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	char_draw.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "char_draw.h"
#include <QPainter>

void CharDrawSpace::draw(QPainter &p, QRectF rc)
{
    rc.adjust(1,0,-1,0);
    if(rc.height() > 12)
        rc.adjust(0,2,0,-2);
    if(rc.width() > 6)
    {
        p.setPen(QPen(QColor(150,150,150),1,Qt::DashLine));
        p.drawRect(rc);
    }
    p.fillRect(rc,QColor(250,200,0));
}

CharDrawLineFeed::CharDrawLineFeed(QObject *parent)
    : Super('\n',parent),_tag("LF")
{
    _font.setFamily("arial");
}

void CharDrawLineFeed::draw(QPainter &p, QRectF rc)
{
    auto old = p.font();
    _font.setPointSize(old.pointSize() - 3);
    p.setFont(_font);

    QFontMetrics m(_font);
    QRectF _rc = m.boundingRect(_tag);
    _rc.moveTo(rc.x() + 2,rc.top() + (rc.height() - _rc.height()) * 0.5);
    auto pt = _rc.bottomLeft();
    _rc.adjust(-1,-1,2,1);
    p.fillRect(_rc,QColor(60,60,60));
    p.setPen(QColor(255,255,255));
    p.drawText(pt + QPointF{ 0,-1 },_tag);

    p.setFont(old);
}
