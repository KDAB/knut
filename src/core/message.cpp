/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "message.h"
#include "utils/log.h"

#include <QVariant>

namespace Core {

/*!
 * \qmltype Message
 * \brief Singleton with methods to display different messages to the user.
 * \inqmlmodule Knut
 * \ingroup Utilities
 *
 * The `message` property in QML can be used to display different messages to the user, via logs.
 *
 * ```qml
 * Message.log("Hello World!")
 * ```
 */

Message::Message(QObject *parent)
    : QObject(parent)
{
}

Message::~Message() = default;

/*!
 * \qmlmethod Message::error(string text)
 * Shows the message `text` as an error in the log.
 */
void Message::error(const QString &text)
{
    spdlog::error("{}", text);
}

/*!
 * \qmlmethod Message::log(string text)
 * Shows the message `text` as a log in the log.
 */
void Message::log(const QString &text)
{
    spdlog::info("{}", text);
}

/*!
 * \qmlmethod Message::debug(string text)
 * Shows the message `text` as a debug in the log.
 */
void Message::debug(const QString &text)
{
    spdlog::debug("{}", text);
}

/*!
 * \qmlmethod Message::warning(string text)
 * Shows the message `text` as a warning in the log.
 */
void Message::warning(const QString &text)
{
    spdlog::warn("{}", text);
}

} // namespace Core
