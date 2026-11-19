#include <QApplication>
#include <QCommandLineParser>
#include "HttpBackend.h"
#include "LocalBackend.h"
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("orderbook");

    // Needed once a backend moves to another thread (queued signal delivery).
    qRegisterMetaType<BookSnapshot>();
    qRegisterMetaType<PlaceResult>();

    QCommandLineParser cli;
    cli.addHelpOption();
    QCommandLineOption serverOpt({"s", "server"}, "Base URL of the order book server, e.g. http://localhost:8080. "
                                                  "Omit to use the built-in local engine.", "url");
    cli.addOption(serverOpt);
    cli.process(app);

    IBookBackend* backend = cli.isSet(serverOpt)
        ? static_cast<IBookBackend*>(new HttpBackend(cli.value(serverOpt), &app))
        : static_cast<IBookBackend*>(new LocalBackend(&app));

    MainWindow window(backend);
    window.show();
    return app.exec();
}
