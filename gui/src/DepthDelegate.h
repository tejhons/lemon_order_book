#pragma once
#include <QAbstractItemView>
#include <QPainter>
#include <QStyledItemDelegate>
#include "BookModels.h"
#include "theme.h"

// Paints the depth bar behind each ladder row. The bar spans the whole row
// (not just one cell), anchored to the right and sized by DepthRole.
class DepthDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx) const override {
        const int kind = idx.data(LadderModel::KindRole).toInt();
        if (kind != LadderModel::Spread) {
            if (auto* view = qobject_cast<const QAbstractItemView*>(opt.widget)) {
                const int w    = view->viewport()->width();
                const int left = int(w * (1.0 - idx.data(LadderModel::DepthRole).toDouble()));
                const QRect bar = QRect(left, opt.rect.top(), w - left, opt.rect.height())
                                      .intersected(opt.rect);
                p->fillRect(bar, kind == LadderModel::Ask ? theme::askFill() : theme::bidFill());
            }
        }
        QStyledItemDelegate::paint(p, opt, idx);
    }
};
