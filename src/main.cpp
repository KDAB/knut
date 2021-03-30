#include <QApplication>

#include <lsp/client.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Lsp::Client client("clangd", {});
    client.start();

    return app.exec();
}
