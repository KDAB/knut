#include "lsp/client.h"
#include "lsp/notifications.h"
#include "lsp/requests.h"

#include "common/test_utils.h"

#include <QTest>

class TestClient : public QObject
{
    Q_OBJECT

private slots:
    void initializeAndShutdown()
    {
        CHECK_CLANGD;

        Lsp::Client client("cpp", "clangd", {});
        auto logs = Test::LogSilencers {"cpp_client", "cpp_server", "cpp_messages"};

        QVERIFY(client.initialize(Test::testDataPath()));
        QCOMPARE(client.state(), Lsp::Client::Initialized);
        QVERIFY(client.shutdown());
        QCOMPARE(client.state(), Lsp::Client::Shutdown);
    }

    void openClose()
    {
        CHECK_CLANGD;

        Lsp::Client client("cpp", "clangd", {"--log=verbose", "--pretty"});
        auto logs = Test::LogSilencers {"cpp_client", "cpp_server", "cpp_messages"};

        client.initialize(Test::testDataPath() + "/cppdocument");

        Lsp::DidOpenTextDocumentParams openParams;
        openParams.textDocument.uri = Lsp::Client::toUri(Test::testDataPath() + "/cppdocument/test/hello.cpp");
        openParams.textDocument.version = 1;
        openParams.textDocument.text = "";
        openParams.textDocument.languageId = "cpp";
        client.didOpen(openParams);

        Lsp::DidCloseTextDocumentParams closeParams;
        closeParams.textDocument.uri = Lsp::Client::toUri(Test::testDataPath() + "/cppdocument/test/hello.cpp");
        client.didClose(closeParams);

        QVERIFY(client.shutdown());
        QCOMPARE(client.state(), Lsp::Client::Shutdown);
    }
};

QTEST_MAIN(TestClient)
#include "tst_client.moc"
