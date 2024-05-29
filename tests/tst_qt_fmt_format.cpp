/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "../utils/qt_fmt_format.h"

#include <QDateTime>
#include <QDebug>
#include <QRect>
#include <QRegularExpression>
#include <QSize>
#include <QTest>
#include <QUuid>

class QtFmtFormatTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testQtFmtFormat();
};

// Custom class streamable via QDebug
class CustomClass
{
};
QDebug operator<<(QDebug d, CustomClass)
{
    return d << "CustomClass()";
}

// Custom class streamable via fmt::formatBothFmtAndQDebugClass and QDebug
class BothFmtAndQDebugClass
{
};
template <>
struct fmt::formatter<BothFmtAndQDebugClass, char>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        auto it = ctx.begin();
        auto end = ctx.end();
        if (it != end && *it != '}')
            throw fmt::format_error("Only {} is supported");
        return it;
    }
    template <typename FormatContext>
    auto format(const BothFmtAndQDebugClass &, FormatContext &ctx) const
    {
        return fmt::format_to(ctx.out(), "BothFmtAndQDebugClass via std");
    }
};

QDebug operator<<(QDebug d, BothFmtAndQDebugClass)
{
    return d << "BothFmtAndQDebugClass via QDebug";
}

template <>
struct Qt_fmt::exclude_from_qdebug_fmt<BothFmtAndQDebugClass> : fmt::true_type
{
};

void QtFmtFormatTest::testQtFmtFormat()
{
    // This is a bit sketchy because it depends on QDebug's format. Better than nothing?
    using namespace fmt::literals;

    // Fundamental / Standard Library types supported by fmt::format _and_ QDebug;
    // should all go through fmt::format
    QCOMPARE(fmt::format("{}", 42), "42");
    QCOMPARE(fmt::format("{}", 10.5), "10.5");
    QCOMPARE(fmt::format("{}", "hello"), "hello");
    QCOMPARE(fmt::format("{}", (const char *)"hello"), "hello");

    QCOMPARE(fmt::format("{}", "hello"s), "hello");
    QCOMPARE(fmt::format("{}", "hello"sv), "hello");

    // Qt types
    QCOMPARE(fmt::format("{}", QStringLiteral("hello")), "hello");
    QCOMPARE(fmt::format("{}", QByteArray("hello")), "hello");
    QCOMPARE(fmt::format("{}", QDateTime(QDate(2000, 2, 29), QTime(12, 12, 12), Qt::UTC)),
             "QDateTime(2000-02-29 12:12:12.000 UTC Qt::UTC)");
    QCOMPARE(fmt::format("{}", QUuid()), "QUuid({00000000-0000-0000-0000-000000000000})");
    QCOMPARE(fmt::format("{}", QRect()), "QRect(0,0 0x0)");
    QCOMPARE(fmt::format("{}", QSizeF()), "QSizeF(-1, -1)");

    // Q_FLAG / Q_ENUM
    QCOMPARE(fmt::format("{}", Qt::Alignment(Qt::AlignLeft | Qt::AlignVCenter)),
             "QFlags<Qt::AlignmentFlag>(AlignLeading|AlignVCenter)");
    QCOMPARE(fmt::format("{}", Qt::AlignLeft), "Qt::AlignLeft");

    // User defined types
    QCOMPARE(fmt::format("{}", CustomClass()), "CustomClass()");
    QCOMPARE(fmt::format("{}", BothFmtAndQDebugClass()), "BothFmtAndQDebugClass via std");
}

static_assert(Qt_fmt::detail::IsFormattableViaQDebug<QString>);
static_assert(Qt_fmt::detail::IsFormattableViaQDebug<QByteArray>);
static_assert(Qt_fmt::detail::IsFormattableViaQDebug<QDateTime>);
static_assert(Qt_fmt::detail::IsFormattableViaQDebug<QUuid>);
static_assert(Qt_fmt::detail::IsFormattableViaQDebug<QRect>);

QTEST_APPLESS_MAIN(QtFmtFormatTest)

#include "tst_qt_fmt_format.moc"
