/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	propertywidget.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "propertywidget.h"

#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QStyledItemDelegate>
#include <QDebug>
#include <QEvent>
#include <QHeaderView>
#include <QProxyStyle>
#include <QApplication>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QIdentityProxyModel>
#include <QItemEditorFactory>

// 封闭的ItemFlag扩展
enum PropertyWidgetItemFlag
{
    GroupTitleHidden = Qt::ItemFlag::ItemIsUserTristate << 1
};

/***** PropertyWidgetItem::Prv ******/
struct PropertyWidgetItem::Prv : public QStandardItem
{
    PropertyWidgetItem* that = nullptr;

    Prv() = default;

    // EditRole=value, DisplayRole=name
    Prv(PropertyWidgetItem* that,const QString& name,const QVariant& value)
        : that(that)
    {
        setText(name);
        setData(1,PropertyWidget::LevelRole);
        setData(value,Qt::EditRole);
    }
    ~Prv()
    {
        if(!that) return;
        that->d = nullptr;  // 将外部类的此类指针设为空, 防止重复析构
        delete that;        // 析构外部类
    }

    QStandardItem *clone() const override
    {
        auto prv = new Prv(*this);
        prv->that = nullptr;    // 克隆的私有数据类没有外部项
        return prv;
    }

    QVariant data(int role) const override
    {
        if(_value.typeId() == QMetaType::Float)
            int aa = 0;
        else if(_value.typeId() == QMetaType::QString)
            int bb = 0;
        if(role == Qt::EditRole)
            return _value;
        return QStandardItem::data(role);
    }

    void setData(const QVariant &value, int role) override
    {
        if(role == Qt::EditRole)
            _value = value;
        else
            QStandardItem::setData(value,role);
    }
private:
    QVariant _value;
};

/***** PropertyWidgetItem ******/
PropertyWidgetItem::PropertyWidgetItem(const QString &name, const QVariant &value)
    : d(new Prv(this,name,value))
{}

PropertyWidgetItem::~PropertyWidgetItem()
{
    if(d) delete d; // 私有数据类可能比此类早析构, 比如被model删除
}

PropertyWidgetItem* PropertyWidgetItem::add(const QString &name,const QVariant& value)
{
    auto item = new PropertyWidgetItem(name,value);
    add(item);
    return item;
}

void PropertyWidgetItem::add(PropertyWidgetItem *item)
{
    if(!d) return;
    d->appendRow({item->d,new Prv()});
}

PropertyWidgetItem *PropertyWidgetItem::child(int idx)
{
    if(!d) return nullptr;
    auto p = dynamic_cast<Prv*>(d->child(idx));
    if(!p) return nullptr;
    return p->that;
}

const PropertyWidgetItem *PropertyWidgetItem::child(int idx) const
{
    if(!d) return nullptr;
    auto p = dynamic_cast<Prv*>(d->child(idx));
    if(!p) return nullptr;
    return p->that;
}

PropertyWidgetItem *PropertyWidgetItem::parent() const
{
    if(!d) return nullptr;
    auto p = dynamic_cast<Prv*>(d->parent());
    if(!p) return nullptr;
    return p->that;
}

int PropertyWidgetItem::index(const PropertyWidgetItem *item) const
{
    if(!d) return -1;
    const auto count = d->rowCount();
    for(int i=0;i<count;++i)
    {
        if((PropertyWidgetItem::Prv*)d->child(i) == item->d)
            return i;
    }
    return -1;
}

int PropertyWidgetItem::index(const QString &name) const
{
    if(!d) return -1;
    const auto count = d->rowCount();
    for(int i=0;i<count;++i)
    {
        if(d->child(i)->data(Qt::DisplayRole) == name)
            return i;
    }
    return -1;
}

void PropertyWidgetItem::remove(int idx)
{
    if(!d || idx < 0 || idx >= d->rowCount())
        return;
    d->removeRow(idx);
}

int PropertyWidgetItem::count() const
{
    if(!d) return 0;
    return d->rowCount();
}

void PropertyWidgetItem::clear()
{
    if(!d) return;
    d->removeRows(0,d->rowCount());
}

void PropertyWidgetItem::setName(const QString &name)
{
    if(!d) return;
    d->setText(name);
}

QString PropertyWidgetItem::name() const
{
    if(!d) return "";
    return d->text();
}

void PropertyWidgetItem::setValue(const QVariant &value)
{
    if(!d) return;
    d->setData(value,Qt::EditRole);
}

