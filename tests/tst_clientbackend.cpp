#include "common/test_utils.h"
#include "lsp/clientbackend.h"
#include "lsp/notificationmessage_json.h"
#include "lsp/notifications.h"
#include "lsp/requestmessage_json.h"
#include "lsp/requests.h"
#include "lsp/types_json.h"

#include <QSignalSpy>
#include <QTest>

class TestClientBackend : public QObject
{
    Q_OBJECT

private slots:
    void sendSynchronousRequets()
    {
        CHECK_CLANGD;

        Lsp::ClientBackend client("cpp", "clangd", {});
        auto logs = Test::LogSilencers {"cpp_server", "cpp_messages"};

        QSignalSpy errorOccured(&client, &Lsp::ClientBackend::errorOccured);
        QSignalSpy finished(&client, &Lsp::ClientBackend::finished);
        client.start();

        Lsp::InitializeRequest initializeRequest;
        initializeRequest.id = 1;
        auto initializeResponse = client.sendRequest(initializeRequest);
        QVERIFY(initializeResponse.isValid());
        QVERIFY(!initializeResponse.error);
        client.sendNotification(Lsp::InitializedNotification());

        Lsp::ShutdownRequest shutdownRequest;
        shutdownRequest.id = 2;
        auto shutdownResponse = client.sendRequest(shutdownRequest);
        QVERIFY(shutdownResponse.isValid());
        QVERIFY(!shutdownResponse.error);
        client.sendNotification(Lsp::ExitNotification());

        QCOMPARE(errorOccured.count(), 0);
        finished.wait();
        QVERIFY(finished.count());
    }

    void sendAsynchronousRequets()
    {
        CHECK_CLANGD;

        Lsp::ClientBackend client("cpp", "clangd", {});
        auto logs = Test::LogSilencers {"cpp_server", "cpp_messages"};

        QSignalSpy errorOccured(&client, &Lsp::ClientBackend::errorOccured);
        QSignalSpy finished(&client, &Lsp::ClientBackend::finished);
        client.start();

        auto shutdownCallback = [&](Lsp::ShutdownRequest::Response response) {
            if (response.isValid() && !response.error)
                client.sendNotification(Lsp::ExitNotification());
        };
        auto initializeCallback = [&](Lsp::InitializeRequest::Response response) {
            if (response.isValid() && !response.error) {
                client.sendNotification(Lsp::InitializedNotification());
                Lsp::ShutdownRequest shutdownRequest;
                shutdownRequest.id = 2;
                client.sendAsyncRequest(shutdownRequest, shutdownCallback);
            }
        };

        Lsp::InitializeRequest initializeRequest;
        initializeRequest.id = 1;
        client.sendAsyncRequest(initializeRequest, initializeCallback);

        QCOMPARE(errorOccured.count(), 0);
        finished.wait();
        QVERIFY(finished.count());
    }
};

QTEST_MAIN(TestClientBackend)
#include "tst_clientbackend.moc"
