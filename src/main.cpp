#include <QApplication>

#include <core/settings.h>
#include <lsp/client.h>

#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const auto cppServer = Core::Settings::instance()->value<Core::Settings::LspServer>("/lsp/cpp");

    Lsp::Client client(cppServer.program, cppServer.arguments);
    QObject::connect(&client, &Lsp::Client::initialized, [&client]() { client.shutdown(); });
    client.start();

    return app.exec();
}
