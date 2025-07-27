/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	dlg_auto.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_auto.h"
#include "ui_dlg_auto.h"
#include <QDir>
#include <QUuid>
#include <QSettings>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

void readSetting(const QString& path, AutoSetting& set)
{
    QSettings stt(path, QSettings::IniFormat);
    stt.beginGroup("Auto");

    set.get = stt.value("Mode").toString().compare("GET") == 0;
    set.interval = stt.value("Interval").toUInt();
    set.appid = stt.value("AppID").toString();
    set.key = stt.value("Key").toString();
    set.src = stt.value("Src").toString();
    set.trs = stt.value("Trs").toString();
    set.url = stt.value("Url").toString();
    {
        auto &params = set.params;
        params.clear();
        auto sp = stt.value("Params").toString().split('&');
        for (auto &it : sp) {
            auto kv = it.split("=");
            if (kv.size() < 2)
                continue;
            params.push_back(make_kv(kv[0], kv[1]));
        }
    }

    set.directly = stt.value("Directly").toBool();
    set.equal = stt.value("Equal").toBool();
    {
        auto& rsk = set.rep_state_keys;
        rsk.clear();
        auto sp = stt.value("RepStateKeys").toString().split('>');
        for(auto& it : sp)
        {
            rsk.push_back(it);
        }
    }
    set.rep_state_value = stt.value("RepStateValue").toString();
    {
        auto& rrk = set.rep_result_keys;
        rrk.clear();
        auto sp = stt.value("RepResultKeys").toString().split('>');
        for(auto& it : sp)
        {
            rrk.push_back(it);
        }
    }
    {
        auto& rmk = set.rep_msg_keys;
        rmk.clear();
        auto sp = stt.value("RepMsgKeys").toString().split('>');
        for(auto& it : sp)
        {
            rmk.push_back(it);
        }
    }

    stt.endGroup();
}

inline QString md5(const QString& text)
{ return QCryptographicHash::hash(text.toLatin1(),QCryptographicHash::Md5).toHex(); }

inline QString sha256(const QString& text)
{ return QCryptographicHash::hash(text.toLatin1(),QCryptographicHash::Sha256).toHex(); }

inline bool valid(const QJsonValue& v)
{ return !v.isNull() && !v.isUndefined(); }

QJsonValue getValue(const QJsonDocument& doc,const std::list<QString>& keys)
{
    QJsonValue v(QJsonValue::Undefined);
    for(auto& key : keys)
    {
        if(!valid(v))
        {
            if(!doc.isArray())
                v = doc[key];
            else
            {
                bool ok = false;
                auto n = key.toInt(&ok);
                v = ok ? doc[n] : doc[key];
            }
        }
        else
        {
            if(!v.isArray())
                v = v[key];
            else
            {
                bool ok = false;
                auto n = key.toInt(&ok);
                v = ok ? v[n] : doc[key];
            }
        }
        if(!valid(v))
            break;
    }
    return v;
}

