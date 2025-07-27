/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	im_tiberian_sun.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include <QAction>
#include <plugin.h>
#include <text.h>
#include "dlg_im_ts.h"

class Imp : public Plugin
{
    bool mount(QAction *act, ck::Text *text) override
    {
        QObject::connect(act,&QAction::triggered,this,[this,text](){
            DlgImTS dlg(text);
            if(dlg.exec() == dlg.Accepted)
                emit reloadRequest();
        });
        return true;
    }

    Type type() const override
    {
        return IMPORT;
    }

    const QString &name() const override
    {
        return _name;
    }

    const QString &desc() const override
    {
        return _desc;
    }

    QString _name = tr("泰伯利亚之日");
    QString _desc = tr("从泰伯利亚之日资源导入");
};

Plugin* instance()
{
    static Imp imp;
    return &imp;
}
