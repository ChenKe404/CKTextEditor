/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_replace.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_replace.h"
#include "ui_dlg_replace.h"
#include "../control/text_tablewidget.h"

#include <QShowEvent>
#include <QStandardItem>
#include "../mainwindow.h"
#include "../ui_mainwindow.h"

DlgReplace::DlgReplace(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::DlgReplace),
    _mw(nullptr),
    _last_row(-1),_pos(0),_len(0),
    _cur_item(nullptr),
    _last_is_prev(false)
{
    ui->setupUi(this);
    // setWindowFlag(Qt::WindowStaysOnTopHint);
    connect(ui->btn_prev,&QPushButton::clicked,this,[this]() { locate_replace(true); });
    connect(ui->btn_next,&QPushButton::clicked,this,[this]() { locate_replace(false); });
    connect(ui->btn_all,&QPushButton::clicked,this,&DlgReplace::onAll);
}

DlgReplace::~DlgReplace()
{
    delete ui;
}

void DlgReplace::attach(MainWindow* mw)
{
    _mw = mw;
}

void DlgReplace::showEvent(QShowEvent *e)
{
    _mw->ui->table->filter("",0);   // 清空过滤效果
}

void DlgReplace::locate(bool prev)
{
    auto from = ui->edt_from->text();
    if(from.isEmpty())
        return;
    uint32_t flag = TextTableWidget::TRS;
    if(ui->chk_icase->isChecked())
        flag |= TextTableWidget::IGNORE_CASE;

    auto table = _mw->ui->table;
    // 定位指定行的译文项
    if(!_cur_item)
    {
        auto m = table->model();
        auto row = table->currentIndex().row();
        if(prev)
        {
            row = table->find(row,from,flag,true);
            if(row < 0 && prevGroup())
                row = table->find(m->rowCount() - 1,from,flag,true);
        }
        else
        {
            row = table->find(row,from,flag,false);
            if(row < 0 && nextGroup())
                row = table->find(0,from,flag,false);
        }
        _last_row = row;
        _cur_item = table->setCurrentRow(row);
    }

    // 查找条目中的内容位置
    if(_cur_item)
    {
        auto cases = ui->chk_icase->isChecked() ? Qt::CaseInsensitive : Qt::CaseSensitive;
        auto text = _cur_item->data(Qt::EditRole).toString();
        if(prev)
            _pos = text.lastIndexOf(from,cases);
        else
            _pos = text.indexOf(from,0,cases);
        auto b = text.contains(from);
        _len = from.length();
        if(_pos < 0) _cur_item = nullptr;
    }
}

void DlgReplace::locate_replace(bool prev)
{
    auto table = _mw->ui->table;
    auto row = table->currentIndex().row();
    if(row < 0)
    {
        table->setCurrentRow(0);
        row = 0;
    }
    if(row != _last_row)
        _cur_item = nullptr;
    if(_last_is_prev != prev)
        _pos = -1;

    if(!_cur_item || _pos < 0)
        locate(prev);
    else if(_cur_item && _pos >= 0)    // 执行替换
    {
        auto to = ui->edt_to->text();
        auto text = _cur_item->data(Qt::EditRole).toString();
        text.replace(_pos,_len,to);

        auto m = table->model();
        auto idx = table->indexFromItem(_cur_item);
        m->setData(idx,text);

        auto cases = ui->chk_icase->isChecked() ? Qt::CaseInsensitive : Qt::CaseSensitive;
        auto from = ui->edt_from->text();
        if(prev)
            _pos = text.lastIndexOf(from,cases);
        else
            _pos = text.indexOf(from,0,cases);
        if(_pos < 0) _cur_item = nullptr;
        _len = from.length();
    }
    _last_is_prev = prev;
}

void DlgReplace::onAll()
{
    auto table = _mw->ui->table;
    auto& text = _mw->_text;
    const auto cases = ui->chk_icase->isChecked() ? Qt::CaseInsensitive : Qt::CaseSensitive;
    const auto from = ui->edt_from->text();
    const auto to = ui->edt_to->text();

    int num = 0;
    QString t;
    if(ui->chk_current_only->isChecked())
    {
        auto& grp = *table->group();
        for(auto& it : grp)
        {
            t = it.second.c_str();
            bool flag = false;
            auto pos = 0;
            while((pos = t.indexOf(from,pos,cases)) >= 0)
            {
                t.replace(pos,from.length(),to);
                flag = true;
                ++num;
            }
            if(flag)
                grp.set(it.first.c_str(),cstr(t));
        }
    }
    else
    {
        for(auto& grp : text)
        {
            for(auto& it : grp.second)
            {
                t = it.second.c_str();
                bool flag = false;
                auto pos = 0;
                while((pos = t.indexOf(from,pos,cases)) >= 0)
                {
                    t.replace(pos,from.length(),to);
                    flag = true;
                    ++num;
                }
                if(flag)
                {
                    auto _grp = const_cast<ck::Text::Group*>(&grp.second);
                    _grp->set(it.first.c_str(),cstr(t));
                }
            }
        }
    }
    table->reload();
    Information(tr("替换了%1处").arg(num));
}

bool DlgReplace::prevGroup()
{
    auto cob = _mw->ui->cob_group;
    auto idx = cob->currentIndex();
    if(idx < 1) return false;
    cob->setCurrentIndex(idx - 1);
    return true;
}

bool DlgReplace::nextGroup()
{
    auto cob = _mw->ui->cob_group;
    auto idx = cob->currentIndex();
    if(idx >= cob->count() - 1) return false;
    cob->setCurrentIndex(idx + 1);
    return true;
}
