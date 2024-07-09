/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import QtQuick
import Knut
import Knut.Test

TestCase {
    name: "Settings"

    function test_globalSettings() {
        // We can't test the values, as they could change
        verify(Settings.hasValue("/lsp/servers/0/arguments"))
        verify(Settings.hasValue("/lsp/servers/0/program"))
        verify(!Settings.hasValue("/foo/bar"))

        // Test missing '/'
        verify(Settings.hasValue("lsp/servers/0/arguments"))
        verify(Settings.hasValue("lsp/servers/0/program"))
        verify(!Settings.hasValue("foo/bar"))
    }

    function test_projectSettings() {
        verify(Settings.hasValue("/foobar/foo"))
        compare(Settings.value("/lsp/servers/0/program"), "notclangd")
        compare(Settings.value("/lsp/servers/0/arguments"), ["foo", "bar"])

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