///////////////////////////////////////////////////
/// DlgAuto
///
DlgAuto::DlgAuto(ck::Text* text)
    : QDialog(nullptr),
    ui(new Ui::DlgAuto),
    _text(text),
    _item_prop(nullptr),
    _dlg_help(this),
    _parser(ui,&_set),
    _stopping(false)
{
    ui->setupUi(this);
    ui->pgs->setVisible(false);
    ui->tree->add("root");
    ui->tree->setGroupTitleHidden(0);
    ui->tree->setNameEditable(true);
    ui->btn_stop->setEnabled(false);
    ui->edt_log->setVisible(false);

    auto act = new QAction(tr("添加"));
    connect(act,&QAction::triggered,this,&DlgAuto::onParamAdd);
    _menu_params.addAction(act);

    act = new QAction(tr("删除"));
    connect(act,&QAction::triggered,this,&DlgAuto::onParamDelete);
    _menu_params.addAction(act);

    connect(ui->tree,&PropertyWidget::contextMenuRequest,this,[this](const QPoint& pos,PropertyWidgetItem* item){
        _item_prop = item;
        auto acts = _menu_params.actions();
        acts[1]->setVisible(item != nullptr);
        _menu_params.popup(pos);
    });
    connect(ui->chk_directly,&QCheckBox::clicked,this,[this](bool checked){
        ui->edt_state_key->setEnabled(!checked);
        ui->cob_equal->setEnabled(!checked);
        ui->edt_state_value->setEnabled(!checked);
        ui->edt_result_key->setEnabled(!checked);
    });
    connect(ui->btn_help,&QPushButton::clicked,&_dlg_help,&DlgAutoHelp::show);
    connect(ui->cob_template,&QComboBox::currentIndexChanged,this,[this](int idx){
        auto i = idx - 1;
        if(i < 0 || i > _templates.size())
        {
            readSetting();
            setSetting();
        }
        else
        {
            _set = _templates[i].v;
            setSetting();
        }
    });
    connect(ui->btn_test,&QPushButton::clicked,this,[this](){
        getSetting();
        request(ui->edt_test->text(),0);
    });
    connect(ui->btn_run,&QPushButton::clicked,this,&DlgAuto::onRun);
    connect(ui->btn_stop,&QPushButton::clicked,this,[this](){
        _work.terminate();
        _stopping = true;
        _mtx.lock();
        for(auto& it : _replies)
        {
            it->abort();
        }
        _replies.clear();
        _mtx.unlock();
        setControlsEnable(true);
        _stopping = false;
    });
    initTemplates();
    readSetting();
    setSetting();

    connect(ui->btn_save,&QPushButton::clicked,this,[this](){
        getSetting();
        saveSetting();
    });
    connect(ui->btn_show_log,&QPushButton::clicked,this,[this](){
        ui->edt_log->setVisible(!ui->edt_log->isVisible());
    });
    connect(this,&DlgAuto::log,this,[this](const QString& text){
        ui->edt_log->setVisible(true);
        ui->edt_log->appendPlainText(text);
    });
    connect(&_work,&AutoWork::request,this,&DlgAuto::request,Qt::QueuedConnection);
}

DlgAuto::~DlgAuto()
{
    _work.terminate();
    _work.wait();
    while(!_replies.empty()) {}
    delete ui;
}

void DlgAuto::initTemplates()
{
    QDir dir = QDir::currentPath()+"/auto_api_templates";
    if(!dir.exists()) return;
    const auto path = dir.absolutePath()+"/";

    AutoSetting tmp;
    for(auto& it : dir.entryList(QDir::Files)){
        auto ext = it.right(3);
        if(ext.compare("ini",Qt::CaseInsensitive) != 0)
            continue;
        auto name = it.left(it.size()-4);
        ::readSetting(path+it,tmp);
        _templates.push_back(make_kv(name,tmp));
    }
    for(auto& it : _templates)
    {
        ui->cob_template->addItem(it.k);
    }
}

void DlgAuto::readSetting()
{
    ::readSetting(profile_path()+"setting.ini",_set);
}

void DlgAuto::saveSetting()
{
    QSettings stt(profile_path()+"setting.ini", QSettings::IniFormat);
    stt.beginGroup("Auto");

    stt.setValue("Mode", _set.get ? "GET" : "POST");
    stt.setValue("Interval",_set.interval);
    stt.setValue("AppID",_set.appid);
    stt.setValue("Key",_set.key);
    stt.setValue("Src",_set.src);
    stt.setValue("Trs",_set.trs);
    stt.setValue("Url",_set.url);
    {
        QString str;
        for(auto& it : _set.params)
        {
            str.append(it.k+"="+it.v).append('&');
        }
        if(!str.isEmpty())
            str.resize(str.size()-1);
        stt.setValue("Params",str);
    }

    stt.setValue("Directly",_set.directly);
    stt.setValue("Equal",_set.equal);
    {
        QString str;
        for(auto& it : _set.rep_state_keys)
        {
            str.append(it).append('>');
        }
        if(!str.isEmpty())
            str.resize(str.size()-1);
        stt.setValue("RepStateKeys",str);
    }
    stt.setValue("RepStateValue",_set.rep_state_value);
    {
        QString str;
        for(auto& it : _set.rep_result_keys)
        {
            str.append(it).append('>');
        }
        if(!str.isEmpty())
            str.resize(str.size()-1);
        stt.setValue("RepResultKeys",str);
    }
    {
        QString str;
        for(auto& it : _set.rep_msg_keys)
        {
            str.append(it).append('>');
        }
        if(!str.isEmpty())
            str.resize(str.size()-1);
        stt.setValue("RepMsgKeys",str);
    }

    stt.endGroup();
}

