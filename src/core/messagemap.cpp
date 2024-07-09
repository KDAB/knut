/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "messagemap.h"

#include "querymatch.h"
#include "textdocument.h"

#include <kdalgorithms.h>

namespace Core {

/*!
 * \qmltype MessageMapEntry
 * \brief Refers to a single entry within the `MessageMap`
 * \inqmlmodule Knut
 * \ingroup CppDocument
 * \sa MessageMap
 *
 * A MessageMapEntry is a single entry within the `MessageMap`.
 * It contains the name of the message as well as the list of parameters.
 */

/*!
 * \qmlproperty RangeMark MessageMapEntry::range
 * The range of the entire message call, including parameters.
 * Use this to select & delete this message from the MESSAGE_MAP entirely.
 */
/*!
 * \qmlproperty string MessageMapEntry::name
 * The name of the entry.
 */
/*!
 * \qmlproperty list<RangeMark> MessageMapEntry::parameters
 * A list of `RangeMark` instances referring to each parameter of the entry.
 */
/*!
 * \qmlproperty bool MessageMapEntry::isValid
 * Whether the struct refers to a real entry.
 * Possible reasons why a MessageMapEntry might not be valid:
 * - The entry was default-constructed.
 * - The document this entry refers to no longer exists.
 *
 * Please note that an entry that has been deleted may still be valid, but all its ranges will be empty.
 */
bool MessageMapEntry::isValid() const
{
    return range.isValid();
}

QString MessageMapEntry::toString() const
{
    return QString("%1(%2)").arg(name).arg(parameters.size());
}

MessageMapEntry fromMessage(const QueryMatch &match, const RangeMark &range)
{
    auto name = match.getInRange("message-name", range);
    auto parameters = match.getAllInRange("parameter", range);

    return MessageMapEntry {.range = range, .name = name.text(), .parameters = parameters};
}

/*!
 * \qmltype MessageMap
 * \brief Message map in a MFC C++ document
 * \inqmlmodule Knut
 * \ingroup CppDocument
 *
 * The `MessageMap` object represents the data contained in the MFC MessageMap.
 */

/*!
 * \qmlproperty string MessageMap::className
 * The name of the class this message map belongs to.
 */
/*!
 * \qmlproperty string MessageMap::superClass
 * The name of the super class this class inherits from.
 */
/*!
 * \qmlproperty RangeMark MessageMap::range
 * The entire range of the Message Map.
 * Can be used to select & delete the entire message map.
 */
/*!
 * \qmlproperty bool MessageMap::isValid
 * Whether the MessageMap refers to an actual message map.
 * If no MessageMap could be found, this will be false.
 */
/*!
 * \qmlproperty list<MessageMapEntry> MessageMap::entries
 * All entries found in the MessageMap as `MessageMapEntry`.
 */
MessageMap::MessageMap(const QueryMatch &match)
    : className(match.get("class").text())
    , superClass(match.get("superclass").text())
{
    auto begin = match.get("begin");
    auto end = match.get("end");
    range = RangeMark(begin.document(), begin.start(), end.end());

    const auto messages = match.getAll("message");
    for (const auto &message : messages) {
        entries.append(fromMessage(match, message));
    }
}

bool MessageMap::isValid() const
{
    return range.isValid();
}

/*!
 * \qmlmethod MessageMapEntry MessageMap::get(string name)
 *
 * Gets the first entry with the given `name`.
 * If no entry could be found, isValid will be false on the resulting MessageMapEntry.
 */
MessageMapEntry MessageMap::get(const QString &name) const
{
    auto entry = kdalgorithms::find_if(entries, [&name](const auto &e) {
        return e.name == name;
    });
    if (entry) {
        return *entry;
    }
    return {};
}

/*!
 * \qmlmethod list<MessageMapEntry> MessageMap::getAll(string name)
 *
 * Gets all entries with the given `name`.
 */
Core::MessageMapEntryList MessageMap::getAll(const QString &name) const
{
    return kdalgorithms::filtered(entries, [&name](const auto &entry) {
        return entry.name == name;
    });
}

QString MessageMap::toString() const
{
    return QString("MessageMap(%1, %2 entries)").arg(className).arg(entries.size());
}

} // namespace Core
