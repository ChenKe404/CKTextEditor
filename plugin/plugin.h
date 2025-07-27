/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	plugin.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef CKTEXTEDITOR_PLUGIN_H
#define CKTEXTEDITOR_PLUGIN_H

#include <QObject>
#include <QString>

namespace ck
{
class Text;
}

class QAction;
class Plugin : public QObject
{
    Q_OBJECT
public:
    enum Type {
        IMPORT
    };
public:
    // 挂载插件到菜单项
    // @act 菜单项
    // @text 文本管理对象指针
    virtual bool mount(QAction* act, ck::Text* text) = 0;
    // 插件类型
    virtual Type type() const = 0;
    // 插件名称
    virtual const QString& name() const = 0;
    // 插件声明
    virtual const QString& desc() const = 0;
signals:
    // 请求重新加载text
    void reloadRequest();
};

// 返回插件的静态实例
extern "C" Q_DECL_EXPORT Plugin* instance();
using PluginInstanceFunc = Plugin*(*)();

#endif  //! CKTEXTEDITOR_PLUGIN_H
