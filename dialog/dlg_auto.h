/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_auto.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_AUTO_H
#define DLG_AUTO_H

#include <QDialog>
#include <QMenu>
#include <QtNetwork/QNetworkAccessManager>
#include <QThread>
#include <set>
#include "dlg_auto_help.h"

namespace Ui {
class DlgAuto;
}

struct AutoSetting {
    bool get;   // 是否 get 请求模式
    uint32_t interval;  // 请求间隔
    QString appid,key;
    QString src,trs;
    QString url;
    std::list<KV<QString,QString>> params; // 请求参数
    bool directly;  // 请求结果是否直接返回的
    bool equal;     // 失败响应的值判断方式
    std::list<QString> rep_state_keys;  // 失败响应key
    QString rep_state_value;            // 失败响应值
    std::list<QString> rep_result_keys; // 结果响应key
    std::list<QString> rep_msg_keys;    // 响应消息key
};

// 请求参数解析器
class AutoParamsParser
{
public:
    AutoParamsParser(Ui::DlgAuto *ui,const AutoSetting*);
    void setText(const QString&);   // 待翻译文本
    const std::map<QString,QString>& parse();   // 解析并返回请求参数
private:
    void parseCrypt(const QString& key, const QString& str, QTextStream& ts);
    void dereference(const QString& key, QString& value);
public:
private:
    Ui::DlgAuto *ui;
    const AutoSetting* _set;
    QTime _time;
    QString _text,_uuid;

    std::list<QString> _keypath;   // 键路径, 防止解引用时出现循环引用而无限递归
    std::map<QString,QString> _params;
};

class AutoWork : public QThread
{
    Q_OBJECT
public:
    void setInterval(int interval);
    void push(const QString& text,Group* grp);
    void clear();
    size_t size() const;
signals:
    void request(QString text,Group* grp);
protected:
    void run();
private:
    int _interval = 100;
    std::mutex _mtx;
    std::list<std::pair<QString,Group*>> _list;
};

class DlgAuto : public QDialog
{
    Q_OBJECT
public:

public:
    explicit DlgAuto(ck::Text*);
    ~DlgAuto();

private:
    void initTemplates();
    void readSetting();
    void saveSetting();
    void setSetting();
    void getSetting();
    void setControlsEnable(bool yes);
    void onParamAdd();
    void onParamDelete();
    void onRun();
signals:
    void progress(int value);
    void log(const QString&);
private:
    void request(const QString &text,Group* grp);
private:
    Ui::DlgAuto *ui;
    ck::Text* _text;
    QMenu _menu_params;
    DlgAutoHelp _dlg_help;
    class PropertyWidgetItem* _item_prop;

    AutoSetting _set;
    std::vector<KV<QString,AutoSetting>> _templates;
    AutoParamsParser _parser;
    QNetworkAccessManager _net;

    AutoWork _work;
    bool _stopping;
    std::mutex _mtx;
    std::set<QNetworkReply*> _replies;
};

#endif // DLG_AUTO_H
