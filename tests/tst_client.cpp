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
};

QTEST_MAIN(TestClient)
#include "tst_client.moc"
