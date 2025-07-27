/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	mainwindow.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <set>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPluginLoader>
#include <dlg_about.h>
#include <QJsonDocument>
#include <QJsonArray>
#include <dlg_transform.h>
#include <QSettings>
#include <QTranslator>

#include "dlg_find.h"
#include "dlg_attr.h"
#include "dlg_group_manage.h"
#include "dlg_auto.h"
#include "plugin/plugin.h"

QString language_name(const QString& filename, QString& locName)
{
    static QRegularExpression regx("(.*)_(.+)_(.+)\\.qm");
    auto mt = regx.match(filename);
    if(!mt.hasCaptured(3) || mt.captured(1) != "cktexteditor")
        return "";
    locName = mt.captured(2)+"_"+mt.captured(3);
    QLocale loc(locName);
    return loc.nativeLanguageName();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    _dlg_find(this),
    _changed(false),
    _item_prop(nullptr)
{
    ui->setupUi(this);
    ui->splitter->setCollapsible(0,false);
    ui->splitter->setCollapsible(1,true);
    ui->splitter->setSizes({1,0});
    _dlg_find.attach(ui->table);
    _dlg_find.setParent(this,Qt::Window);
    _dlg_replace.attach(this);
    _dlg_replace.setParent(this,Qt::Window);
    loadPlugins();
    _title = windowTitle();

    connect(ui->table,&TextTableWidget::srcChanged,this,[this](){ _changed = true; });
    connect(ui->table,&TextTableWidget::trsChanged,this,[this](){ _changed = true; });
    connect(ui->act_new,&QAction::triggered,this,&MainWindow::onNew);
    connect(ui->act_open,&QAction::triggered,this,&MainWindow::onOpen);
    connect(ui->act_save,&QAction::triggered,this,&MainWindow::onSave);
    connect(ui->act_save_as,&QAction::triggered,this,&MainWindow::onSaveAs);
    connect(ui->act_popup_edit,&QAction::triggered,this,[this](bool checked){
        ui->table->setInlineEdit(!checked);
    });
    connect(ui->act_show_space,&QAction::triggered,this,[this](bool checked){
        ui->table->showSpace(checked);
    });
    connect(ui->act_show_line_feed,&QAction::triggered,this,[this](bool checked){
        ui->table->showLineFeed(checked);
    });

    connect(ui->act_charset,&QAction::triggered,this,&MainWindow::onExportCharset);
    connect(ui->btn_attr,&QPushButton::clicked,this,&MainWindow::onProp);
    connect(&_timer,&QTimer::timeout,this,&MainWindow::onTimeout);

    connect(ui->tree,&PropertyWidget::valueChanged,this,&MainWindow::onPropChanged);
    connect(ui->tree,&PropertyWidget::contextMenuRequest,this,&MainWindow::onPropMenu);

    connect(ui->btn_fit_lineheight,&QPushButton::clicked,ui->table,&QTableView::resizeRowsToContents);
    connect(ui->btn_def_lineheight,&QPushButton::clicked,this,[this](){
        auto header = ui->table->verticalHeader();
        auto size = header->defaultSectionSize();
        auto count = header->count();
        for(int i=0;i<count;++i) {
            header->resizeSection(i,size);
        }
    });
    connect(ui->cob_group,&QComboBox::currentIndexChanged,this,&MainWindow::refresh);
    connect(ui->btn_group,&QPushButton::clicked,this,[this](){
        DlgGroupManage dlg(&_text,this);
        dlg.exec();
        reload();
    });
    connect(ui->cob_sort,&QComboBox::currentIndexChanged,this,[this](int idx){
        using S = TextTableWidget::Sort;
        switch (idx) {
        case 0:     // 顺序(从小到大)
            ui->table->sort(S::ASC);
            break;
        case 1:     // 逆序(从大到小)
            ui->table->sort(S::DESC);
            break;
        case 2:     // 置顶未完成
            ui->table->sort(S::UNFINISHED_TOPMOST);
            break;
        case 3:     // 从短到长
            ui->table->sort(S::SHORT2LONG);
            break;
        case 4:     // 从长到短
            ui->table->sort(S::LONG2SHORT);
            break;
        }
    });

    connect(ui->act_json_in,&QAction::triggered,this,&MainWindow::onImportJson);
    connect(ui->act_json_out,&QAction::triggered,this,&MainWindow::onExportJson);
    connect(ui->act_auto,&QAction::triggered,this,[this](){
        DlgAuto dlg(&_text);
        dlg.exec();
        reload();
    });
    connect(ui->act_about,&QAction::triggered,this,[](){
        DlgAbout dlg;
        dlg.exec();
    });
    connect(ui->act_transform,&QAction::triggered,this,&MainWindow::onTransform);

    auto act = new QAction(tr("添加"));
    _menu_prop.addAction(act);
    connect(act,&QAction::triggered,this,&MainWindow::onPropAdd);

    act = new QAction(tr("删除"));
    _menu_prop.addAction(act);
    connect(act,&QAction::triggered,this,&MainWindow::onPropDelete);

    int count = ui->table->attach(_text.get());
    ui->table->showSpace(false);
    ui->table->showLineFeed(false);
    message(tr("共%1项").arg(count));

    ui->tree->add(tr("文档"));
    ui->tree->add(tr("组"));
    reload();
    updateTitle();

    QString locName;
    QTranslator tra;
    auto path = qApp->applicationDirPath()+"/translations/";
    QDir dir(path);
    for(auto& it : dir.entryList({"*.qm"},QDir::Files))
    {
        auto name = language_name(it,locName);
        if(name.isEmpty() || !tra.load(path+it))
            continue;
        auto act = new QAction(name);
        act->setData(locName);
        connect(act,&QAction::triggered,this,&MainWindow::onLanguage);
        ui->menu_language->addAction(act);
    }
    connect(ui->act_chinese,&QAction::triggered,this,&MainWindow::onLanguage);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *e = static_cast<QKeyEvent*>(event);
        if (e->modifiers() & Qt::ControlModifier && e->key() == Qt::Key_F) {
            _dlg_find.show();
            return true;
        }
        else if (e->modifiers() & Qt::ControlModifier && e->key() == Qt::Key_R) {
            _dlg_replace.show();
            _dlg_replace.raise();
            return true;
        }
    }
    return QMainWindow::event(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(!confirm())
    {
        event->ignore();
        return;
    }
    event->accept();
}

void MainWindow::message(const QString &msg)
{
    _timer.stop();
    ui->statusbar->showMessage(msg);
    _timer.start(5000);
}

bool MainWindow::confirm()
{
    if(_changed)
    {
        if(QMessageBox::question(this,tr("确认"),tr("当前文件未保存,要放弃所做的更改吗!")) == QMessageBox::No)
            return false;
    }
    _changed = false;
    return true;
}

void MainWindow::onTimeout()
{
    ui->statusbar->showMessage("");
}

void MainWindow::onNew()
{
    if(!confirm())
        return;

    _text.clear();
    _filename.clear();
    reload();
    updateTitle();
}

void MainWindow::onOpen()
{
    if(!confirm())
        return;

    QFileDialog dlg(this,tr("打开翻译文件"),"",tr("CKText (*.ckt)"));
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    if(dlg.exec() != QFileDialog::Accepted)
        return;
    auto files = dlg.selectedFiles();
    auto filename = files.back().toLocal8Bit().toStdString();
    if(!_text.open(filename.c_str()))
        message(tr("打开失败"));
    else
    {
        message(tr("打开成功"));
        _filename = filename;
        reload();
        updateTitle();
    }
}

void MainWindow::onSave()
{
    if(_filename.empty())
        onSaveAs();
    else
    {
        size_t total = 0;
        for(auto& grp : _text)
        {
            for(auto& it : grp.second)
            {
                ++total;
            }
        }

        bool compress = false;
        if(total > 2000)
            compress = Question(tr("当前文本条目大于2000, 是否需要压缩?"));
        if(_text.save(_filename.c_str()))
        {
            message(tr("保存成功"));
            _changed = false;
        }
    }
}

void MainWindow::onSaveAs()
{
    QFileDialog dlg(this,tr("保存翻译文件"),"",tr("CKText (*.ckt)"));
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    if(dlg.exec() != QFileDialog::Accepted)
        return;
    auto files = dlg.selectedFiles();
    auto filename = files.back().toLocal8Bit().toStdString();

    size_t total = 0;
    for(auto& grp : _text)
    {
        for(auto& it : grp.second)
        {
            ++total;
        }
    }

    bool compress = false;
    if(total > 2000)
        compress = Question(tr("当前文本条目大于2000, 是否需要压缩?"));

    if(!_text.save(filename.c_str(),compress))
        message(tr("保存失败"));
    else
    {
        message(tr("保存成功"));
        _filename = filename;
        _changed = false;
        updateTitle();
    }
}

void MainWindow::onProp()
{
    auto sp = ui->splitter;
    auto sizes = sp->sizes();
    if(sizes[1] < 1)
        sp->setSizes({ 100,10 });
    else
        sp->setSizes({1,0});
}

void MainWindow::onPropMenu(const QPoint &pos, PropertyWidgetItem *item)
{
    _item_prop = item;
    if(!item) return;
    auto acts = _menu_prop.actions();
    // "添加"选项在子节点无效
    acts.front()->setEnabled(item->level() < 2);
    // "删除"选项在根节点无效
    acts.back()->setEnabled(item->level() > 1);
    _menu_prop.popup(pos);
}

void MainWindow::onPropAdd()
{
    if(!_item_prop || _item_prop->level() != 1) // 只能往根节点添加
        return;
    DlgAttr dlg;
    if(dlg.exec() != QDialog::Accepted)
        return;
    auto name = dlg.name().toUtf8().toStdString();
    auto value = dlg.value();

    ck::Text::Property* prop = nullptr;
    if(_item_prop == ui->tree->item(0)) // 文档属性
        prop = &_text.prop();
    else    // 组属性
    {
        auto grp = currentGroup();
        if(!grp) return;
        prop = &grp->prop();
    }

    prop->set(name.c_str(),cast(value));
    auto idx = _item_prop->index(name.c_str());
    if(idx < 0) _item_prop->add(name.c_str(),value);
    else _item_prop->child(idx)->setValue(value);
}

void MainWindow::onPropDelete()
{
    if(!_item_prop || _item_prop->level() == 1) // 根节点不可删除
        return;
    auto parent = _item_prop->parent();
    auto name = _item_prop->name();
    auto sname = name.toStdString();

    ck::Text::Property* prop = nullptr;
    if(_item_prop == ui->tree->item(0)) // 文档属性
        prop = &_text.prop();
    else
    {
        auto grp = currentGroup();
        if(!grp) return;
        prop = &grp->prop();
    }

    prop->remove(sname.c_str());
    parent->remove(parent->index(name));
}

void MainWindow::onPropChanged(PropertyWidgetItem *item, const QVariant &value)
{
    bool restore = false;
    if(value.typeId() == QMetaType::QString)
    {
        auto text = value.toString();
        auto sstr = text.toStdString();
        if(sstr.empty())
        {
            Warning(tr("字符串需要值!"));
            restore = true;
        }
        else if(sstr.size() > 255)
        {
            Warning(tr("字符串字节长度不能大于255!"));
            restore = true;
        }
    }

    auto sname = item->name().toStdString();
    ck::Text::Property* prop = nullptr;
    if(item->parent() == ui->tree->item(0)) // 文档属性
        prop = &_text.prop();
    else  // 组属性
    {
        auto grp = currentGroup();
        if(!grp) return;
        prop = &grp->prop();
    }

    if(!restore)
        prop->set(sname.c_str(),cast(value));
    else
    {
        auto old = prop->get(sname.c_str());
        if(!old.valid()) return;

        ui->tree->blockSignals(true);
        item->setValue(cast(old));
        ui->tree->blockSignals(false);
    }
}

void MainWindow::onExportCharset()
{
    QFileDialog dlg(this,tr("选择输出文件"),{},"TXT (*.txt)");
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    if(dlg.exec() != QFileDialog::Accepted)
        return;
    QString tmp;
    std::set<QChar> charset;
    for(auto& grp : _text)
    {
        for(auto& it : grp.second)
        {
            tmp = it.first.c_str();
            for(auto& c : tmp)
                charset.insert(c);
            tmp = it.second.c_str();
            for(auto& c : tmp)
                charset.insert(c);
        }
    }
    if(charset.empty())
        return;
    auto filename = dlg.selectedFiles().front();
    QFile file(filename);
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        Warning(tr("无法打开文件: %1").arg(filename));
        return;
    }
    QTextStream ts(&file);
    ts.setEncoding(QStringConverter::Utf8);
    ts.setGenerateByteOrderMark(true);
    for(auto& c : charset)
    {
        ts << c;
    }
    file.close();
}

