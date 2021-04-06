#include <QApplication>

#include <lsp/client.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Lsp::Client client("clangd", {});
    QObject::connect(&client, &Lsp::Client::initialized, [&client]() { client.shutdown(); });
    client.start();

    return app.exec();
}