QVariant PropertyWidgetItem::value() const
{
    if(!d) return {};
    return d->data(Qt::EditRole);
}

int PropertyWidgetItem::row() const
{
    if(!d) return 0;
    return d->row();
}

int PropertyWidgetItem::level() const
{
    if(!d) return 0;
    return d->data(PropertyWidget::LevelRole).toInt();
}

void PropertyWidgetItem::setFlags(Qt::ItemFlags flags)
{
    if(!d) return;
    d->setFlags(flags);
}

bool PropertyWidgetItem::testFlag(Qt::ItemFlag flag) const
{
    if(!d) return false;
    return d->flags().testFlag(flag);
}

void PropertyWidgetItem::setFlag(Qt::ItemFlag flag, bool on)
{
    if(!d) return;
    d->setFlags(d->flags().setFlag(flag,on));
}

/***** PropertyWidgetModel ******/
class PropertyWidgetModel : public QStandardItemModel
{
    Q_OBJECT

    using Super = QStandardItemModel;
public:
    PropertyWidgetModel()
    {
        connect(this,&Super::rowsInserted,this,&PropertyWidgetModel::onUpdateLevel);
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid()) return QVariant();
        auto idx = index;
        if(idx.column()==0)
        {
            switch (role) {
            case PropertyWidget::IndentRole:
                return _indent;
            }
        }
        else if(role == PropertyWidget::IndentRole)
            return 0;
        else
        {
            idx = index.siblingAtColumn(0);
            switch (role) {
            case Qt::DisplayRole:
                role = Qt::EditRole;
            }
        }

        return Super::data(idx,role);
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) override
    {
        bool ret = false;
        if(role == Qt::DisplayRole && index.column() == 0)
        {
            auto oldname = index.data().toString();
            auto name = value.toString();
            // 不能为空
            if(name.isEmpty() || oldname == name)
                return false;
            // 不能重名
            auto parent = Super::itemFromIndex(index.parent());
            if(parent)
            {
                auto item = Super::itemFromIndex(index);
                const auto count = parent->columnCount();
                for(int i=0;i<count;++i)
                {
                    auto child = parent->child(i);
                    if(child != item)
                    {
                        auto _name = child->data(Qt::DisplayRole).toString();
                        if(_name == name)
                            return false;
                    }
                }
            }

            ret = Super::setData(index,value,role);
            emit nameChanged(index,oldname);
        }
        else
        {
            ret = Super::setData(index,value,role);
            if(role == Qt::EditRole && index.column() == 0)
                emit valueChanged(index,value);
        }
        return ret;
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if(index.column() > 0)
            return flags(index.siblingAtColumn(0)); // 总是使用0列的标志
        return Super::flags(index);
    }

    void add(PropertyWidgetItem* item)
    { appendRow(item->d); }

    inline PropertyWidgetItem *item(int row)
    {
        return ((PropertyWidgetItem::Prv*)Super::item(row,0))->that;
    }

    inline void setIndent(int indent) { _indent = indent; }
    inline int indent() const { return _indent; }

    inline void setNameEditable(bool yes) { _name_editable = yes; }
    inline bool nameEditable() const { return _name_editable; }
signals:
    void nameChanged(const QModelIndex&,const QString& oldname);
    void valueChanged(const QModelIndex&,const QVariant&);
private:
    // 更新项和其所有子项的层级属性
    void updateLevel(QStandardItem* item,int parent_level)
    {
        if(!item) return;
        auto level = parent_level+1;
        item->setData(level,PropertyWidget::LevelRole);
        for(int i=0;i<item->rowCount();++i)
        { updateLevel(item->child(i),level); }
    }

    void onUpdateLevel(const QModelIndex &parent, int first, int last)
    {
        auto par = itemFromIndex(parent);
        if(!par) return;
        auto par_level = par->data(PropertyWidget::LevelRole).toInt();
        for(int i=first;i<=last;++i)
        { updateLevel(par->child(i),par_level); }
    }
private:
    friend class PropertyWidgetItemPrivate;
    int _indent=15;
    bool _name_editable = false;
};

