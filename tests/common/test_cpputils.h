/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "common/test_utils.h"
#include "core/cppdocument.h"
#include "core/knutcore.h"
#include "core/project.h"
#include "core/qmldocument.h"

#include <functional>

namespace Test {
inline void testCppDocument(const QString &projectRoot, const QString &documentPath,
                            const std::function<void(Core::CppDocument *)> &test)
{
    Core::KnutCore core;
    const auto rootFullPath = Test::testDataPath() + "/" + projectRoot;
    QVERIFY(QFileInfo::exists(rootFullPath));

    Core::Project::instance()->setRoot(rootFullPath);
    auto document = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(documentPath));
    QVERIFY(document);

    test(document);
}

inline void testQmlDocument(const QString &projectRoot, const QString &documentPath,
                            const std::function<void(Core::QmlDocument *)> &test)
{
    Core::KnutCore core;
    const auto rootFullPath = Test::testDataPath() + "/" + projectRoot;
    QVERIFY(QFileInfo::exists(rootFullPath));

    Core::Project::instance()->setRoot(rootFullPath);
    auto document = qobject_cast<Core::QmlDocument *>(Core::Project::instance()->open(documentPath));
    QVERIFY(document);

    test(document);
}

} // namespace Test