void DlgAuto::setSetting()
{
    ui->cob_reqmode->setCurrentIndex(_set.get ? 0 : 1);
    ui->spb_interval->setValue(_set.interval);
    ui->edt_appid->setText(_set.appid);
    ui->edt_key->setText(_set.key);
    ui->edt_src->setText(_set.src);
    ui->edt_trs->setText(_set.trs);
    ui->edt_url->setText(_set.url);
    {
        auto& params = _set.params;
        auto root = ui->tree->item(0);
        root->clear();
        for(auto& it : params)
        {
            root->add(it.k,it.v);
        }
    }
    ui->chk_directly->setChecked(_set.directly);
    ui->cob_equal->setCurrentIndex(_set.equal ? 0 : 1);
    {
        QString str;
        auto& rsk = _set.rep_state_keys;
        for(auto& it : rsk)
        {
            str.append(it).append('>');
        }
        if(!str.isEmpty())
            str.resize(str.size()-1);
        ui->edt_state_key->setText(str);
    }
    ui->edt_state_value->setText(_set.rep_state_value);
    {
        QString str;
        auto& rrk = _set.rep_result_keys;
        for(auto& it : rrk)
        {
            str.append(it).append('>');
        }
        if(!str.isEmpty())
            str.resize(str.size()-1);
        ui->edt_result_key->setText(str);
    }
    {
        QString str;
        auto& rmk = _set.rep_msg_keys;
        for(auto& it : rmk)
        {
            str.append(it).append('>');
        }
        if(!str.isEmpty())
            str.resize(str.size()-1);
        ui->edt_msg_key->setText(str);
    }
}

void DlgAuto::getSetting()
{
    _set.get = ui->cob_reqmode->currentIndex() == 0;
    _set.interval = ui->spb_interval->value();
    _set.appid = ui->edt_appid->text();
    _set.key = ui->edt_key->text();
    _set.src = ui->edt_src->text();
    _set.trs = ui->edt_trs->text();
    _set.url = ui->edt_url->text();
    {
        auto& params = _set.params;
        params.clear();
        auto root = ui->tree->item(0);
        const auto count = root->count();
        for(int i=0;i<count;++i)
        {
            auto item = root->child(i);
            params.push_back(make_kv(item->name(),item->value().toString()));
        }
    }
    _set.directly = ui->chk_directly->isChecked();
    _set.equal = ui->cob_equal->currentIndex() == 0;
    {
        auto& rsk = _set.rep_state_keys;
        rsk.clear();
        auto sp = ui->edt_state_key->text().split('>');
        for(auto& it : sp)
        {
            rsk.push_back(it);
        }
    }
    _set.rep_state_value = ui->edt_state_value->text();
    {
        auto& rrk = _set.rep_result_keys;
        rrk.clear();
        auto sp = ui->edt_result_key->text().split('>');
        for(auto& it : sp)
        {
            rrk.push_back(it);
        }
    }
    {
        auto& rmk = _set.rep_msg_keys;
        rmk.clear();
        auto sp = ui->edt_msg_key->text().split('>');
        for(auto& it : sp)
        {
            rmk.push_back(it);
        }
    }
}

void DlgAuto::setControlsEnable(bool yes)
{
    ui->pgs->setVisible(!yes);
    ui->cob_reqmode->setEnabled(yes);
    ui->spb_interval->setEnabled(yes);
    ui->cob_template->setEnabled(yes);
    ui->edt_appid->setEnabled(yes);
    ui->edt_key->setEnabled(yes);
    ui->edt_src->setEnabled(yes);
    ui->edt_trs->setEnabled(yes);
    ui->edt_url->setEnabled(yes);
    ui->tree->setEnabled(yes);
    ui->btn_help->setEnabled(yes);
    ui->chk_directly->setEnabled(yes);
    ui->cob_equal->setEnabled(yes);
    ui->edt_state_key->setEnabled(yes);
    ui->edt_state_value->setEnabled(yes);
    ui->edt_result_key->setEnabled(yes);
    ui->btn_test->setEnabled(yes);
    ui->chk_override->setEnabled(yes);
    ui->btn_run->setEnabled(yes);
    ui->btn_stop->setEnabled(!yes);
}

