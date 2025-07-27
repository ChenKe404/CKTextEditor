/*
*******************************************************************************
    ChenKe404's CKT file editor
*******************************************************************************
@project	cktexteditor
@authors	chenke404
@file	text_tablewidget.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "text_edit.h"
#include "text_tablewidget.h"

#include <QContextMenuEvent>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QPlainTextEdit>
#include <dlg_edit.h>
#include <QSortFilterProxyModel>
#include "char_draw.h"

class Model : public QStandardItemModel
{
    using Super = QStandardItemModel;
    Q_OBJECT
public:
    Model() : _inline_edit(true)
    {}

    inline bool inlineEdit() const
    { return _inline_edit; }

    inline void setInlineEdit(bool yes)
    { _inline_edit = yes; }

    void clearRows()
    {
        while(rowCount() > 0)
            removeRow(rowCount()-1);
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
    {
        if(index.column() == 0) // src
        {
            auto oldsrc = data(index,Qt::EditRole).toString();
            if(Super::setData(index,value,role))
            {
                auto idx = index.siblingAtColumn(1);
                auto src = value.toString();
                auto trs = data(idx,Qt::EditRole).toString();
                emit srcChanged(oldsrc,src,trs);
                return true;
            }
        }
        else if(Super::setData(index,value,role))
        {
            auto idx = index.siblingAtColumn(0);
            auto src = data(idx,Qt::EditRole).toString();
            auto trs = value.toString();
            emit trsChanged(src,trs);
            return true;
        }
        return false;
    }

    using QStandardItemModel::sort;
    void sort(TextTableWidget::Sort order)
    {
        using S = TextTableWidget::Sort;
        switch (order) {
        case S::ASC:
            Super::sort(0);
            break;
        case S::DESC:
            Super::sort(0,Qt::DescendingOrder);
            break;
        case S::UNFINISHED_TOPMOST:
            Super::sort(1,Qt::AscendingOrder);
            break;
        case S::SHORT2LONG:
        case S::LONG2SHORT:
            break;
        }
    }
signals:
    void srcChanged(const QString& oldsrc,const QString& src,const QString& trs);
    void trsChanged(const QString& src,const QString& trs);
private:
    bool _inline_edit;
};

class SortModel : public QSortFilterProxyModel
{
    using QSortFilterProxyModel::setSourceModel;
    using Sort = TextTableWidget::Sort;
public:
    SortModel()  {
        setSourceModel(new Model);
    }

    Model* source() {
        return (Model*)sourceModel();
    }

    void setOrder(Sort order) {
        _order = order;
    }

    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override {
        if(_cols.empty())
            return true;
        auto regx = filterRegularExpression();
        if(regx.pattern().isEmpty())
            return true;
        for(auto& c : _cols)
        {
            auto idx = sourceModel()->index(source_row, c, source_parent);
            if(!idx.isValid()) continue;
            auto text = idx.data(Qt::DisplayRole).toString();
            if(regx.match(text).hasMatch())
                return true;
        }
        return false;
    }

    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override {
        auto left = source_left.data(sortRole()).toString();
        auto right = source_right.data(sortRole()).toString();
        switch (_order) {
        case Sort::ASC:
            return left < right;
        case Sort::DESC:
            return right < left;
        case Sort::UNFINISHED_TOPMOST:
            return left < right;
        case Sort::SHORT2LONG:
            return left.length() < right.length();
        case Sort::LONG2SHORT:
            return right.length() < left.length();
        }
        return false;
    }

    void setFilterKeyColumns(const std::vector<int>& cols) {
        _cols = cols;
    }
private:
    Sort _order = Sort::ASC;
    std::vector<int> _cols;
};

inline Model* the_model(TextTableWidget* w)
{ return ((SortModel*)w->model())->source(); }

inline Model* the_model(const QModelIndex &i)
{ return ((SortModel*)i.model())->source(); }

class Delegate : public QStyledItemDelegate
{
    using Super = QStyledItemDelegate;
public:
    Delegate() :
        _cd_space(' ',this),
        _cd_tab('\t',this),
        _cd_line_feed(this)
    {}

    template<typename T,typename T1>
    static T* remove_cast(const T1* o) {
        return const_cast<T*>((T*)o);
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        const auto m = the_model(index);
        if(m->inlineEdit())
        {
            auto e = new TextEdit(parent);
            e->setCharDraw(remove_cast<CharDraw>(&_cd_space));
            e->setCharDraw(remove_cast<CharDraw>(&_cd_tab));
            e->setCharDraw(remove_cast<CharDraw>(&_cd_line_feed));
            return e;
        }
        else
        {
            const auto col = index.column();
            DlgEdit dlg;
            dlg.setCharDraw(remove_cast<CharDraw>(&_cd_space));
            dlg.setCharDraw(remove_cast<CharDraw>(&_cd_tab));
            dlg.setCharDraw(remove_cast<CharDraw>(&_cd_line_feed));

            dlg.setText(index.data().toString());
            if(col == 0)
            {
                dlg.setWindowTitle(tr("编辑原文"));
                dlg.setCanEmpty(false);
            }
            else
                dlg.setWindowTitle(tr("编辑译文"));

            if(dlg.exec() == QDialog::Accepted)
            {
                if(col == 0 && dlg.text().isEmpty())
                    Warning(tr("原文不可以为空!"));
                else
                {
                    auto _m = (SortModel*)index.model();
                    _m->setData(index,dlg.text());
                }
            }
            return nullptr;
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        auto m = ((SortModel*)model)->source();
        if(m->inlineEdit())
        {
            auto edit = (QPlainTextEdit*)editor;
            auto text = edit->toPlainText();
            if(index.column() == 0 && text.isEmpty())
                Warning(tr("原文不可以为空!"));
            else
                model->setData(index,text);
        }
    }

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override {
        QStyledItemDelegate::initStyleOption(option, index);
        option->displayAlignment = _alg;
    }

    void setTextAlignment(Qt::Alignment alg){
        _alg = alg;
    }

    void showSpace(bool yes){
        _cd_space.setEnable(yes);
        _cd_tab.setEnable(yes);
    }

    void showLineFeed(bool yes){
        _cd_line_feed.setEnable(yes);
    }
private:
    Qt::Alignment _alg = Qt::AlignVCenter | Qt::AlignLeft;
    CharDrawSpace _cd_space;
    CharDrawSpace _cd_tab;
    CharDrawLineFeed _cd_line_feed;
};

class VerticalHeader : public QHeaderView
{
public:
    VerticalHeader(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QHeaderView(orientation,parent)
    {
        setFixedWidth(22);
        _pix_o.load(":/res/o.svg");
        _pix_x.load(":/res/x.svg");
    }
protected:
    void paintSection(QPainter *p, const QRect &rect, int logicalIndex) const
    {
        p->save();
        QStyleOptionHeader option;
        initStyleOption(&option);
        option.rect = rect;
        option.section = logicalIndex;
        style()->drawControl(QStyle::CE_Header, &option, p, this);
        p->restore();

        p->save();
        p->setPen(QColor(200,200,200));
        p->drawLine(QLine{ rect.bottomLeft(),rect.bottomRight() });

        bool ok = true;
        auto idx = model()->index(logicalIndex,1);
        if(!idx.isValid())
            ok = false;
        else
        {
            auto v = idx.data();
            ok = v.typeId() == QMetaType::QString && !v.toString().isEmpty();
        }

        auto x = rect.x() + (rect.width() - 20) / 2;
        auto y = rect.y() + (rect.height() - 20) / 2;
        QRect rc{x,y,20,20};
        p->drawPixmap(rc,ok ? _pix_o : _pix_x);
        p->restore();
    }
private:
    QPixmap _pix_o;
    QPixmap _pix_x;
};

TextTableWidget::TextTableWidget(QWidget* parent)
    : QTableView(parent),
    _group(nullptr)
{
    setModel(new SortModel);
    setItemDelegate(new Delegate);
    auto m = the_model(this);
    m->setHorizontalHeaderLabels({ tr("原文"),tr("译文") });
    {
        auto headerH = horizontalHeader();
        headerH->setMinimumSectionSize(150);
        headerH->setStretchLastSection(true);
    }
    setVerticalHeader(new VerticalHeader(Qt::Vertical,this));
    connect(m,&Model::srcChanged,this,[this](const QString& oldsrc,const QString& src,const QString& trs){
        if(!_group) return;
        _group->remove(cstr(oldsrc));
        _group->set(cstr(src),cstr(trs));
        emit srcChanged(oldsrc,src,trs);
    });
    connect(m,&Model::trsChanged,this,[this](const QString& src,const QString& trs){
        if(!_group) return;
        _group->set(cstr(src),cstr(trs));
        emit trsChanged(src,trs);
    });

    auto act = new QAction(tr("新增"));
    connect(act,&QAction::triggered,this,[this](){
        if(!_group) return;
        DlgEdit dlg;
        dlg.setCanEmpty(false);
        dlg.setWindowTitle(tr("输入原文"));
        dlg.onCheck([this](const QString& text){
            auto sstr = ::sstr(text);
            if(_group->u8(sstr.c_str()))
            {
                Warning(tr("这个原文已存在!"));
                return false;
            }
            return true;
        });
        if(dlg.exec() != QDialog::Accepted)
            return;
        auto text = dlg.text();
        auto sstr = ::sstr(text);
        if(_group->set(sstr.c_str(),""))
        {
            auto m = the_model(this);
            auto item = new QStandardItem(dlg.text());
            m->appendRow(item);
            auto idx = m->indexFromItem(item);
            idx = ((SortModel*)model())->mapFromSource(idx);
            setCurrentIndex(idx);
        }
    });
    _menu.addAction(act);

    act = new QAction(tr("刷新"));
    connect(act,&QAction::triggered,this,&TextTableWidget::reload);
    _menu.addAction(act);

    _menu.addSeparator();
    act = new QAction(tr("清空译文"));
    connect(act,&QAction::triggered,this,[this](){
        auto rows = selectionModel()->selectedIndexes();
        if(!rows.empty() && !Question(tr("你确定要清空所选条目的译文吗?")))
            return;
        for(auto& row : rows)
        {
            auto src = row.siblingAtColumn(0).data().toString().toStdString();
            _group->set(src.c_str(),"");
            auto idx = row.siblingAtColumn(1);
            model()->setData(idx,"",Qt::DisplayRole);
        }
    });
    _menu.addAction(act);

    act = new QAction(tr("删除条目"));
    connect(act,&QAction::triggered,this,[this](){
        auto rows = selectionModel()->selectedIndexes();
        if(!rows.empty() && !Question(tr("你确定要删除所选条目吗?")))
            return;
        std::sort(rows.begin(),rows.end(),[](const auto& a,const auto& b){
            return a.row() < b.row();
        });
        for(auto i=rows.rbegin(); i!=rows.rend(); ++i)
        {
            auto src = i->siblingAtColumn(0).data().toString().toStdString();
            _group->remove(src.c_str());
            model()->removeRow(i->row());
        }
    });
    _menu.addAction(act);


    setColumnWidth(0,350);
}

int TextTableWidget::attach(ck::Text::Group *group)
{
    _group = group;
    return reload();
}

void TextTableWidget::detach()
{
    _group = nullptr;
    clear();
}

ck::Text::Group *TextTableWidget::group() const {
    return _group;
}

void TextTableWidget::setInlineEdit(bool yes)
{
    the_model(this)->setInlineEdit(yes);
}

void TextTableWidget::setTextAlignment(Qt::Alignment alg)
{
    ((Delegate*)itemDelegate())->setTextAlignment(alg);
    repaint();
}

QStandardItem *TextTableWidget::setCurrentRow(int row)
{
    auto m = (SortModel*)model();
    auto idx = m->index(row,1);
    if(idx.isValid())
    {
        setCurrentIndex(idx);
        idx = m->mapToSource(idx);
        return m->source()->itemFromIndex(idx);
    }
    return nullptr;
}

int TextTableWidget::find(int row,const QString& keyword,uint32_t flag,bool last)
{
    auto m = (SortModel*)model();
    auto count = m->rowCount();
    row = std::max(row,0);
    row = std::min(row,count - 1);
    for(int i=row;;(last ? --i : ++i))
    {
        if(last && i < 0)
            break;
        else if(!last && i >= count)
            break;

        auto idx = m->index(i,flag & SRC ? 0 : 1);
        idx = m->mapToSource(idx);
        if(!idx.isValid())
            continue;
        auto text = idx.data(Qt::EditRole).toString();
        auto pos = text.indexOf(keyword,0,flag & IGNORE_CASE ? Qt::CaseInsensitive : Qt::CaseSensitive);
        if(pos >= 0)
            return i;
    }
    return -1;
}

QModelIndex TextTableWidget::indexFromItem(QStandardItem *item)
{
    auto m = (SortModel*)model();
    auto idx = m->source()->indexFromItem(item);
    return m->mapFromSource(idx);
}

void TextTableWidget::clear()
{
    the_model(this)->clear();
}

int TextTableWidget::reload()
{
    int num = 0;
    if(!_group) return num;
    auto m = the_model(this);
    m->clearRows();
    if(_filter_flag & ONCE) {
        _filter_flag = 0;
        ((SortModel*)model())->setFilterRegularExpression("");
    }
    for(auto& it : *_group)
    {
        auto src = QString::fromUtf8(it.first.c_str());
        auto tran = QString::fromUtf8(it.second.c_str());
        m->appendRow({ new QStandardItem(src),new QStandardItem(tran) });
        ++num;
    }
    return num;
}

void TextTableWidget::filter(const QString &text,uint32_t flag)
{
    _filter_flag = flag;
    auto m = (SortModel*)model();
    std::vector<int> cols;
    if(flag & SRC)
        cols.push_back(0);
    if(flag & TRS)
        cols.push_back(1);
    if(cols.empty())
        cols.push_back(0);
    if(flag & IGNORE_CASE)
        m->setFilterCaseSensitivity(Qt::CaseInsensitive);
    else
        m->setFilterCaseSensitivity(Qt::CaseSensitive);
    m->setFilterKeyColumns(cols);
    m->setFilterRegularExpression(text);
}

void TextTableWidget::sort(Sort st)
{
    auto m = (SortModel*)model();
    m->setSortRole(Qt::DisplayRole);
    m->setOrder(st);
    if(st == UNFINISHED_TOPMOST)
    {
        if(m->sortColumn() == 1)
        {
            m->blockSignals(true);
            m->sort(0);
            m->blockSignals(false);
        }
        m->sort(1);
    }
    else
    {
        if(m->sortColumn() == 0)
        {
            m->blockSignals(true);
            m->sort(1);
            m->blockSignals(false);
        }
        m->sort(0);
    }
}

void TextTableWidget::showSpace(bool yes)
{
    ((Delegate*)itemDelegate())->showSpace(yes);
}

void TextTableWidget::showLineFeed(bool yes)
{
    ((Delegate*)itemDelegate())->showLineFeed(yes);
}

void TextTableWidget::contextMenuEvent(QContextMenuEvent *ev)
{
    _menu.popup(ev->globalPos());
}

#include <text_tablewidget.moc>
