#pragma once
#include <QObject>
#include <QString>
#include "types.h"

// The GUI only ever talks to this interface. Implement it for a WebSocket feed,
// a shared-memory bridge, an in-process engine, etc. without touching the UI.
class IBookBackend : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    ~IBookBackend() override = default;

    virtual void start() = 0;      // begin polling / connect
    virtual void refresh() = 0;    // request a fresh snapshot
    virtual void placeOrder(Side side, qint64 priceCents, qint64 quantity) = 0;
    virtual void cancelOrder(qint64 id) = 0;

signals:
    void snapshotReady(const BookSnapshot& snapshot);
    void connectionChanged(bool live);
    void orderPlaced(const PlaceResult& result);
    void orderCancelled(qint64 id);
    void requestFailed(const QString& message);
};