void to_property(const QJsonValue& jv,ck::Text::Property& out)
{
    if(!jv.isObject())
        return;
    auto jo = jv.toObject();
    for(auto& it : jo.keys())
    {
        auto k = it.toStdString();
        if(k.size() > 64) // 属性名称长度不能超过64
        {
            qWarning() << "the property name too long(>64)!";
            continue;
        }
        auto v = cast(jo[it].toVariant());
        if(v.type() == ck::var::TP_NUL)
        {
            qWarning() << "the property value invalid!";
            continue;
        }
        else if(v.type() == ck::var::TP_STRING)
        {
            if(((std::string)v).size() > 255)
            {
                qWarning() << "the property value too long(>255)!";
                continue;
            }
        }
        out.set(k.c_str(),v);
    }
}

void to_group(const QJsonValue& jv,ck::Text::Group& out)
{
    auto jo = jv.toObject();
    to_property(jo["props"],out.prop());
    auto ja = jo["trans"].toArray();
    for(auto i=ja.cbegin(); i!=ja.cend(); ++i)
    {
        auto jo = i->toObject();
        auto src = jo["src"].toString();
        if(src.isEmpty()) continue;
        auto trs = jo["trs"].toString();
        out.set(src.toStdString().c_str(),trs.toStdString().c_str());
    }
}

