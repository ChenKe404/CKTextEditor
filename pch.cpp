/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	pch.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "pch.h"
#include <QDir>

bool Warning(const QString &text, const QString &caption)
{
    return QMessageBox::warning(nullptr,caption.isEmpty() ? QObject::tr("警告") : caption,text) == QMessageBox::Ok;
}

bool Information(const QString &text, const QString &caption)
{
    return QMessageBox::information(nullptr,caption.isEmpty() ? QObject::tr("提示") : caption,text) == QMessageBox::Ok;
}

bool Question(const QString &text, const QString &caption)
{
    return QMessageBox::question(nullptr,caption.isEmpty() ? QObject::tr("确认") : caption,text) == QMessageBox::Yes;
}

QVariant cast(const ck::var &v)
{
    switch (v.type()) {
    case ck::var::TP_BOOL:
        return (bool)v;
    case ck::var::TP_INT:
        return (int)v;
    case ck::var::TP_FLOAT:
        return (double)(float)v;
    case ck::var::TP_STRING:
        return ((std::string)v).c_str();
        break;
    default: break;
    }
    return {};
}

ck::var cast(const QVariant &v)
{
    using Type = QMetaType::Type;
    switch (v.typeId()) {
    case Type::Bool:
        return v.toBool();
    case Type::Short:
    case Type::UShort:
    case Type::Int:
    case Type::UInt:
    case Type::Long:
    case Type::ULong:
    case Type::LongLong:
    case Type::ULongLong:
        return v.toInt();
    case Type::Float:
    case Type::Double:
        return v.toFloat();
    case Type::QString:
        return v.toString().toUtf8().toStdString();
    }
    return {};
}

constexpr const char* profile_dir = ".cktexteditor";

QString profile_path()
{
    QDir dir = QDir::home();
    if(!dir.exists(profile_dir))
        dir.mkdir(profile_dir);
    auto path = dir.absolutePath().replace('\\','/');
    if(path.back() != '/') path.append('/');
    return path.append(profile_dir).append('/');
}
