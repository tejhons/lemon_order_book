#pragma once
#include <deque>
#include <functional>
#include <map>
#include "IBookBackend.h"

// In-process limit order book with price-time priority. Runs with no server,
// so the GUI works out of the box. Swap for HttpBackend to use your C++ server.
class LocalBackend : public IBookBackend {
    Q_OBJECT
public:
    explicit LocalBackend(QObject* parent = nullptr) : IBookBackend(parent) {}

    void start() override;
    void refresh() override;
    void placeOrder(Side side, qint64 priceCents, qint64 quantity) override;
    void cancelOrder(qint64 id) override;

private:
    struct Order { qint64 id; qint64 qty; };

    BookSnapshot snapshot() const;
    template <typename Book>
    static void aggregate(const Book& book, QVector<Level>& out);

    std::map<qint64, std::deque<Order>, std::greater<>> m_bids;
    std::map<qint64, std::deque<Order>>                 m_asks;
    QVector<Trade> m_trades;   // newest first
    qint64 m_nextId = 1;
};