void MainWindow::onImportJson()
{
    auto filename = QFileDialog::getOpenFileName(this,tr("导入JSON"),"","JSON (*.json)");
    if(filename.isEmpty()) return;
    ck::Text::Group* grp = nullptr;
    QString name("json");
    {
        QString fmt("json_%1");
        int num = 0;
        while(true)
        {
            auto sname = name.toStdString();
            grp = _text.get(sname.c_str());
            if(!grp)
            {
                grp = _text.insert(sname.c_str());
                break;
            }
            ++num;
            name = fmt.arg(num);
        }
    }

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this,tr("警告"),tr("无法打开所选文件!"));
        return;
    }
    // 解析JSON
    auto json = QJsonDocument::fromJson(file.readAll());

    to_property(json["props"],_text.prop());
    to_group(json["default"],*_text.get());
    auto groups = json["groups"].toArray();
    for(auto i=groups.cbegin(); i!=groups.cend(); ++i)
    {
        auto jo = i->toObject();
        auto qname = jo["name"].toString();
        if(qname.isEmpty()) continue;
        auto name = qname.toStdString();
        if(name.size() > 64)
        {
            qWarning() << "the group name too long(>64)!";
            continue;
        }
        auto grp = _text.get(name.c_str());
        if(!grp) grp = _text.insert(name.c_str());
        if(!grp)
        {
            qWarning() << "cannot insert new group!";
            continue;
        }
        to_group(*i,*grp);
    }
    reload();
}

