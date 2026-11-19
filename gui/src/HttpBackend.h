#pragma once
#include <functional>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QTimer>
#include "IBookBackend.h"

// Talks to the same HTTP API the web page used:
//   GET  /book                         -> {bids:[], asks:[], trades:[]}
//   POST /order?side=&price=&qty=      -> {id, trades:[]}  or {error}
//   POST /cancel?id=                   -> {}               or {error}
class HttpBackend : public IBookBackend {
    Q_OBJECT
public:
    explicit HttpBackend(QString baseUrl, QObject* parent = nullptr);

    void start() override;
    void refresh() override;
    void placeOrder(Side side, qint64 priceCents, qint64 quantity) override;
    void cancelOrder(qint64 id) override;

private:
    void post(const QString& path, std::function<void(const QJsonObject&)> onOk);

    QString m_base;
    QNetworkAccessManager m_nam;
    QTimer m_poll;
    bool m_polling = false;
    bool m_live = false;
};
