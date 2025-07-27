/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_group_manage.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_attr.h"
#include "dlg_decide.h"
#include "dlg_decide_kick.h"
#include "dlg_group_manage.h"
#include "ui_dlg_group_manage.h"

enum Role
{
    NameRole = Qt::UserRole,
    PtrRole
};

DlgGroupManage::DlgGroupManage(ck::Text* text,QWidget *parent)
    : QDialog(parent),
    ui(new Ui::DlgGroupManage),
    _text(text),
    _prop_item(nullptr),
    _states(0),
    _grp_def(text->get())
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(0,2);
    ui->splitter->setStretchFactor(1,3);
    ui->btn_cancel->setVisible(false);

    connect(ui->btn_save,&QPushButton::clicked,this,&DlgGroupManage::onSave);
    connect(ui->btn_cancel,&QPushButton::clicked,this,[this](){
        setEditEnable(false);
        ui->list->setEnabled(true);
        ui->btn_cancel->setVisible(false);
    });

    auto act = new QAction(tr("添加"));
    connect(act,&QAction::triggered,this,&DlgGroupManage::onAdd);
    _menu_item.addAction(act);

    act = new QAction(tr("删除"));
    connect(act,&QAction::triggered,this,&DlgGroupManage::onDelete);
    _menu_item.addAction(act);

    act = new QAction(tr("合并"));
    connect(act,&QAction::triggered,this,&DlgGroupManage::onMerge);
    _menu_item.addAction(act);

    act = new QAction(tr("合并到..."));
    connect(act,&QAction::triggered,this,&DlgGroupManage::onMergeTo);
    _menu_item.addAction(act);

    act = new QAction(tr("剔除重复项与..."));
    connect(act,&QAction::triggered,this,&DlgGroupManage::onKick);
    _menu_item.addAction(act);

    connect(ui->list,&QListWidget::currentItemChanged,this,&DlgGroupManage::onGroupChanged);
    connect(ui->list,&QWidget::customContextMenuRequested,this,&DlgGroupManage::onMenu);

    act = new QAction(tr("添加"));
    connect(act,&QAction::triggered,this,&DlgGroupManage::onPropAdd);
    _menu_prop.addAction(act);

    act = new QAction(tr("删除"));
    connect(act,&QAction::triggered,this,&DlgGroupManage::onPropDelete);
    _menu_prop.addAction(act);
    connect(ui->edt_name,&QLineEdit::textChanged,this,[this](){ ui->btn_save->setEnabled(true); });
    connect(ui->tree,&PropertyWidget::contextMenuRequest,this,[this](const QPoint& pos,PropertyWidgetItem* item){
        _prop_item = item;
        _menu_prop.actions()[1]->setVisible(item != nullptr);   // 没有选择项时不显示"删除"
        _menu_prop.popup(pos);
    });
    connect(ui->tree,&PropertyWidget::valueChanged,this,[this](PropertyWidgetItem* item,const QVariant& value){
        auto name = item->name().toStdString();
        _prop.set(name.c_str(),cast(value));
        ui->btn_save->setEnabled(true);
    });


    ui->tree->add("组属性");
    ui->tree->setGroupTitleHidden(0);
    reload();
    setEditEnable(false);
}

DlgGroupManage::~DlgGroupManage()
{
    delete ui;
}

void DlgGroupManage::reload()
{
    ui->list->clear();
    for(auto& it : *_text)
    {
        const auto& name = it.first;
        auto item = new QListWidgetItem(name.empty() ? tr("[默认组]") : name.c_str());
        item->setData(NameRole,name.c_str());
        item->setData(PtrRole,(uintptr_t)&it.second);
        ui->list->addItem(item);
    }

    ui->edt_name->clear();
    ui->edt_name->setFocus();
    ui->edt_count->clear();
    clearProperty();
}

void DlgGroupManage::setEditEnable(bool yes)
{
    ui->edt_name->setEnabled(yes);
    ui->tree->setEnabled(yes);
    ui->btn_save->setEnabled(false);
}

void DlgGroupManage::clearProperty()
{
    _prop.clear();
    ui->tree->item(0)->clear();
}

void DlgGroupManage::showProperty(const Property &prop)
{
    auto root = ui->tree->item(0);
    root->clear();
    for(auto& it : prop)
    {
        root->add(it.first.c_str(),cast(it.second));
    }
}

