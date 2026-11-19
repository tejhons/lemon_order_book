#pragma once
#include <QMainWindow>
#include "types.h"

class IBookBackend;
class LadderModel;
class TradesModel;
class QTableView;
class QDoubleSpinBox;
class QSpinBox;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(IBookBackend* backend, QWidget* parent = nullptr);

private slots:
    void onSnapshot(const BookSnapshot& s);
    void onConnection(bool live);
    void onPlaced(const PlaceResult& r);
    void onCancelled(qint64 id);
    void onFailed(const QString& message);

private:
    void buildUi();
    void submit(Side side);
    void cancelClicked();
    void say(const QString& text, bool isError = false);

    IBookBackend* m_backend;
    LadderModel*  m_ladder;
    TradesModel*  m_trades;
    QTableView*   m_ladderView = nullptr;
    QDoubleSpinBox* m_price = nullptr;
    QSpinBox*     m_qty = nullptr;
    QSpinBox*     m_cancelId = nullptr;
    QLabel*       m_message = nullptr;
    QLabel*       m_status = nullptr;
    qint64        m_pendingQty = 0;
};
