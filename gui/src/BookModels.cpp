#include "BookModels.h"
#include <QFontDatabase>
#include <algorithm>
#include "theme.h"

static QString dollars(qint64 cents) { return QString::number(cents / 100.0, 'f', 2); }

// ---- LadderModel ---------------------------------------------------------

void LadderModel::setSnapshot(const BookSnapshot& s) {
    beginResetModel();
    m_asks = QVector<Level>(s.asks.crbegin(), s.asks.crend());   // best ask sits next to the spread
    m_bids = s.bids;

    m_maxQty = 1;
    for (const Level& l : m_asks) m_maxQty = std::max(m_maxQty, l.quantity);
    for (const Level& l : m_bids) m_maxQty = std::max(m_maxQty, l.quantity);

    if (!s.bids.isEmpty() && !s.asks.isEmpty()) {
        m_spreadText = QString("Spread $%1  (best bid %2, best ask %3)")
                           .arg(dollars(s.asks.first().price - s.bids.first().price),
                                dollars(s.bids.first().price), dollars(s.asks.first().price));
    } else if (s.bids.isEmpty() && s.asks.isEmpty()) {
        m_spreadText = "No orders yet";
    } else {
        m_spreadText = "Spread needs a bid and an ask";
    }
    endResetModel();
}

int LadderModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : int(m_asks.size() + 1 + m_bids.size());
}

Qt::ItemFlags LadderModel::flags(const QModelIndex& index) const {
    return index.isValid() ? Qt::ItemIsEnabled : Qt::NoItemFlags;
}

QVariant LadderModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    const int row = index.row(), col = index.column();
    const int askCount = int(m_asks.size());

    if (row == askCount) {                                  // spread row (column 0 is spanned)
        switch (role) {
        case Qt::DisplayRole:       return col == 0 ? QVariant(m_spreadText) : QVariant();
        case Qt::TextAlignmentRole: return int(Qt::AlignCenter);
        case Qt::ForegroundRole:    return theme::muted();
        case KindRole:              return Spread;
        }
        return {};
    }

    const bool isAsk = row < askCount;
    const Level& l = isAsk ? m_asks[row] : m_bids[row - askCount - 1];

    switch (role) {
    case Qt::DisplayRole:
        if (col == 0) return dollars(l.price);
        if (col == 1) return l.quantity;
        return l.orders;
    case Qt::TextAlignmentRole:
        return int((col == 0 ? Qt::AlignLeft : Qt::AlignRight) | Qt::AlignVCenter);
    case Qt::FontRole: {
        QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        f.setBold(col == 0);
        return f;
    }
    case Qt::ForegroundRole:
        if (col == 0) return isAsk ? theme::ask() : theme::bid();
        return {};
    case KindRole:  return isAsk ? Ask : Bid;
    case DepthRole: return double(l.quantity) / double(m_maxQty);
    }
    return {};
}

QVariant LadderModel::headerData(int section, Qt::Orientation o, int role) const {
    if (o != Qt::Horizontal) return {};
    static const char* names[] = {"Price", "Quantity", "Orders"};
    if (role == Qt::DisplayRole && section < 3) return names[section];
    if (role == Qt::TextAlignmentRole) return int((section == 0 ? Qt::AlignLeft : Qt::AlignRight) | Qt::AlignVCenter);
    return {};
}

// ---- TradesModel ---------------------------------------------------------

void TradesModel::setTrades(const QVector<Trade>& t) {
    beginResetModel();
    m_trades = t;
    endResetModel();
}

QVariant TradesModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    const Trade& t = m_trades[index.row()];
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0: return dollars(t.price);
        case 1: return t.quantity;
        case 2: return t.buyOrderId;
        default: return t.sellOrderId;
        }
    case Qt::TextAlignmentRole:
        return int((index.column() == 0 ? Qt::AlignLeft : Qt::AlignRight) | Qt::AlignVCenter);
    case Qt::FontRole:
        return QFontDatabase::systemFont(QFontDatabase::FixedFont);
    }
    return {};
}

QVariant TradesModel::headerData(int section, Qt::Orientation o, int role) const {
    if (o != Qt::Horizontal) return {};
    static const char* names[] = {"Price", "Quantity", "Buy id", "Sell id"};
    if (role == Qt::DisplayRole && section < 4) return names[section];
    if (role == Qt::TextAlignmentRole) return int((section == 0 ? Qt::AlignLeft : Qt::AlignRight) | Qt::AlignVCenter);
    return {};
}
