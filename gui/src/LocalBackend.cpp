#include "LocalBackend.h"
#include <algorithm>

template <typename Book>
void LocalBackend::aggregate(const Book& book, QVector<Level>& out) {
    for (const auto& [price, queue] : book) {
        Level l; l.price = price; l.orders = int(queue.size());
        for (const Order& o : queue) l.quantity += o.qty;
        out.push_back(l);
    }
}

BookSnapshot LocalBackend::snapshot() const {
    BookSnapshot s;
    aggregate(m_bids, s.bids);
    aggregate(m_asks, s.asks);
    s.trades = m_trades;
    return s;
}

void LocalBackend::start() {
    // Seed a little liquidity so the ladder isn't empty on first launch.
    for (int i = 1; i <= 4; ++i) {
        placeOrder(Side::Buy,  10000 - i * 10, 10 * i);
        placeOrder(Side::Sell, 10000 + i * 10, 10 * i);
    }
    emit connectionChanged(true);
    emit snapshotReady(snapshot());
}

void LocalBackend::refresh() { emit snapshotReady(snapshot()); }

void LocalBackend::placeOrder(Side side, qint64 price, qint64 quantity) {
    if (price <= 0 || quantity <= 0) {
        emit requestFailed("Price and quantity must be above zero.");
        return;
    }

    PlaceResult result;
    result.id = m_nextId++;
    qint64 remaining = quantity;

    auto match = [&](auto& book, auto crosses, bool buying) {
        while (remaining > 0 && !book.empty()) {
            auto it = book.begin();                  // best opposite level
            if (!crosses(it->first)) break;
            auto& queue = it->second;
            while (remaining > 0 && !queue.empty()) {
                Order& resting = queue.front();
                const qint64 fill = std::min(remaining, resting.qty);
                Trade t{it->first, fill,
                        buying ? result.id : resting.id,
                        buying ? resting.id : result.id};
                result.trades.push_back(t);
                m_trades.prepend(t);
                remaining -= fill;
                resting.qty -= fill;
                if (resting.qty == 0) queue.pop_front();
            }
            if (queue.empty()) book.erase(it);
        }
    };

    if (side == Side::Buy) {
        match(m_asks, [&](qint64 p) { return p <= price; }, true);
        if (remaining > 0) m_bids[price].push_back({result.id, remaining});
    } else {
        match(m_bids, [&](qint64 p) { return p >= price; }, false);
        if (remaining > 0) m_asks[price].push_back({result.id, remaining});
    }

    if (m_trades.size() > 50) m_trades.resize(50);
    emit orderPlaced(result);
    emit snapshotReady(snapshot());
}

void LocalBackend::cancelOrder(qint64 id) {
    auto tryErase = [id](auto& book) {
        for (auto it = book.begin(); it != book.end(); ++it) {
            auto& q = it->second;
            auto found = std::find_if(q.begin(), q.end(), [id](const Order& o) { return o.id == id; });
            if (found != q.end()) {
                q.erase(found);
                if (q.empty()) book.erase(it);
                return true;
            }
        }
        return false;
    };

    if (tryErase(m_bids) || tryErase(m_asks)) {
        emit orderCancelled(id);
        emit snapshotReady(snapshot());
    } else {
        emit requestFailed(QString("Order %1 not found.").arg(id));
    }
}
