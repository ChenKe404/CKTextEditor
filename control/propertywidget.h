/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	propertywidget.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef PROPERTYWIDGET_H
#define PROPERTYWIDGET_H

#include <QObject>
#include <QTreeView>

class PropertyWidgetModel;
class PropertyWidgetItem
{
private:
    PropertyWidgetItem(const PropertyWidgetItem&) = delete;
public:
    PropertyWidgetItem(const QString& name,const QVariant& value = "");
    ~PropertyWidgetItem();

    PropertyWidgetItem* add(const QString& name,const QVariant& value = "");
    void add(PropertyWidgetItem* item);

    PropertyWidgetItem* child(int idx);
    const PropertyWidgetItem* child(int idx) const;
    PropertyWidgetItem* parent() const;

    int index(const PropertyWidgetItem*) const;
    int index(const QString& name) const;
    void remove(int idx);
    int count() const;
    void clear();

    void setName(const QString& name);
    QString name() const;

    void setValue(const QVariant& value);
    QVariant value() const;

    int row() const;
    int level() const;

    void setFlags(Qt::ItemFlags flags);
    bool testFlag(Qt::ItemFlag flag) const;
    void setFlag(Qt::ItemFlag flag,bool on = true);
private:
    struct Prv;
    Prv* d;
    friend class PropertyWidgetModel;
    friend class PropertyWidget;
};

class PropertyWidget : public QTreeView
{
    Q_OBJECT
public:
    enum Role
    {
        LevelRole = Qt::UserRole + 1,
        IndentRole = Qt::UserRole + 2,
    };
public:
    PropertyWidget(QWidget* parent = nullptr);
    int count() const;
    PropertyWidgetItem* item(int idx) const;
    PropertyWidgetItem* add(const QString& name);
    void setGroupTitleHidden(int row,bool hide = true); // 隐藏组标题行
    void setGroupHidden(int row,bool hide = true);      // 隐藏组, 包括子行
    void setExpanded(int row,bool expand = true);
    void setNameEditable(bool yes);
    using QTreeView::setExpanded;

    int sizeHintForRow(int row) const override;
signals:
    void nameChanged(PropertyWidgetItem* item,const QString& oldname);
    void valueChanged(PropertyWidgetItem* item,const QVariant& value);
    void contextMenuRequest(QPoint,PropertyWidgetItem*);

private:
    using QTreeView::setModel;
    using QTreeView::setIndentation;
    using QTreeView::indentation;
    using QTreeView::clicked;
private:
    PropertyWidgetModel* _model;
protected:
    void drawBranches(QPainter *, const QRect &, const QModelIndex &) const override;
    void contextMenuEvent(QContextMenuEvent *event) override;
};

#endif // PROPERTYWIDGET_H
