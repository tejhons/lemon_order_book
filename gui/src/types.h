#pragma once
#include <QtGlobal>
#include <QVector>

// All prices are integer cents, exactly like the web version's API.

enum class Side { Buy, Sell };

struct Level {
    qint64 price = 0;
    qint64 quantity = 0;
    int    orders = 0;
};

struct Trade {
    qint64 price = 0;
    qint64 quantity = 0;
    qint64 buyOrderId = 0;
    qint64 sellOrderId = 0;
};

// bids: best (highest) first. asks: best (lowest) first. trades: newest first.
struct BookSnapshot {
    QVector<Level> bids;
    QVector<Level> asks;
    QVector<Trade> trades;
};

struct PlaceResult {
    qint64 id = 0;
    QVector<Trade> trades;
};
