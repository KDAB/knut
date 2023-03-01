#include "message_map.h"

#include "querymatch.h"
#include "textdocument.h"

#include <kdalgorithms.h>
#include <spdlog/spdlog.h>

namespace Core {

/*!
 * \qmltype MessageMapEntry
 * \brief Refers to a single entry within the \c MessageMap
 * \instantiates Core::MessageMap
 *
 * A MessageMapEntry is a single entry within the \c MessageMap.
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
 * A list of \c RangeMark instances referring to each parameter of the entry.
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
    // IMPORTANT: This needs to be put into an l-value,
    // otherwise find_if will create a dangling reference!
    // See https://github.com/KDAB/KDAlgorithms/issues/50
    auto messageNames = match.getAll("message-name");
    auto name = kdalgorithms::find_if(messageNames, [&range](const auto &name) {
        return range.contains(name);
    });

    auto parameters =
        kdalgorithms::filtered<QVector<RangeMark>>(match.getAll("parameter"), [&range](const auto &param) {
            return range.contains(param);
        });

    return MessageMapEntry {.range = range, .name = name ? name->text() : "", .parameters = parameters};
}

/*!
 * \qmltype MessageMap
 * \brief Keeps track of a message map within a text document.
 * \instantiates Core::MessageMap
 *
 * The MessageMap object represents the data contained in the MFC MessageMap.
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
 * All entries found in the MessageMap as \c MessageMapEntry .
 */
MessageMap::MessageMap(const QueryMatch &match)
    : className(match.get("class").text())
    , superClass(match.get("superclass").text())
{
    auto begin = match.get("begin");
    auto end = match.get("end");
    range = RangeMark(begin.document(), begin.start(), end.end());

    auto messages = match.getAll("message");
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
 * Gets the first entry with the given \a name.
 * If no entry could be found, isValid will be false on the resulting MessageMapEntry.
 */
MessageMapEntry MessageMap::get(const QString &name) const
{
    auto entry = kdalgorithms::find_if(entries, [&name](const auto &entry) {
        return entry.name == name;
    });
    if (entry) {
        return *entry;
    }
    return {};
}

/*!
 * \qmlmethod list<MessageMapEntry> MessageMap::getAll(string name)
 *
 * Gets all entries with the given \a name.
 */
QVector<MessageMapEntry> MessageMap::getAll(const QString &name) const
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
