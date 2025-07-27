/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	mainwindow.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QTimer>
#include "cktext/text.h"

#include "dlg_find.h"
#include "dlg_replace.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class PropertyWidgetItem;
class QListWidgetItem;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool event(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
private:
    void message(const QString& msg);
    bool confirm();

    void onTimeout();
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onSelect(QListWidgetItem*);
    void onProp();
    void onPropMenu(const QPoint&,PropertyWidgetItem*);
    void onPropAdd();
    void onPropDelete();
    void onPropChanged(PropertyWidgetItem*,const QVariant&);
    void onExportCharset();
    void onImportJson();
    void onExportJson();
    void onTransform();
    void onLanguage();
private slots:
    void loadPlugins();
    void reload();
    void refresh();
    ck::Text::Group* currentGroup();
    void updateTitle();
private:
    Ui::MainWindow *ui;
    QString _title;
    ck::Text _text;
    std::string _filename;
    bool _changed;

    QTimer _timer;
    DlgFind _dlg_find;
    DlgReplace _dlg_replace;
    PropertyWidgetItem* _item_prop;
    QMenu _menu_prop;
    QMenu _menu_group;

    friend class DlgReplace;
};
#endif // MAINWINDOW_H
