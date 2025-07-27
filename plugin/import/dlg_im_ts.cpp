/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_im_ts.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_im_ts.h"
#include "ui_dlg_im_ts.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStringDecoder>
#include <QtCore5Compat/QTextCodec>
#include <text.h>
#include <mini/ini.h>
#include <pe.h>
#include <resource.h>

DlgImTS::DlgImTS(ck::Text* text,QWidget *parent)
    : QDialog(parent),
    ui(new Ui::DlgImTS),
    _text(text)
{
    ui->setupUi(this);
    ui->list_ini->setSelectionMode(QListWidget::ExtendedSelection);
    ui->pgs->setVisible(false);
    ui->pgs->setValue(0);

    connect(ui->btn_dll,&QPushButton::clicked,this,&DlgImTS::onGameLanguage);
    connect(ui->btn_add,&QPushButton::clicked,this,&DlgImTS::onIniAdd);
    connect(ui->btn_remove,&QPushButton::clicked,this,&DlgImTS::onIniRemove);
    connect(ui->btn_up,&QPushButton::clicked,this,&DlgImTS::onIniUp);
    connect(ui->btn_down,&QPushButton::clicked,this,&DlgImTS::onIniDown);
}

DlgImTS::~DlgImTS()
{
    delete ui;
}

void DlgImTS::accept()
{
    auto text = ui->edt_dll->text();
    if(!text.isEmpty())
        handleDLL(text);

    // 如果遇到同名项, 排列在前的ini优先级更高
    QStringList list;
    const auto count = ui->list_ini->count();
    for(int i=count-1;i>=0;--i)
    {
        list.push_back(ui->list_ini->item(i)->text());
    }
    if(!list.isEmpty())
        handleINI(list);

    QDialog::accept();
}

void DlgImTS::onGameLanguage()
{
    QFileDialog dlg(this,tr("泰伯利亚之日Language.dll文件"),{},"Language.dll;;PE (*.exe *.dll)");
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    if(dlg.exec() != QFileDialog::Accepted)
        return;
    auto list = dlg.selectedFiles();
    ui->edt_dll->setText(list.back());
}

void DlgImTS::onIniAdd()
{
    QFileDialog dlg(this,tr("泰伯利亚之日ini文件"),{},"INI (*.ini)");
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFiles);
    if(dlg.exec() != QFileDialog::Accepted)
        return;
    for(auto& it : dlg.selectedFiles())
    {
        it.replace('\\','/');
        if(!ui->list_ini->findItems(it,Qt::MatchExactly).empty())
            continue;
        ui->list_ini->addItem(it);
    }
}

void DlgImTS::onIniRemove()
{
    for(auto& it : ui->list_ini->selectedItems())
    {
        auto idx = ui->list_ini->indexFromItem(it);
        ui->list_ini->takeItem(idx.row());
    }
}

void DlgImTS::onIniUp()
{
    auto row = ui->list_ini->currentRow();
    if(row < 1) return;
    auto item = ui->list_ini->takeItem(row);
    ui->list_ini->insertItem(row - 1,item);
    ui->list_ini->setCurrentRow(row - 1);
}

void DlgImTS::onIniDown()
{
    auto row = ui->list_ini->currentRow();
    if(row > ui->list_ini->count() - 2) return;
    auto item = ui->list_ini->takeItem(row);
    ui->list_ini->insertItem(row + 1,item);
    ui->list_ini->setCurrentRow(row + 1);
}

static ck::Text::Group* make_group(ck::Text* text, const QString& qname)
{
    ck::Text::Group* grp = nullptr;
    int num = -1;
    QString str(qname+"_%1");
    while(true)
    {
        auto name = num < 0 ? qname.toStdString() : str.arg(num).toStdString();
        if(!text->get(name.c_str()))
        {
            grp = text->insert(name.c_str());
            if(!grp)
                QMessageBox::warning(nullptr,DlgImTS::tr("警告"),DlgImTS::tr("无法创建组,任务中断!"));
            break;
        }
        ++num;
    }
    return grp;
}

