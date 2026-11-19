#include "MainWindow.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTableView>
#include <QVBoxLayout>

#include "BookModels.h"
#include "DepthDelegate.h"
#include "IBookBackend.h"
#include "theme.h"

namespace {

void styleTable(QTableView* v) {
    v->setSelectionMode(QAbstractItemView::NoSelection);
    v->setEditTriggers(QAbstractItemView::NoEditTriggers);
    v->setShowGrid(false);
    v->setFocusPolicy(Qt::NoFocus);
    v->verticalHeader()->hide();
    v->verticalHeader()->setDefaultSectionSize(26);
    v->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    v->horizontalHeader()->setHighlightSections(false);
}

} // namespace

MainWindow::MainWindow(IBookBackend* backend, QWidget* parent)
    : QMainWindow(parent), m_backend(backend),
      m_ladder(new LadderModel(this)), m_trades(new TradesModel(this)) {
    setWindowTitle("Order Book");
    resize(1000, 640);
    buildUi();

    connect(m_backend, &IBookBackend::snapshotReady,     this, &MainWindow::onSnapshot);
    connect(m_backend, &IBookBackend::connectionChanged, this, &MainWindow::onConnection);
    connect(m_backend, &IBookBackend::orderPlaced,       this, &MainWindow::onPlaced);
    connect(m_backend, &IBookBackend::orderCancelled,    this, &MainWindow::onCancelled);
    connect(m_backend, &IBookBackend::requestFailed,     this, &MainWindow::onFailed);

    m_backend->start();
}

void MainWindow::buildUi() {
    // --- left: price ladder ---
    m_ladderView = new QTableView;
    m_ladderView->setModel(m_ladder);
    m_ladderView->setItemDelegate(new DepthDelegate(m_ladderView));
    styleTable(m_ladderView);
    // The spread row spans all columns; re-apply after every model reset.
    connect(m_ladder, &QAbstractItemModel::modelReset, this, [this] {
        m_ladderView->clearSpans();
        m_ladderView->setSpan(m_ladder->spreadRow(), 0, 1, 3);
    });
    m_ladderView->setSpan(m_ladder->spreadRow(), 0, 1, 3);

    auto* ladderBox = new QGroupBox("Price levels");
    auto* ladderLayout = new QVBoxLayout(ladderBox);
    ladderLayout->addWidget(m_ladderView);

    // --- right: place order ---
    m_price = new QDoubleSpinBox;
    m_price->setDecimals(2);
    m_price->setRange(0.01, 1'000'000.0);
    m_price->setSingleStep(0.01);
    m_price->setValue(100.00);
    m_price->setPrefix("$");

    m_qty = new QSpinBox;
    m_qty->setRange(1, 1'000'000'000);
    m_qty->setValue(10);

    auto* buy  = new QPushButton("Buy");
    auto* sell = new QPushButton("Sell");
    buy->setStyleSheet(QString("QPushButton { background:%1; color:white; font-weight:600; padding:7px; border-radius:4px; }")
                           .arg(theme::bid().name()));
    sell->setStyleSheet(QString("QPushButton { background:%1; color:white; font-weight:600; padding:7px; border-radius:4px; }")
                            .arg(theme::ask().name()));
    connect(buy,  &QPushButton::clicked, this, [this] { submit(Side::Buy); });
    connect(sell, &QPushButton::clicked, this, [this] { submit(Side::Sell); });

    m_message = new QLabel;
    m_message->setWordWrap(true);
    m_message->setMinimumHeight(m_message->fontMetrics().height() * 2);

    auto* orderBox = new QGroupBox("Place a limit order");
    auto* grid = new QGridLayout(orderBox);
    grid->addWidget(new QLabel("Price"),    0, 0);
    grid->addWidget(new QLabel("Quantity"), 0, 1);
    grid->addWidget(m_price, 1, 0);
    grid->addWidget(m_qty,   1, 1);
    grid->addWidget(buy,     2, 0);
    grid->addWidget(sell,    2, 1);
    grid->addWidget(m_message, 3, 0, 1, 2);

    // --- right: cancel ---
    m_cancelId = new QSpinBox;
    m_cancelId->setRange(1, 2'000'000'000);
    auto* cancelBtn = new QPushButton("Cancel order");
    connect(cancelBtn, &QPushButton::clicked, this, &MainWindow::cancelClicked);

    auto* cancelBox = new QGroupBox("Cancel an order");
    auto* cancelLayout = new QHBoxLayout(cancelBox);
    auto* idCol = new QFormLayout;
    idCol->addRow("Order id", m_cancelId);
    cancelLayout->addLayout(idCol, 1);
    cancelLayout->addWidget(cancelBtn);

    // --- right: recent trades ---
    auto* tradesView = new QTableView;
    tradesView->setModel(m_trades);
    styleTable(tradesView);
    auto* tradesBox = new QGroupBox("Recent trades");
    auto* tradesLayout = new QVBoxLayout(tradesBox);
    tradesLayout->addWidget(tradesView);

    auto* right = new QVBoxLayout;
    right->addWidget(orderBox);
    right->addWidget(cancelBox);
    right->addWidget(tradesBox, 1);

    auto* central = new QWidget;
    auto* root = new QHBoxLayout(central);
    root->addWidget(ladderBox, 14);
    root->addLayout(right, 10);
    setCentralWidget(central);

    m_status = new QLabel("Connecting…");
    statusBar()->addPermanentWidget(m_status);
}

void MainWindow::submit(Side side) {
    const qint64 cents = qRound64(m_price->value() * 100.0);   // dollars -> integer cents
    m_pendingQty = m_qty->value();
    m_backend->placeOrder(side, cents, m_pendingQty);
}

void MainWindow::cancelClicked() { m_backend->cancelOrder(m_cancelId->value()); }

void MainWindow::onSnapshot(const BookSnapshot& s) {
    m_ladder->setSnapshot(s);
    m_trades->setTrades(s.trades);
}

void MainWindow::onConnection(bool live) {
    m_status->setText(live ? "Live" : "Server not reachable");
    m_status->setStyleSheet(live ? QString() : QString("color:%1").arg(theme::ask().name()));
}

void MainWindow::onPlaced(const PlaceResult& r) {
    qint64 filled = 0;
    for (const Trade& t : r.trades) filled += t.quantity;
    say(QString("Order %1 placed: %2 of %3 filled.").arg(r.id).arg(filled).arg(m_pendingQty));
}

void MainWindow::onCancelled(qint64 id) { say(QString("Order %1 cancelled.").arg(id)); }
void MainWindow::onFailed(const QString& message) { say(message, true); }

void MainWindow::say(const QString& text, bool isError) {
    m_message->setText(text);
    m_message->setStyleSheet(isError ? QString("color:%1").arg(theme::ask().name()) : QString());
}
