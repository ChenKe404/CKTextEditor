/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	pch.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef PCH_H
#define PCH_H

#include <QString>
#include <QMessageBox>
#include <text.h>

using Group = ck::Text::Group;
using Property = ck::Text::Property;

bool Warning(const QString& text,const QString& caption = {});

bool Information(const QString& text,const QString& caption = {});

bool Question(const QString& text,const QString& caption = {});

QVariant cast(const ck::var&);

ck::var cast(const QVariant&);

#define cstr(qstr) qstr.toStdString().c_str()

inline std::string sstr(const QString& qstr)
{ return qstr.toStdString(); }

template<typename K,typename V>
struct KV
{
    K k;
    V v;
};

template<typename K,typename V>
inline KV<K,V> make_kv(const K& k,const V& v)
{ return { k,v }; }

QString profile_path();

#endif // PCH_H
