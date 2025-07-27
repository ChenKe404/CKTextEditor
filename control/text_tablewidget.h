/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	text_tablewidget.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef TEXT_TABLEWIDGET_H
#define TEXT_TABLEWIDGET_H

#include <QMenu>
#include <QTableView>
#include <cktext/text.h>

class QStandardItem;
class TextTableWidget : public QTableView
{
    Q_OBJECT
public:
    enum Sort{
        ASC,    // 升序
        DESC,   // 降序
        UNFINISHED_TOPMOST,  // 置顶未完成的
        SHORT2LONG, // 短到长
        LONG2SHORT  // 长到短
    };
    enum FilterFlag
    {
        SRC = 1,    // 原文
        TRS = 2,    // 译文
        IGNORE_CASE = 4,    // 忽略大小写
        ONCE = 8    // 只执行一次, 重新加载页面后清除过滤
    };
public:
    TextTableWidget(QWidget* parent = nullptr);

    // @return 条目数
    int attach(ck::Text::Group*);
    void detach();
    ck::Text::Group* group() const;

    void setInlineEdit(bool yes);
    void setTextAlignment(Qt::Alignment);
    // 定位到行并返回译文项
    QStandardItem* setCurrentRow(int row);
    // @row 从此行开始查找
    // @last 向前查找
    // @return 下一个匹配的行, 没找到则返回-1
    int find(int row,const QString& keyword,uint32_t flag,bool last);
    QModelIndex indexFromItem(QStandardItem*);

    void clear();
    // @return 条目数
    int reload();

    void filter(const QString& text,uint32_t flag);

    void sort(Sort);
    void showSpace(bool yes);
    void showLineFeed(bool yes);
signals:
    void srcChanged(const QString& old,const QString& src,const QString& trs);
    void trsChanged(const QString& src,const QString& trs);
protected:
    void contextMenuEvent(QContextMenuEvent *ev) override;
    using QTableView::setModel;
private:
    ck::Text::Group* _group;
    QMenu _menu;
    uint32_t _filter_flag = 0;
};

#endif // TEXT_TABLEWIDGET_H
