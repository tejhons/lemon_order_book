#include "HttpBackend.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace {

QVector<Level> parseLevels(const QJsonArray& a) {
    QVector<Level> out;
    out.reserve(a.size());
    for (const QJsonValue& v : a) {
        const QJsonObject o = v.toObject();
        out.push_back({o["price"].toInteger(), o["quantity"].toInteger(), int(o["orders"].toInteger())});
    }
    return out;
}

QVector<Trade> parseTrades(const QJsonArray& a) {
    QVector<Trade> out;
    out.reserve(a.size());
    for (const QJsonValue& v : a) {
        const QJsonObject o = v.toObject();
        out.push_back({o["price"].toInteger(), o["quantity"].toInteger(),
                       o["buyOrderId"].toInteger(), o["sellOrderId"].toInteger()});
    }
    return out;
}

} // namespace

HttpBackend::HttpBackend(QString baseUrl, QObject* parent)
    : IBookBackend(parent), m_base(std::move(baseUrl)) {
    while (m_base.endsWith('/')) m_base.chop(1);
    m_poll.setInterval(1000);
    connect(&m_poll, &QTimer::timeout, this, &HttpBackend::refresh);
}

void HttpBackend::start() {
    m_poll.start();
    refresh();
}

void HttpBackend::refresh() {
    if (m_polling) return;               // don't stack requests if the server is slow
    m_polling = true;

    QNetworkReply* reply = m_nam.get(QNetworkRequest(QUrl(m_base + "/book")));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        m_polling = false;
        reply->deleteLater();

        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const bool ok = status >= 200 && status < 300;
        if (ok != m_live) { m_live = ok; emit connectionChanged(ok); }
        if (!ok) return;

        const QJsonObject o = QJsonDocument::fromJson(reply->readAll()).object();
        BookSnapshot s;
        s.bids   = parseLevels(o["bids"].toArray());
        s.asks   = parseLevels(o["asks"].toArray());
        s.trades = parseTrades(o["trades"].toArray());
        emit snapshotReady(s);
    });
}

void HttpBackend::post(const QString& path, std::function<void(const QJsonObject&)> onOk) {
    QNetworkReply* reply = m_nam.post(QNetworkRequest(QUrl(m_base + path)), QByteArray());
    connect(reply, &QNetworkReply::finished, this, [this, reply, onOk = std::move(onOk)] {
        reply->deleteLater();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QJsonObject o = QJsonDocument::fromJson(reply->readAll()).object();

        if (status == 0) {
            if (m_live) { m_live = false; emit connectionChanged(false); }
            emit requestFailed("Server not reachable.");
        } else if (status >= 400) {
            emit requestFailed(o["error"].toString("Request failed."));
        } else {
            onOk(o);
            refresh();
        }
    });
}

void HttpBackend::placeOrder(Side side, qint64 price, qint64 qty) {
    const QString path = QString("/order?side=%1&price=%2&qty=%3")
                             .arg(side == Side::Buy ? "buy" : "sell").arg(price).arg(qty);
    post(path, [this](const QJsonObject& o) {
        PlaceResult r;
        r.id = o["id"].toInteger();
        r.trades = parseTrades(o["trades"].toArray());
        emit orderPlaced(r);
    });
}

void HttpBackend::cancelOrder(qint64 id) {
    post(QString("/cancel?id=%1").arg(id), [this, id](const QJsonObject&) { emit orderCancelled(id); });
}