void DlgGroupManage::mergeTo(Group *target,const QString& target_name)
{
    Group* grp_def = nullptr;
    for(auto i=_items.cbegin(); i!=_items.cend();)
    {
        auto& it = *i;
        auto grp = (Group*)it->data(PtrRole).toLongLong();
        int count = std::distance(grp->begin(),grp->end());
        if(grp == target)   // 如果选择了目标组, 则将其从items剔除
            i = _items.erase(i);
        else if(grp == _grp_def) // 如果选择了默认组, 则后续将默认组所有内容清空
        {
            grp_def = grp;
            ++i;
        }
        else if(count < 1)  // 删除空组
        {
            ui->list->takeItem(ui->list->indexFromItem(it).row());
            auto name = it->data(NameRole).toString().toStdString();
            _text->remove(name.c_str());
            i = _items.erase(i);
        }
        else
            ++i;
    }
    if(_items.empty())
        return;

    bool use_merged = true;
    bool following = false;
    for(auto& item : _items)
    {
        auto name = item->data(NameRole).toString().toStdString();
        auto grp = (Group*)item->data(PtrRole).toLongLong();
        for(auto& it : *grp)
        {
            auto exists = target->u8(it.first.c_str());
            if(exists && following)
            {
                if(use_merged)
                    continue;
                else
                {
                    target->set(it.first.c_str(),it.second.c_str());
                    continue;
                }
            }
            if(exists)  // 存在相同原文项
            {
                DlgDecide dlg;
                dlg.setName(target_name,name.c_str());
                dlg.set(it.first,exists,it.second);
                dlg.exec();
                following = dlg.following();
                use_merged = dlg.useMerged();
                if(use_merged)
                    continue;
            }
            target->set(it.first.c_str(),it.second.c_str());
        }
        if(grp != _grp_def)
        {
            _text->remove(name.c_str());
            ui->list->takeItem(ui->list->indexFromItem(item).row());
        }
    }
    if(grp_def)
        grp_def->clear();

    _items.clear();
}

void DlgGroupManage::onGroupChanged(QListWidgetItem* item, QListWidgetItem* last)
{
    if(!item) return;
    if(_states.testFlag(ST_MergetTo))
    {
        if(Question(tr("您确定要将已选的组合并到\"%1\"吗?").arg(item->text())))
        {
            auto grp = (Group*)item->data(PtrRole).toLongLong();
            mergeTo(grp,item->text());
        }

        const auto count = ui->list->count();
        for(int i=0;i<count;++i)
        {
            auto it = ui->list->item(i);
            it->setFlags(it->flags() | Qt::ItemIsEnabled);
        }
        _states.setFlag(ST_MergetTo,false);
    }
    if(_states.testFlag(ST_KickWith))
    {
        if(Question(tr("您确定要将已选组中与\"%1\"重复的项删除吗?").arg(item->text())))
        {
            bool following = false; // 之后的也这样
            bool use_retain = true;
            int count = 0;
            const char* no_src = "";
            auto grp = (Group*)item->data(PtrRole).toLongLong();
            for(auto& it : _items)
            {
                auto ptr = (Group*)it->data(PtrRole).toLongLong();
                if(it == item || ptr->empty())
                    continue;
                for(auto i=ptr->begin();i!=ptr->end();)
                {
                    const auto& src = i->first.c_str();
                    const auto& trs_kick = i->second;
                    auto trs_retain = grp->u8(src,no_src);
                    if(!trs_retain) // 保留组内没有目标原文
                    {
                        ++i;
                        continue;
                    }
                    if(trs_retain == no_src)    // 保留译文为空
                    {
                        if(!trs_kick.empty())
                            grp->set(src,trs_kick.c_str());    // 直接保留将被剔除项的译文
                    }
                    else if(!trs_kick.empty())   // 保留译文和将被剔除译文都存在
                    {
                        if(!following && trs_kick.compare(trs_retain) != 0)   // 两个译文不一致则需要确认
                        {
                            DlgDecideKick dlg;
                            dlg.setName(item->text(),it->text());
                            dlg.set(src,trs_retain,trs_kick);
                            dlg.exec();
                            following = dlg.following();
                            use_retain = dlg.useRetain();
                        }
                        if(!use_retain) // 使用将被剔除的译文
                            grp->set(src,trs_kick.c_str());
                    }

                    i = ptr->remove(i);
                    ++count;
                }
            }
            Information(tr("已剔除 %1 项.").arg(count));
        }

        const auto count = ui->list->count();
        for(int i=0;i<count;++i)
        {
            auto it = ui->list->item(i);
            it->setFlags(it->flags() | Qt::ItemIsEnabled);
        }
        _states.setFlag(ST_KickWith,false);
    }
    else
    {
        if(ui->btn_save->isEnabled() && !Question(tr("您所做的更改还未保存,要放弃这些更改吗?")))
        {
            ui->list->blockSignals(true);
            ui->list->setCurrentItem(last);
            ui->list->blockSignals(false);
            return;
        }

        setEditEnable(true);
        ui->btn_cancel->setVisible(false);
        auto name = item->data(NameRole).toString();
        ui->edt_name->blockSignals(true);
        if(!name.isEmpty())
            ui->edt_name->setText(name);
        else
        {
            ui->edt_name->setText(tr("[默认组]"));
            ui->edt_name->setEnabled(false);
        }
        ui->edt_name->blockSignals(false);
        auto grp = (ck::Text::Group*)item->data(PtrRole).toLongLong();
        ui->edt_count->setText(QString::number(std::distance(grp->begin(),grp->end())));
        _prop = grp->prop();
        showProperty(_prop);
    }
}

