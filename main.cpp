/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	main.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "mainwindow.h"

#include <QApplication>
#include <QItemEditorFactory>
#include <QSettings>
#include <QTranslator>
#include <windows.h>

bool load_language(QApplication& a,const QString& locName)
{
    static QTranslator tra;
    if(tra.load("language/cktexteditor_" + locName + ".qm"))
        return a.installTranslator(&tra);
    return false;
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("ChenKe404");
    QCoreApplication::setApplicationName("CKTextEditor");

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/res/icon.png"));

    QSettings sets(QSettings::UserScope);
    auto lang = sets.value("Language","en_us");
    if(lang != "zh_cn")
        load_language(a,lang.toString());

    MainWindow w;
    w.show();
    return a.exec();
}
