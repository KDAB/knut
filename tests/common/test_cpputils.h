#pragma once

#include "common/test_utils.h"
#include "core/cppdocument.h"
#include "core/knutcore.h"
#include "core/project.h"

#include <functional>

namespace Test {
void testCppDocument(const QString &projectRoot, const QString &documentPath,
                     std::function<void(Core::CppDocument *)> test)
{
    Core::KnutCore core;
    const auto rootFullPath = Test::testDataPath() + "/" + projectRoot;
    QVERIFY(QFileInfo::exists(rootFullPath));

    Core::Project::instance()->setRoot(rootFullPath);
    auto document = qobject_cast<Core::CppDocument *>(Core::Project::instance()->open(documentPath));
    QVERIFY(document);

    test(document);
}

} // namespace Test