inline std::string win1252_to_utf8(const std::string& latin1) {
    static auto codec = QTextCodec::codecForName("Windows-1252");
    return codec->toUnicode(latin1.c_str()).toStdString();
}

void DlgImTS::handleDLL(const QString & filename)
{
    pexx::context ctx;
    auto sstr = filename.toLocal8Bit().toStdString();
    if(ctx.load(sstr.c_str()) != pexx::E_OK)
    {
        QMessageBox::warning(this,tr("警告"),tr("读取DLL失败!"));
        return;
    }

    pexx::ResourceAccessor acc(&ctx);
    pexx::VersionInfo vi;
    acc.get_version(vi);

    std::vector<pexx::String> strs;
    acc.get_string(strs);
    if(!strs.empty())
    {
        auto grp = make_group(_text,"dll_strings");
        if(grp)
        {
            for(auto& it : strs)
            {
                auto str = QString::fromStdU16String(it.s);
                grp->set(str.toStdString().c_str(),"");
            }
        }
    }

    std::vector<pexx::Dialog> dlgs;
    acc.get_dialog(dlgs);
    if(!dlgs.empty())
    {
        auto grp = make_group(_text,"dll_dialogs");
        if(grp)
        {
            for(auto& dlg : dlgs)
            {
                auto str = QString::fromStdU16String(dlg.text);
                grp->set(str.toStdString().c_str(),"");
                for(auto& ctl : dlg.ctrls)
                {
                    str = QString::fromStdU16String(ctl.text);
                    grp->set(str.toStdString().c_str(),"");
                }
            }
        }
    }
}

void DlgImTS::handleINI(const QStringList &list)
{
    using namespace mINI;

    auto pgs = ui->pgs;
    pgs->setValue(0);
    pgs->setMaximum(list.size());
    pgs->setVisible(true);

    ck::Text::Group* grp = nullptr;
    for(auto& it : list)
    {
        auto pos = it.lastIndexOf('/');
        auto qname = it.right(it.size() - pos - 1);
        if(qname.isEmpty())
        {
            pgs->setValue(pgs->value() + 1);
            continue;
        }

        grp = make_group(_text,qname);
        if(!grp) return;

        INIFile file(it.toStdString());
        INIStructure ini;
        if(!file.read(ini))
        {
            pgs->setValue(pgs->value() + 1);
            continue;
        }

        for(auto& it : ini)
        {
            auto sec = it.second;
            auto v = sec.get("Name");
            if(!v.empty())
                grp->set(win1252_to_utf8(v).c_str(),"");

            v = sec.get("Description");
            if(!v.empty())
                grp->set(win1252_to_utf8(v).c_str(),"");

            // mission 任务简报
            v = sec.get("Briefing");
            if(!v.empty())
            {
                auto m = ini.get(v);
                if(m.size() < 1)
                    grp->set(win1252_to_utf8(v).c_str(),"");
                else
                {
                    std::string str;
                    for(auto& it : m)
                    {
                        str.append(it.second);
                        if(it.second.back() != '@')
                            str.append(" ");
                    }
                    if(!str.empty() && str.back() == ' ')
                        str.pop_back();
                    grp->set(win1252_to_utf8(str).c_str(),"");
                }
            }

            // snow/temperat 地形地块名称
            v = sec.get("SetName");
            if(!v.empty())
                grp->set(win1252_to_utf8(v).c_str(),"");

            // tutorial 教程文本
            if(it.first.compare("tutorial") == 0)
            {
                for(auto& it : sec)
                {
                    grp->set(win1252_to_utf8(it.second).c_str(),"");
                }
            }
        }
        pgs->setValue(pgs->value() + 1);
    }
    pgs->setVisible(false);
}