void DlgGroupManage::onMenu(const QPoint & pos)
{
    auto acts = _menu_item.actions();
    auto item = ui->list->itemAt(pos);
    if(item)
    {
        auto grp = (Group*)item->data(PtrRole).toLongLong();
        acts[1]->setVisible(grp != _grp_def);   // 默认组不显示"删除"
        for(int i=2;i<acts.count();++i)
        {
            acts[i]->setVisible(true);
        }
    }
    else    // 隐藏除"添加"之外的选项
    {
        for(int i=1;i<acts.count();++i)
        {
            acts[i]->setVisible(false);
        }
    }

    if(!_states.testFlag(ST_MergetTo) && !_states.testFlag(ST_KickWith) )
        _menu_item.popup(ui->list->mapToGlobal(pos));
    else
    {
        const auto count = ui->list->count();
        for(int i=0;i<count;++i)
        {
            auto it = ui->list->item(i);
            it->setFlags(it->flags() | Qt::ItemIsEnabled);
        }
        _states.setFlag(ST_MergetTo,false);
        _states.setFlag(ST_KickWith,false);
    }
}

void DlgGroupManage::onAdd()
{
    ui->list->setEnabled(false);
    setEditEnable(true);
    ui->btn_save->setEnabled(true);
    ui->btn_cancel->setVisible(true);
    ui->edt_name->clear();
    ui->edt_name->setFocus();
    ui->edt_count->clear();
    clearProperty();
}

void DlgGroupManage::onDelete()
{
    auto item = ui->list->currentItem();
    if(!item || !Question(tr("确定要删除所选的组吗?")))
        return;

    auto text = item->text().toStdString();
    if(text.empty())
        return;
    _text->remove(text.c_str());
    reload();

    setEditEnable(false);
    ui->btn_cancel->setVisible(false);
}

void DlgGroupManage::onMerge()
{
    auto items = ui->list->selectedItems();
    if(items.size() < 2)
        return;
    _items.assign(items.begin(),items.end());
    QString target_name;
    Group* target = nullptr;
    // 合并如果选择了默认组, 则将所有内容合并到默认组
    for(auto i=_items.begin();i!=_items.end();)
    {
        auto& it = *i;
        auto grp = (Group*)it->data(PtrRole).toLongLong();
        if(grp != _grp_def)
            ++i;
        else
        {
            target_name = it->text();
            target = grp;
            i = _items.erase(i);
            break;
        }
    }
    // 创建合并组
    if(!target)
    {
        int num = -1;
        QString str("merged_%1");
        while(true)
        {
            auto name = num < 0 ? "merged" : str.arg(num).toStdString();
            if(!_text->get(name.c_str()))
            {
                target = _text->insert(name.c_str());
                if(!target)
                {
                    Warning(tr("无法创建合并组!"));
                    return;
                }
                auto item = new QListWidgetItem(name.c_str());
                item->setData(NameRole,name.c_str());
                item->setData(PtrRole,(uintptr_t)target);
                target_name = name.c_str();
                ui->list->addItem(item);
                break;
            }
            ++num;
        }
    }

    mergeTo(target,target_name);
}

