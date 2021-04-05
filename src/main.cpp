#include <QApplication>

#include <core/languages.h>
#include <lsp/client.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Lsp::Client client(Core::Language_C | Core::Language_CXX, "clangd", {});
    QObject::connect(&client, &Lsp::Client::initialized, &client, &Lsp::Client::shutdown);
    client.start();

    return app.exec();
}