QJsonObject to_json(const ck::Text::Property& prop)
{
    QJsonObject jo;
    for(auto& it : prop)
    {
        jo[it.first.c_str()] = QJsonValue::fromVariant(cast(it.second));
    }
    return jo;
}

QJsonObject to_json(const ck::Text::Group& grp,const QString& name = "")
{
    QJsonObject jo;
    if(!name.isEmpty())
        jo["name"] = name;
    jo["props"] = to_json(grp.prop());
    QJsonArray ja;
    QJsonObject item;
    for(auto& it : grp)
    {
        item["src"] = it.first.c_str();
        item["trs"] = it.second.c_str();
        ja.append(item);
    }
    jo["trans"] = ja;
    return jo;
}

void MainWindow::onExportJson()
{
    auto filename = QFileDialog::getSaveFileName(this,tr("导出JSON"),"","JSON (*.json)");
    if(filename.isEmpty()) return;
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this,tr("警告"),tr("无法创建目标文件!"));
        return;
    }

    QJsonObject json;
    // 写入属性
    json["prop"] = to_json(_text.prop());

    QJsonArray groups;
    for(auto& grp : _text)
    {
        if(grp.first.empty())
            json["default"] = to_json(grp.second);
        else
            groups.append(to_json(grp.second,grp.first.c_str()));
    }
    json["groups"] = groups;

    // 输出
    QJsonDocument doc(json);
    QTextStream ts(&file);
    ts.setEncoding(QStringConverter::Utf8);
    ts << doc.toJson();
    file.close();
}

