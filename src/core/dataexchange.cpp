/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "dataexchange.h"
#include "querymatch.h"

#include <kdalgorithms.h>

namespace Core {

/*!
 * \qmltype DataExchangeEntry
 * \brief Refers to a single entry within the `DoDataExchange`
 * \inqmlmodule Knut
 * \ingroup CppDocument
 * \sa DataExchange
 *
 * A DataExchangeEntry is a single entry within the `DoDataExchange`.
 * It contains the function, member and idc used for the entry.
 */

/*!
 * \qmlproperty string DataExchangeEntry::function
 * Function of the data exchange, see [Standard Dialog Data Exchange
 * Routines](https://learn.microsoft.com/en-us/cpp/mfc/reference/standard-dialog-data-exchange-routines?view=msvc-170).
 */
/*!
 * \qmlproperty string DataExchangeEntry::member
 * The member of the data exchange.
 */
/*!
 * \qmlproperty string DataExchangeEntry::idc
 * The IDC of the data exchange
 */
Q_INVOKABLE QString DataExchangeEntry::toString() const
{
    return QString("%1(%2, %3)").arg(function, idc, member);
}

QString DataValidationEntry::toString() const
{
    return QString("%1(%2, %3)").arg(function, member, arguments.join(", "));
}

static DataExchangeEntry fromDDX(const QueryMatch &ddxCall)
{
    auto function = ddxCall.get("ddx-function").text();
    auto idc = ddxCall.get("ddx-idc").text();
    auto member = ddxCall.get("ddx-member").text();

    return DataExchangeEntry {.function = function, .idc = idc, .member = member};
}

static QList<DataExchangeEntry> queryDDXCalls(const QueryMatch &ddxFunction)
{
    const auto ddxCalls = ddxFunction.queryIn("body", R"EOF(
                (expression_statement
                    (call_expression
                        function: (identifier) @ddx-function(#match? "^DDX_" @ddx-function)
                        arguments: (argument_list
                            (_)* "," ; The CDataExchange* pDX argument
                            (_)* @ddx-idc ","
                            (_)* @ddx-member
                            (#exclude! @ddx-idc @ddx-member comment)))) @ddx
    )EOF");

    return kdalgorithms::transformed(ddxCalls, fromDDX);
}

static DataValidationEntry fromDDV(const QueryMatch &ddvCall)
{
    auto function = ddvCall.get("ddv-function").text();
    auto member = ddvCall.get("ddv-member").text();
    auto arguments = kdalgorithms::transformed(ddvCall.getAll("ddv-arguments"), &RangeMark::text);

    return DataValidationEntry {.function = function, .member = member, .arguments = arguments};
}

static QList<DataValidationEntry> queryDDVCalls(const QueryMatch &ddxFunction)
{
    const auto ddvCalls = ddxFunction.queryIn("body", R"EOF(
                (expression_statement
                    (call_expression
                        function: (identifier) @ddv-function(#match? "^DDV_" @ddv-function)
                        arguments: (argument_list
                            (_)* ","
                            (_)* @ddv-member ","
                            ((_) ","?)* @ddv-arguments
                            (#exclude! @ddv-arguments @ddv-member comment)))) @ddv
    )EOF");

    return kdalgorithms::transformed(ddvCalls, fromDDV);
}

/*!
 * \qmltype DataExchange
 * \brief DataExchange entries in a MFC C++ document.
 * \inqmlmodule Knut
 * \ingroup CppDocument
 *
 * The `DataExchange` object represents the data contained in the MFC `DoDataExchange` method.
 */

/*!
 * \qmlproperty string DataExchange::className
 * The name of the class this data exchange belongs to.
 */
/*!
 * \qmlproperty list<DataExchangeEntry> DataExchange::entries
 * All entries found in the data exchange method as `DataExchangeEntry`.
 */
/*!
 * \qmlproperty RangeMark DataExchange::range
 * The entire range of the `DoDataExchange` method.
 */
DataExchange::DataExchange(QString _className, const QueryMatch &ddxFunction)
    : className(std::move(_className))
    , range(ddxFunction.get("definition"))
{
    entries = queryDDXCalls(ddxFunction);
    validators = queryDDVCalls(ddxFunction);
}

bool DataExchange::isValid() const
{
    return range.isValid();
}

QString DataExchange::toString() const
{
    return QString("DoDataExchange(%1, %2 entries)").arg(className).arg(entries.size());
}

} // namespace Core
