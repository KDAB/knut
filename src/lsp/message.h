#pragma once

#include <QByteArray>

namespace Lsp {

/*!
 * \brief Parse the given LSP message
 * \return the content of the message, without the headers, or empty if it's invalid
 */
QByteArray parseMessage(const QByteArray &message);

/*!
 * \brief Create a new LSP message to send
 * \return the message to send, with the header + content
 */
QByteArray toMessage(const QByteArray &content);

}
