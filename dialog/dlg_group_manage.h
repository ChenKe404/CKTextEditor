/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_group_manage.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_GROUP_MANAGE_H
#define DLG_GROUP_MANAGE_H

#include <QDialog>
#include <QMenu>

namespace Ui {
class DlgGroupManage;
}

class PropertyWidgetItem;
class QListWidgetItem;
class DlgGroupManage : public QDialog
{
    Q_OBJECT
    enum State {
        ST_MergetTo = 1,
        ST_KickWith = 2
    };
    using States = QFlags<State>;
public:
    explicit DlgGroupManage(ck::Text*, QWidget *parent = nullptr);
    ~DlgGroupManage();

    void reload();
private:
    void setEditEnable(bool);
    void clearProperty();
    void showProperty(const Property& prop);
    void mergeTo(Group*,const QString& name);
    void onGroupChanged(QListWidgetItem *,QListWidgetItem *);
    void onMenu(const QPoint&);
    void onAdd();
    void onDelete();
    void onMerge();
    void onMergeTo();
    void onKick();
    void onSave();
    void onPropAdd();
    void onPropDelete();
private:
    Ui::DlgGroupManage *ui;
    ck::Text* _text;
    const Group* _grp_def;
    Property _prop;
    PropertyWidgetItem* _prop_item;
    QMenu _menu_item;
    QMenu _menu_prop;
    States _states;
    std::vector<QListWidgetItem*> _items;
};

#endif // DLG_GROUP_MANAGE_H
