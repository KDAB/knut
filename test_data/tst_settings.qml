import QtQuick 2.12
import Script 1.0
import Script.Test 1.0

TestCase {
    name: "Settings"

    function test_globalSettings() {
        // We can't test the values, as they could change
        verify(Settings.hasValue("/lsp/cpp/arguments"))
        verify(Settings.hasValue("/lsp/cpp/program"))
        verify(!Settings.hasValue("/foo/bar"))

        // Test missing '/'
        verify(Settings.hasValue("lsp/cpp/arguments"))
        verify(Settings.hasValue("lsp/cpp/program"))
        verify(!Settings.hasValue("foo/bar"))
    }

    function test_projectSettings() {
        verify(Settings.hasValue("/foobar/foo"))
        compare(Settings.value("/lsp/cpp/program"), "notclangd")
        compare(Settings.value("/lsp/cpp/arguments"), ["foo", "bar"])

        compare(Settings.value("/answer"), 42)
        compare(Settings.value("/pi"), 3.14)
        compare(Settings.value("/colors"), ["red", "green", "blue"]);

        // Test missing '/'
        compare(Settings.value("enabled"), true);
        compare(Settings.value("foo"), "bar");

        // Test missing values
        compare(Settings.value("/bar", "default"), "default");
        compare(Settings.value("/baz", 1), 1);

        // Test values we can't parse
        verify(Settings.hasValue("/numbers"));
        compare(Settings.value("/numbers"), undefined);
    }
}