void MainWindow::onTransform()
{
    DlgTransform dlg(&_text,this);
    if(dlg.exec() == QDialog::Accepted)
        reload();
}

void MainWindow::loadPlugins()
{
#ifdef _WIN32
    constexpr auto EXT = ".dll";
#else
    constexpr auto EXT = ".so";
#endif

    QStringList strlist;
    const auto path = QDir::currentPath()+"/plugin/";
    QDir dir(path);
    for(auto& it : dir.entryList(QDir::Filter::Files))
    {
        auto pos = it.lastIndexOf('.');
        if(pos < 0)
            continue;
        auto ext = it.right(it.length() - pos);
        if(ext.compare(EXT,Qt::CaseInsensitive) != 0)
            continue;
        strlist.push_back(it);
    }

    for(auto& it : strlist)
    {
        QLibrary lib(path+it);
        if(!lib.load())
            continue;
        auto func = (PluginInstanceFunc)(lib.resolve("instance"));
        if(!func) continue;

        auto ins = func();
        connect(ins,SIGNAL(reloadRequest()),this,SLOT(reload()));

        switch (ins->type()) {
        case Plugin::IMPORT:
            auto act = new QAction(ins->name());
            act->setToolTip(ins->desc());
            if(ins->mount(act,&_text))
                ui->menu_import->addAction(act);
            break;
        }
    }
}

void MainWindow::reload()
{
    int idx = 0;
    auto oldname = ui->cob_group->currentData().toString().toStdString();
    ui->cob_group->clear();
    int num = 0;
    for(auto& it : _text)
    {
        auto& name = it.first;
        if(name.empty())
            ui->cob_group->addItem(tr("[默认组]"),name.c_str());
        else
            ui->cob_group->addItem(QString::fromUtf8(name.c_str()),name.c_str());
        if(oldname.compare(name) == 0)
            idx = num;
        ++num;
    }
    ui->cob_group->setCurrentIndex(idx);
    ui->table->resizeRowsToContents();
}

void MainWindow::refresh()
{
    auto grp = currentGroup();
    if(!grp)
    {
        ui->cob_group->setCurrentIndex(0);
        grp = _text.get();
    }
    int count = ui->table->attach(grp);
    message(tr("共%1项").arg(count));

    auto* prop = &_text.prop();
    auto item = ui->tree->item(0);
    item->clear();
    for(auto& it : *prop)
    {
        item->add(it.first.c_str(),cast(it.second));
    }

    prop = &grp->prop();
    item = ui->tree->item(1);
    item->clear();
    for(auto& it : *prop)
    {
        item->add(it.first.c_str(),cast(it.second));
    }
}

ck::Text::Group *MainWindow::currentGroup()
{
    auto name = ui->cob_group->currentData().toString().toStdString();
    return _text.get(name.c_str());
}

void MainWindow::updateTitle()
{
    if(_filename.empty())
        setWindowTitle(_title+" : 新建的");
    else
        setWindowTitle(_title+" : "+QString::fromLocal8Bit(_filename.c_str()));
}


void MainWindow::onLanguage()
{
    QSettings sets(QSettings::UserScope);
    auto act = (QAction*)sender();
    if(act == ui->act_chinese)
        sets.setValue("Language","zh_cn");
    else
        sets.setValue("Language",act->data().toString());
    Information(tr("重启后生效"));
}