void DlgAuto::onParamAdd()
{
    auto root = ui->tree->item(0);
    QString key("key");
    {
        QString fmt("key_%1");
        int num = 0;
        while(root->index(key) >= 0)
        {
            key = fmt.arg(num);
            ++num;
        }
    }
    root->add(key,"");
}

void DlgAuto::onParamDelete()
{
    auto root = ui->tree->item(0);
    if(_item_prop)
    {
        root->remove(root->index(_item_prop));
    }
}

void DlgAuto::onRun()
{
    if(_work.isRunning())
    {
        Warning(tr("正在翻译!"));
        return;
    }

    _work.clear();
    for(auto& grp : *_text)
    {
        auto grp_ptr = const_cast<ck::Text::Group*>(&grp.second);
        for(auto& it : grp.second)
        {
            if(!ui->chk_override->isChecked() && !it.second.empty())
                continue;
            _work.push(it.first.c_str(),grp_ptr);
        }
    }
    if(_work.size() < 1)
    {
        Warning(tr("没有可以翻译的文本!"));
        return;
    }
    setControlsEnable(false);
    getSetting();

    ui->pgs->setMaximum((int)_work.size());
    ui->pgs->setValue(0);

    _work.setInterval(ui->spb_interval->value());
    _work.start();
}

void DlgAuto::request(const QString &text,Group* grp)
{
    _parser.setText(text);
    auto pms = _parser.parse();

    QNetworkRequest req;
    QNetworkReply* rep;

    QUrlQuery query;
    for(auto& it : pms)
    {
        query.addQueryItem(it.first,QUrl::toPercentEncoding(it.second));
    }
    QUrl url(_set.url);
    if(_set.get)
    {
        url.setQuery(query);
        req.setUrl(url);
        rep = _net.get(req);
    }
    else
    {
        req.setHeader(req.ContentTypeHeader,"application/x-www-form-urlencoded");
        req.setUrl(url);
        rep = _net.post(req,query.toString().toUtf8());
    }

    _mtx.lock();
    _replies.insert(rep);
    _mtx.unlock();
    connect(rep,&QNetworkReply::destroyed,this,[this](QObject* obj){
        auto value = ui->pgs->value() + 1;
        ui->pgs->setValue(value);
        _mtx.lock();
        _replies.erase((QNetworkReply*)obj);
        _mtx.unlock();
        if(value >= ui->pgs->maximum())
            setControlsEnable(true);
    });
    connect(rep, &QNetworkReply::finished, this, [this,grp,text,rep]() {
        const auto result = rep->readAll();
        // qDebug(result);
        rep->deleteLater();
        if(_stopping)
            return;
        if(_set.directly)
        {
            if(!grp)
                ui->edt_test_result->setText(result);
            else if(grp)
            {
                emit log("OK: "+text+" >> "+result);
                auto src = text.toStdString();
                auto trs = result.toStdString();
                grp->set(src.c_str(),trs.c_str());
            }
            return;
        }
        QJsonParseError error;
        auto json = QJsonDocument::fromJson(result,&error);
        if(error.error)
            return;
        auto v = getValue(json,_set.rep_state_keys);
        if(valid(v))
        {
            auto str = v.toString();
            bool falied = _set.equal ? (str == _set.rep_state_value) : (str != _set.rep_state_value);
            if(falied)
            {
                v = getValue(json,_set.rep_msg_keys);
                if(!valid(v))
                    emit log("ERROR("+str+"): "+text);
                else
                {
                    auto msg = v.toString();
                    qDebug() << "MSG: " << msg;
                    emit log("ERROR("+str+"): "+text + "\n  MSG: " + msg);
                }
                return;
            }
        }
        v = getValue(json,_set.rep_result_keys);
        if(valid(v))
        {
            qDebug() << "OK:" << v.toString();
            auto result = v.toString();
            if(!grp)
                ui->edt_test_result->setText(result);
            else if(grp)
            {
                emit log("OK: "+text+" >> "+result);
                auto src = text.toStdString();
                auto trs = result.toStdString();
                grp->set(src.c_str(),trs.c_str());
            }
        }
        else
        {

        }
    });
}

///////////////////////////////////////////////////
/// AutoParamsParser
///
AutoParamsParser::AutoParamsParser(Ui::DlgAuto *ui,const AutoSetting *set)
    : ui(ui),_set(set)
{}

