import QtQuick 2.12
import Script 1.0
import Script.Test 1.0

TestCase {
    name: "Settings"

    function test_globalSettings() {
        verify(Settings.hasValue("/lsp/cpp/arguments"))
        compare(Settings.value("/lsp/cpp/program"), "clangd")

        // Test missing '/'
        verify(Settings.hasValue("lsp/cpp/arguments"))
        compare(Settings.value("lsp/cpp/program"), "clangd")
    }
}
