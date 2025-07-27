/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_transform.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_transform.h"
#include "ui_dlg_transform.h"
#include <QFileDialog>

DlgTransform::DlgTransform(ck::Text* text,QWidget *parent)
    : QDialog(parent),
    ui(new Ui::DlgTransform),
    _text_to(text)
{
    ui->setupUi(this);
    for(auto& it : *text)
    {
        QString name = it.first.c_str();
        QString text = it.first.empty() ? tr("默认组") : name;
        auto item = new QListWidgetItem(text);
        item->setData(Qt::UserRole + 1, name);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        ui->list->addItem(item);
    }

    connect(ui->btn_browse,&QPushButton::clicked,this,&DlgTransform::onFile);
}

DlgTransform::~DlgTransform()
{
    delete ui;
}

void DlgTransform::attach(ck::Text *text)
{
    _text_to = text;
}

void DlgTransform::accept()
{
    auto path = ui->edt_path->text().toLocal8Bit().toStdString();
    if(path.empty())
    {
        Warning(tr("请选择一个翻译来源!"));
        return;
    }
    std::vector<std::string> list;
    const auto count = ui->list->count();
    for(int i=0;i<count;++i)
    {
        auto item = ui->list->item(i);
        if(item->checkState() == Qt::Unchecked)
            continue;
        list.push_back(item->data(Qt::UserRole + 1).toString().toStdString());
    }
    if(list.empty())
    {
        Warning(tr("没有选定可用的组!"));
        return;
    }

    if(!_text.open(path.c_str()))
    {
        Warning(tr("文件打开失败!"));
        return;
    }
    if(_text.empty())
    {
        Warning(tr("翻译来源是空的!"));
        return;
    }

    int num = 0;
    const auto overwrite = ui->chk_overwrite->isChecked();
    for(auto& name : list)
    {
        auto grp =_text_to->get(name.c_str());
        if(!grp) continue;
        for(auto& it : *grp)
        {
            auto& src = it.first;
            if(!overwrite && !it.second.empty())    // 不覆盖就跳过有译文的项
                continue;
            auto trs = _text.u8(src.c_str());
            if(!trs) continue;  // 来源没有对应的译文
            grp->set(src.c_str(),trs);
            ++num;
        }
    }
    Information(tr("已转移了 %1 条译文.").arg(num));
    QDialog::accept();
}

void DlgTransform::onFile()
{
    auto filename = QFileDialog::getOpenFileName(this,tr("选择翻译来源"),"","CKText (*.ckt)");
    if(filename.isEmpty()) return;
    ui->edt_path->setText(filename);
}