void AutoParamsParser::setText(const QString & text)
{
    _text = text;
}

const std::map<QString,QString>& AutoParamsParser::parse()
{
    _uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    _time = QDateTime::currentDateTimeUtc().time();

    // qDebug() << "uuid:" << _uuid << "; time:" << _time;

    _params.clear();
    std::map<QString,QString*> exprs;
    for(auto& it : _set->params)
    {
        auto &v = _params[it.k] = it.v.trimmed();
        if(v.size() > 1 && v.front()=='`' && v.back()=='`')  // 表达式, 之后再处理
        {
            v.removeFirst();
            v.removeLast();
            exprs[it.k] = &v;
        }
        else
        {
            dereference(it.k,v);
        }
    }

    QString tmp;
    QTextStream ts(&tmp);
    for(auto& it : exprs)
    {
        ts.flush();
        parseCrypt(it.first,*it.second,ts);
        *it.second = tmp;
    }

    return _params;
}

void AutoParamsParser::parseCrypt(const QString& key, const QString& str, QTextStream& ts)
{
    if(str.isEmpty())
        return;
    QString (*do_crypt)(const QString&) = nullptr;
    auto left = str.indexOf("md5(",Qt::CaseInsensitive);
    int offset = 0;
    if(left >=0)
    {
        do_crypt = md5;
        offset += 4;
    }
    else if((left = str.indexOf("sha256(",Qt::CaseInsensitive)) >= 0)
    {
        do_crypt = sha256;
        offset += 6;
    }

    if(left >=0)
    {
        // 密文前半部分
        auto tmp = str.mid(0,left);
        dereference(key,tmp);
        ts << tmp;

        left += offset;
        auto right = str.indexOf(')',left);
        if(right < 0) right = str.size();
        tmp = str.mid(left,right-left);
        QString crypt;
        dereference(key,tmp);
        crypt.append(tmp);
        crypt = do_crypt(crypt);
        ts << crypt;

        if(right >=0)
        {
            right++;
            parseCrypt(key,str.mid(right,str.size()-right),ts); // 密文后半部分
        }
    }
    else
    {
        auto tmp = str;
        dereference(key,tmp);
        ts << tmp;
    }
}

void AutoParamsParser::dereference(const QString& key, QString& value)
{
    if(value.isEmpty())
        return;

    _keypath.push_back(key);

    value.replace("{appid}",ui->edt_appid->text());
    value.replace("{key}",ui->edt_key->text());
    value.replace("{text}",_text);
    value.replace("{src}",ui->edt_src->text());
    value.replace("{trs}",ui->edt_trs->text());
    value.replace("{salt}",_uuid);
    value.replace("{timestamp}",QString::number(_time.second()));
    for(auto& it : _params)
    {
        if(value.indexOf('[') < 0)
            break;
        auto match = '['+it.first+']';
        auto pos = value.indexOf(match);
        if(pos < 0) continue;
        // 跳过循环引用
        if(std::find(_keypath.begin(),_keypath.end(),it.first) != _keypath.end())
            continue;

        auto l = it.second.indexOf('[');
        auto r = it.second.indexOf(']');
        bool deref_need = l < r;    // 是否需要解引用
        if(!deref_need)
        {
            l = it.second.indexOf('{');
            r = it.second.indexOf('}');
            deref_need = l < r;
        }
        if(deref_need)
            dereference(it.first,it.second);
        value.replace(match,it.second);
    }

    _keypath.pop_back();
}

///////////////////////////////////////////////////
/// AutoWork
///
void AutoWork::run()
{
    while(!_list.empty())
    {
        _mtx.lock();
        auto it = std::move(_list.back());
        _list.pop_back();
        _mtx.unlock();

        if(isInterruptionRequested())
            break;

        _mtx.lock();
        emit request(it.first,it.second);
        _mtx.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(_interval));
    }
    quit();
}

void AutoWork::setInterval(int interval)
{
    _interval = interval;
}

void AutoWork::push(const QString &text, Group *grp)
{
    _mtx.lock();
    _list.push_back({ text,grp });
    _mtx.unlock();
}

void AutoWork::clear()
{
    _mtx.lock();
    _list.clear();
    _mtx.unlock();
}

size_t AutoWork::size() const
{
    return _list.size();
}
