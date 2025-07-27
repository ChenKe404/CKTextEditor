/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_attr.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_attr.h"
#include "ui_dlg_attr.h"

#include <QItemEditorFactory>
#include <QSpinBox>

DlgAttr::DlgAttr(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgAttr)
{
    ui->setupUi(this);
    ui->cob_type->setCurrentIndex(0);
    onType(0);
    connect(ui->cob_type,&QComboBox::currentIndexChanged,this,&DlgAttr::onType);
}

DlgAttr::~DlgAttr()
{
    delete ui;
}

void DlgAttr::accept()
{
    auto name = ui->edt_name->text();
    auto sstr = name.toStdString();
    if(name.isEmpty())
    {
        Warning(tr("名称不能为空!"));
        return;
    }
    if(sstr.size() > 64)
    {
        Warning(tr("名称字节长度不能大于64!"));
        return;
    }

    _value = {};
    auto item = ui->formLayout->itemAt(1,QFormLayout::FieldRole);
    if(item)
    {
        auto w = item->widget();
        switch (ui->cob_type->currentIndex()) {
        case 0: // bool
            _value = ((QComboBox*)w)->currentIndex() == 0;
            break;
        case 1: // int
            _value = ((QSpinBox*)w)->value();
            break;
        case 2: // float
            _value = ((QDoubleSpinBox*)w)->value();
            break;
        case 3: // string
        {
            auto text = ((QLineEdit*)w)->text();
            auto sstr = text.toStdString();
            if(sstr.empty())
            {
                Warning(tr("需要字符串内容!"));
                return;
            }
            if(sstr.size() > 255)
            {
                Warning(tr("字符串字节长度不能超过255!"));
                return;
            }
            _value = text;
        }
        break;
        }
    }

    QDialog::accept();
}

QString DlgAttr::name() const
{
    return ui->edt_name->text();
}

const QVariant &DlgAttr::value() const
{
    return _value;
}

void DlgAttr::onType(int idx)
{
    auto createEditor = [this](QWidget* e){
        auto lay = ui->formLayout;
        auto old = lay->itemAt(1,QFormLayout::FieldRole);
        if(old) lay->replaceWidget(old->widget(),e);
        else lay->setWidget(1,QFormLayout::FieldRole,e);
    };

    switch (idx) {
    case 0: // bool
    {
        auto e = new QComboBox;
        e->addItem("True");
        e->addItem("False");
        createEditor(e);
    }
    break;
    case 1: // int
    {
        auto e = new QSpinBox;
        e->setMinimum(-99999999);
        e->setMaximum(99999999);
        createEditor(e);
    }
        break;
    case 2: // float
    {
        auto e = new QDoubleSpinBox;
        e->setMinimum(-99999999);
        e->setMaximum(99999999);
        createEditor(e);
    }
        break;
    case 3: // string
    {
        auto e = new QLineEdit;
        e->setPlaceholderText(tr("字节长度小于255"));
        createEditor(e);
    }
    }
}