/***** PropertyWidgetDelegate ******/
class PropertyWidgetDelegate : public QStyledItemDelegate
{
    using Super = QStyledItemDelegate;
public:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize sz = QStyledItemDelegate::sizeHint(option,index);
        sz.setHeight(22);
        if(index.flags().testFlag((Qt::ItemFlag)GroupTitleHidden))  // 标题行高度为0, 隐藏
            sz.setHeight(0);
        return sz;
    }

    void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_ASSERT(index.isValid());
        const int row = index.row();
        const int col = index.column();

        if(index.flags().testFlag((Qt::ItemFlag)GroupTitleHidden))  // 被隐藏的标题行不绘制
            return;

        auto w = option.widget;
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        const auto rc = opt.rect;
        auto rc_full = option.rect;
        if(col == 0) rc_full.setLeft(0);

        const bool opened = opt.state.testFlag(QStyle::State_Open);
        const bool hover = opt.state.testFlag(QStyle::State_MouseOver);
        const bool selected = opt.state.testFlag(QStyle::State_Selected);
        opt.state.setFlag(QStyle::State_MouseOver,false);
        opt.state.setFlag(QStyle::State_Selected,false);

        QPen pen;
        const int level = index.data(PropertyWidget::LevelRole).toInt();

        if(level==1)    // 根节点
        {
            if(row > 0) rc_full.setTop(rc_full.top()+1);
            p->fillRect(rc_full,qRgb(210,210,210));
        }
        else    // 其他节点
        {
            const int num = (rc.top() / rc.height());   // 序号
            // 按序号和状态绘制背景颜色
            if(selected)
                p->fillRect(rc_full,qRgb(85,180,255));
            else if(hover)
                p->fillRect(rc_full,qRgb(245,245,245));
            else if(num % 2 == 0)
                p->fillRect(rc_full,qRgb(255,255,200));
            else
                p->fillRect(rc_full,qRgb(255,255,240));
        }

        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        // 画 折叠/展开 图标
        if(col==0 && index.model()->hasChildren(index))
        {
            const float idt = index.data(PropertyWidget::IndentRole).toInt(); // 子项与父项的缩进
            auto w=5,h=9;
            if(opened) { w=9; h=5; }
            auto x = rc.left() - idt + (idt-w) * 0.5;
            auto y = rc.top() + (rc.height() - h) * 0.5;

            QPainterPath path;
            if(opened)
            {
                path.moveTo(x,y);
                path.lineTo(x + w * 0.5, y + h);
                path.lineTo(x + w, y);
            }
            else
            {
                path.moveTo(x,y);
                path.lineTo(x + w, y + h * 0.5);
                path.lineTo(x, y + h);
            }

            pen.setColor({ 128,128,128 });
            pen.setWidth(2);
            p->strokePath(path,pen);
        }
        p->restore();

        // 原始绘制
        QStyle *style = w ? w->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, p, w);

        // 画边框线
        if(level>1)
        {
            pen.setBrush(Qt::lightGray);
            pen.setWidth(1);
            p->setPen(pen);
            const auto top = rc.top();
            const auto bottom = rc.bottom();
            if(col==1)
                p->drawLine(QLine{ rc.left(),bottom,rc.right(),bottom });
            else
            {
                p->drawLines(QVector<QLine>{
                    { 0,bottom,rc.right(),bottom },
                    { rc.right(),top,rc.right(),bottom },
                });
            }
        }
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        auto m = (PropertyWidgetModel*)index.model();
        if(index.column() == 1)
            return Super::createEditor(parent,option,index);
        else if(m->nameEditable())
            return new QLineEdit(parent);
        else
            return nullptr;
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)override
    {
        if(!index.flags().testFlag(Qt::ItemIsEnabled)) return false;
        const auto type = event->type();
        if(type==QEvent::MouseButtonPress && index.column()==0)
        {
            auto e = (QMouseEvent*)event;
            if(e->button()==Qt::LeftButton)
            {
                auto& opt = const_cast<QStyleOptionViewItem&>(option);
                initStyleOption(&opt,index);
                int idt = index.data(PropertyWidget::IndentRole).toInt();
                auto rc = opt.rect;
                rc.setLeft(rc.left()-idt);
                rc.setWidth(idt);

                auto tree = const_cast<PropertyWidget*>((const PropertyWidget*)opt.widget);
                if(tree && rc.contains(e->pos())) tree->setExpanded(index,!tree->isExpanded(index));
            }
            return false;
        }
        else
            return Super::editorEvent(event,model,option,index);
    }

    void updateEditorGeometry(QWidget *e, const QStyleOptionViewItem &opt, const QModelIndex &index) const override
    {
        auto& o = const_cast<QStyleOptionViewItem&>(opt);
        o.rect.setLeft(o.rect.left()+1);
        o.rect.setHeight(o.rect.height()-1);
        e->setGeometry(o.rect);
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        if(index.column() != 0)
            Super::setEditorData(editor,index);
        else
        {
            auto e = (QLineEdit*)editor;
            e->setText(index.data().toString());
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        if(index.column() != 0)
            Super::setModelData(editor,model,index.siblingAtColumn(0));
        else
        {
            auto e = (QLineEdit*)editor;
            model->setData(index,e->text(),Qt::DisplayRole);
        }
    }
protected:
    void initStyleOption(QStyleOptionViewItem *opt, const QModelIndex &index) const override
    {
        Super::initStyleOption(opt,index);
        const int idt = index.data(PropertyWidget::IndentRole).toInt();
        // 右缩进腾出箭头的空间
        int level = index.data(PropertyWidget::LevelRole).toInt();
        if(level>1) --level;
        opt->rect.setLeft(opt->rect.left() + idt * level);
    }

};