void DlgGroupManage::onMergeTo()
{
    auto items = ui->list->selectedItems();
    if(items.empty())
        return;
    _items.assign(items.begin(),items.end());
    for(auto i=_items.begin(); i!=_items.end(); ++i)
    {
        auto& it = *i;
        it->setFlags(it->flags() &~ Qt::ItemIsEnabled);
    }
    _states.setFlag(ST_MergetTo);
    setEditEnable(false);
}

void DlgGroupManage::onKick()
{
    auto items = ui->list->selectedItems();
    if(items.empty())
        return;
    _items.assign(items.begin(),items.end());
    for(auto i=_items.begin(); i!=_items.end(); ++i)
    {
        auto& it = *i;
        it->setFlags(it->flags() &~ Qt::ItemIsEnabled);
    }
    _states.setFlag(ST_KickWith);
    setEditEnable(false);
}

void DlgGroupManage::onSave()
{
    const auto name = ui->edt_name->text().toStdString();
    if(name.size() > 64)
    {
        Warning(tr("组名长度不能超过64字节!"));
        return;
    }

    // 如果取消按钮可见, 那么现在就是"添加"组
    if(ui->btn_cancel->isVisible())
    {
        if(name.empty())
        {
            Warning(tr("组名不能为空!"));
            return;
        }

        auto grp = _text->get(name.c_str());
        if(grp)
        {
            if(Question(tr("已存在同名组, 要覆盖其属性吗?")))
                grp->prop() = _prop;
        }
        else
        {
            grp = _text->insert(name.c_str(),_prop);
            if(!grp)
                Warning(tr("添加组失败!"));
            else
            {
                auto item = new QListWidgetItem(name.c_str());
                item->setData(NameRole,name.c_str());
                item->setData(PtrRole,(uintptr_t)grp);
                ui->list->addItem(item);
                ui->list->setCurrentItem(item);
                onGroupChanged(item,nullptr);
            }
        }
        ui->list->setEnabled(true);
    }
    else    // 编辑组
    {
        // 如果名称框未启用, 那么现在就是默认组
        if(!ui->edt_name->isEnabled())
        {
            auto grp = _text->get();
            if(grp) grp->prop() = _prop;
        }
        else
        {
            if(name.empty())
            {
                Warning(tr("组名不能为空!"));
                return;
            }

            auto item = ui->list->currentItem();
            if(!item)
            {
                if(!Question(tr("所选组不存在, 要添加吗?")))
                    return;
                auto grp = _text->insert(name.c_str(),_prop);
                if(!grp)
                    Warning(tr("添加组失败!"));
                else
                {
                    auto item = new QListWidgetItem(name.c_str());
                    item->setData(NameRole,name.c_str());
                    item->setData(PtrRole,(uintptr_t)grp);
                    ui->list->addItem(item);
                    ui->list->setCurrentItem(item);
                    ui->btn_save->setEnabled(false);
                    onGroupChanged(item,nullptr);
                }
            }
            else
            {
                auto oldname = item->data(NameRole).toString().toStdString();
                auto grp = (Group*)item->data(PtrRole).toLongLong();
                grp->prop() = _prop;
                if(oldname.compare(name) == 0)
                    ui->btn_save->setEnabled(false);
                else
                {
                    if(!_text->rename(oldname.c_str(),name.c_str()))
                        Warning(tr("重命名组失败!"));
                    else
                    {
                        item->setText(name.c_str());
                        item->setData(NameRole,name.c_str());
                        ui->btn_save->setEnabled(false);
                        onGroupChanged(item,nullptr);
                    }
                } 
            }
        }
    }
}

void DlgGroupManage::onPropAdd()
{
    DlgAttr dlg;
    if(dlg.exec() != QDialog::Accepted)
        return;
    auto name = dlg.name().toUtf8().toStdString();
    auto value = dlg.value();
    _prop.set(name.c_str(),cast(value));
    showProperty(_prop);
    ui->btn_save->setEnabled(true);
}

void DlgGroupManage::onPropDelete()
{
    if(!_prop_item) return;
    auto root = ui->tree->item(0);
    auto name = _prop_item->name().toStdString();
    root->remove(root->index(_prop_item));
    _prop.remove(name.c_str());
    ui->btn_save->setEnabled(true);
}

