#include "dataexchange.h"

#include "querymatch.h"

namespace Core {

/*!
 * \qmltype DataExchangeEntry
 * \brief Refers to a single entry within the `DoDataExchange`
 * \inqmlmodule Script
 * \ingroup CppDocument
 * \since 1.1
 * \sa DataExchange
 *
 * A DataExchangeEntry is a single entry within the `DoDataExchange`.
 * It contains the function, member and idc used for the entry.
 */

/*!
 * \qmlproperty string DataExchangeEntry::function
 * Funciton of the data exchange, see [Standard Dialog Data Exchange
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

DataExchangeEntry fromDDX(const QueryMatch &match, const RangeMark &range)
{
    auto function = match.getInRange("ddx-function", range).text();
    auto idc = match.getInRange("ddx-idc", range).text();
    auto member = match.getInRange("ddx-member", range).text();

    return DataExchangeEntry {.function = function, .idc = idc, .member = member};
}

/*!
 * \qmltype DataExchange
 * \brief DataExchange entries in a MFC C++ document.
 * \inqmlmodule Script
 * \ingroup CppDocument
 * \since 1.1
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
DataExchange::DataExchange(const QString &_className, const QueryMatch &query)
    : className(_className)
{
    range = query.get("do-data-exchange");

    auto ddxList = query.getAll("ddx");
    entries.reserve(ddxList.size());
    for (const auto &ddx : ddxList) {
        entries.push_back(fromDDX(query, ddx));
    }
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
