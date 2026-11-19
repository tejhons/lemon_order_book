#pragma once
#include <QAbstractTableModel>
#include "types.h"

// Price ladder: asks (worst -> best), a spread row, then bids (best -> worst).
class LadderModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Kind { Ask, Bid, Spread };
    enum Role { KindRole = Qt::UserRole + 1, DepthRole };   // DepthRole: 0..1 share of largest level

    using QAbstractTableModel::QAbstractTableModel;

    void setSnapshot(const BookSnapshot& s);
    int  spreadRow() const { return int(m_asks.size()); }

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : 3; }
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation o, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    QVector<Level> m_asks;   // stored in display order (best ask last)
    QVector<Level> m_bids;   // best bid first
    qint64  m_maxQty = 1;
    QString m_spreadText = "No orders yet";
};

class TradesModel : public QAbstractTableModel {
    Q_OBJECT
public:
    using QAbstractTableModel::QAbstractTableModel;

    void setTrades(const QVector<Trade>& t);

    int rowCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : int(m_trades.size()); }
    int columnCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : 4; }
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation o, int role) const override;

private:
    QVector<Trade> m_trades;
};