/***** PropertyWidget ******/
PropertyWidget::PropertyWidget(QWidget *parent)
{
    _model = new PropertyWidgetModel();
    _model->setHorizontalHeaderLabels({ tr("属性"),tr("值") });
    setModel(_model);
    setItemDelegate(new PropertyWidgetDelegate);
    setEditTriggers(QTreeView::DoubleClicked);
    setIndentation(0);
    connect(_model,&PropertyWidgetModel::nameChanged,this,[this](const QModelIndex& idx,const QString& oldname){
        auto item = (PropertyWidgetItem::Prv*)_model->itemFromIndex(idx);
        emit nameChanged(item->that,oldname);
    });
    connect(_model,&PropertyWidgetModel::valueChanged,this,[this](const QModelIndex& idx,const QVariant& value){
        auto item = (PropertyWidgetItem::Prv*)_model->itemFromIndex(idx);
        emit valueChanged(item->that,value);
    });
}

int PropertyWidget::count() const
{
    return _model->rowCount();
}

PropertyWidgetItem *PropertyWidget::item(int idx) const
{
    if(idx < 0 || idx >= _model->rowCount())
        return nullptr;
    return _model->item(idx);
}

PropertyWidgetItem *PropertyWidget::add(const QString &name)
{
    auto item = new PropertyWidgetItem(name);
    item->setFlag(Qt::ItemIsEditable,false);
    _model->add(item);
    return item;
}

void PropertyWidget::setGroupTitleHidden(int row, bool hide)
{
    auto idx = _model->index(row,0);
    auto item = _model->itemFromIndex(idx);
    if(!item) return;
    auto flags = item->flags();
    flags.setFlag((Qt::ItemFlag)GroupTitleHidden,hide);
    item->setFlags(flags);
    QTreeView::setExpanded(idx,true);
}

void PropertyWidget::setGroupHidden(int row, bool hide)
{ setRowHidden(row,{},hide); }

void PropertyWidget::setExpanded(int row,bool expand)
{ QTreeView::setExpanded(_model->index(row,0),true); }

void PropertyWidget::setNameEditable(bool yes)
{
    _model->setNameEditable(yes);
}

int PropertyWidget::sizeHintForRow(int row) const
{
    const auto rowCount = _model->rowCount();
    if (row < 0 || row >= rowCount)
        return -1;
    auto item = _model->item(row)->d;
    if(!item->flags().testFlag((Qt::ItemFlag)GroupTitleHidden))
        return QTreeView::sizeHintForRow(row);

    // 隐藏组标题会导致滚动刷新出现问题, 需要重新从子集计算
    for(int r=0;r<rowCount;++r)
    {
        auto parent = _model->item(r);
        if(!parent->d->flags().testFlag((Qt::ItemFlag)GroupTitleHidden))
            return QTreeView::sizeHintForRow(r);
        else if(parent->count() < 1)
            continue;
        auto idx = _model->indexFromItem(parent->child(0)->d);
        if(idx.isValid()) return indexRowSizeHint(idx);
    }
    return 0;
}

// 不绘制以前的箭头
void PropertyWidget::drawBranches(QPainter *p, const QRect &r, const QModelIndex &i) const
{ }

void PropertyWidget::contextMenuEvent(QContextMenuEvent *event)
{
    auto pos = event->pos();
    auto idx = indexAt(pos);
    if(idx.isValid() && idx.column() > 0)
        idx = idx.siblingAtColumn(0);

    pos = event->globalPos();
    if(!idx.isValid())
        emit contextMenuRequest(pos,nullptr);
    else
        emit contextMenuRequest(pos,((PropertyWidgetItem::Prv*)_model->itemFromIndex(idx))->that);
}

#include <propertywidget.moc>
