#include "message.h"

#include <QVariant>

namespace Core {

/*!
 * \qmltype Message
 * \brief Provides methods to display different messages to the user.
 * \instantiates Core::Message
 * \inqmlmodule Script
 * \since 4.0
 *
 * The `message` property in QML can be used to display different messages to the user, via logs.
 *
 * ```qml
 * message.log("Hello World!")
 * ```
 */

Message::Message(QObject *parent)
    : QObject(parent)
{
    m_logger = spdlog::get("script");
}

Message::~Message() { }

/*!
 * \qmlmethod Message::error( string text)
 * Shows the message `text` as an error in the log.
 */
void Message::error(const QString &text)
{
    m_logger->error("{}", text.toStdString());
}

/*!
 * \qmlmethod Message::log( string text)
 * Shows the message `text` as a log in the log.
 */
void Message::log(const QString &text)
{
    m_logger->info("{}", text.toStdString());
}

/*!
 * \qmlmethod Message::debug( string text)
 * Shows the message `text` as a debug in the log.
 */
void Message::debug(const QString &text)
{
    m_logger->debug("{}", text.toStdString());
}

/*!
 * \qmlmethod Message::warning( string text)
 * Shows the message `text` as a warning in the log.
 */
void Message::warning(const QString &text)
{
    m_logger->warn("{}", text.toStdString());
}

}
