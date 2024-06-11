/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qttsdocument.h"
#include "logger.h"
#include "utils/log.h"

#include <QFile>
#include <QUiLoader>
#include <QWidget>
#include <kdalgorithms.h>

namespace Core {

/*!
 * \qmltype QtTsDocument
 * \brief Provides access to the content of a Ts file (Qt linguist).
 * \inqmlmodule Script
 * \ingroup QtTsDocument/@first
 * \since 1.0
 */

/*!
 * \qmlproperty array<QtUiMessage> QtTsDocument::messages
 * List of all translations in the ts file.
 */

/*!
 * \qmlproperty string QtTsDocument::language
 * Return language name.
 */

/*!
 * \qmlproperty string QtTsDocument::sourceLanguage
 * Return source language name.
 */

QtTsDocument::QtTsDocument(QObject *parent)
    : TextDocument(Type::QtTs, parent)
{
}

/*!
 * \qmlmethod QtTsDocument::setLanguage(string lang)
 * Change language.
 */
void QtTsDocument::setLanguage(const QString &lang)
{
    LOG("QtTsDocument::setLanguage", lang);
    auto ts = m_document.select_node("TS");
    ts.node().attribute("language").set_value(lang.toLatin1().constData());

    setHasChanged(true);
    Q_EMIT languageChanged();
}

/*!
 * \qmlmethod QtTsDocument::setSourceLanguage(string lang)
 * Change source language.
 */
void QtTsDocument::setSourceLanguage(const QString &lang)
{
    LOG("QtTsDocument::setSourceLanguage", lang);
    auto ts = m_document.select_node("TS");
    ts.node().attribute("sourcelanguage").set_value(lang.toLatin1().constData());
    setHasChanged(true);
    Q_EMIT sourceLanguageChanged();
}

void QtTsDocument::addMessage(pugi::xml_node contextChild, const QString &context, const QString &location,
                              const QString &source, const QString &translation, const QString &comment)
{
    pugi::xml_node messageChild = contextChild.append_child("message"); // message

    pugi::xml_node locationChild = messageChild.append_child("location"); // add location
    locationChild.append_attribute("filename").set_value(location.toLatin1().constData());

    pugi::xml_node sourceChild = messageChild.append_child("source"); // add source
    sourceChild.append_child(pugi::node_pcdata).set_value(source.toUtf8().constData());

    if (!comment.isEmpty()) {
        pugi::xml_node commentChild = messageChild.append_child("comment"); // add source
        commentChild.append_child(pugi::node_pcdata).set_value(comment.toUtf8().constData());
    }

    pugi::xml_node translationChild = messageChild.append_child("translation"); // add translation
    translationChild.append_child(pugi::node_pcdata).set_value(translation.toUtf8().constData());

    m_messages.push_back(new QtTsMessage(context, messageChild, this));
}

/*!
 * \qmlmethod QtTsDocument::addMessage(string context, string location, string source, string translation)
 * Add a new source text, its translation located in location within the given context.
 */

void QtTsDocument::addMessage(const QString &context, const QString &fileName, const QString &source,
                              const QString &translation, const QString &comment)
{
    LOG("QtTsDocument::addMessage", context, fileName, source, translation, comment);
    if (fileName.isEmpty() || source.isEmpty() || context.isEmpty()) {
        spdlog::error(R"(Location or context or source is empty)");
    }

    auto isContext = [context](const auto &data) {
        return context == data->context();
    };
    auto contextNode = std::ranges::find_if(m_messages, isContext);

    if (contextNode == m_messages.end()) {
        const auto ts = m_document.select_node("TS");
        pugi::xml_node contextChild = ts.node().append_child("context");

        pugi::xml_node nameChild = contextChild.append_child("name"); // add name
        nameChild.append_child(pugi::node_pcdata).set_value(context.toLatin1().constData());
        addMessage(contextChild, context, fileName, source, translation, comment);
    } else {
        const auto contexts = m_document.select_nodes("//context");
        for (const auto &node : contexts) {
            Q_ASSERT(!node.node().empty());
            const QString contextName = QString::fromLatin1(node.node().child("name").text().as_string());
            if (contextName == context) {
                addMessage(node.node(), context, fileName, source, translation, comment);
                break;
            }
        }
    }
    // m_document.save_file("foo.xml"); // Debug create foo.xml
    Q_EMIT messagesChanged();
}

bool QtTsDocument::doSave(const QString &fileName)
{
    return m_document.save_file(fileName.toLatin1().constData(), "    ");
}

bool QtTsDocument::doLoad(const QString &fileName)
{
    m_messages.clear();
    pugi::xml_parse_result result = m_document.load_file(fileName.toLatin1().constData(), pugi::parse_declaration);

    if (!result) {
        spdlog::critical("{}({}): {}", fileName, result.offset, result.description());
        return false;
    }

    const auto ts = m_document.select_nodes("TS");
    if (ts.empty()) {
        spdlog::critical("invalid file {}", fileName);
        return false;
    }

    const auto contexts = m_document.select_nodes("//context");
    for (const auto &node : contexts) {
        Q_ASSERT(!node.node().empty());
        const QString contextName = QString::fromLatin1(node.node().child("name").text().as_string());

        const auto messages = node.node().select_nodes("message");
        for (const auto &message : messages) {

            m_messages.push_back(new QtTsMessage(contextName, message.node(), this));
        }
    }
    return true;
}

QString QtTsDocument::sourceLanguage() const
{
    const auto ts = m_document.select_node("TS");
    return QString::fromLatin1(ts.node().attribute("sourcelanguage").value());
}

QString QtTsDocument::language() const
{
    const auto ts = m_document.select_node("TS");
    return QString::fromLatin1(ts.node().attribute("language").value());
}

QVector<QtTsMessage *> QtTsDocument::messages() const
{
    return m_messages;
}

/*!
 * \qmltype QtTsMessage
 * \brief Provides access to message.
 * \inqmlmodule Script
 * \ingroup QtTsDocument
 * \since 1.0
 * \sa QtTsDocument
 */

/*!
 * \qmlproperty string QtTsMessage::fileName
 * FileName where come from translate string.
 */

/*!
 * \qmlproperty string QtTsMessage::source
 * Original string which must be translated.
 */

/*!
 * \qmlproperty string QtTsMessage::translation
 * String translated in specific language.
 */

/*!
 * \qmlproperty string QtTsMessage::context
 * Translate context (read only).
 */

/*!
 * \qmlproperty string QtTsMessage::comment
 * Define comment.
 */

QtTsMessage::QtTsMessage(const QString &context, pugi::xml_node message, QObject *parent)
    : QObject(parent)
    , m_context(context)
    , m_message(message)
{
}

QString QtTsMessage::fileName() const
{
    return QString::fromLatin1(m_message.child("location").attribute("filename").value());
}

void QtTsMessage::setFileName(const QString &file)
{
    LOG("QtTsMessage::setFileName", file);
    m_message.child("location").attribute("filename").set_value(file.toLatin1().constData());
    qobject_cast<QtTsDocument *>(parent())->setHasChanged(true);
    Q_EMIT fileNameChanged();
}

QString QtTsMessage::comment() const
{
    return QString::fromLatin1(m_message.child("comment").text().as_string());
}

void QtTsMessage::setComment(const QString &comment)
{
    LOG("QtTsMessage::setComment", comment);
    m_message.child("comment").set_value(comment.toLatin1().constData());
    qobject_cast<QtTsDocument *>(parent())->setHasChanged(true);
    Q_EMIT commentChanged();
}

QString QtTsMessage::source() const
{
    return QString::fromLatin1(m_message.child("source").text().as_string());
}

void QtTsMessage::setSource(const QString &source)
{
    LOG("QtTsMessage::setSource", source);
    m_message.child("source").set_value(source.toLatin1().constData());
    qobject_cast<QtTsDocument *>(parent())->setHasChanged(true);
    Q_EMIT sourceChanged();
}

QString QtTsMessage::translation() const
{
    return QString::fromUtf8(m_message.child("translation").text().as_string());
}

void QtTsMessage::setTranslation(const QString &translate)
{
    LOG("QtTsMessage::setTranslation", translate);
    m_message.child("translation").set_value(translate.toUtf8().constData());
    qobject_cast<QtTsDocument *>(parent())->setHasChanged(true);
    Q_EMIT translationChanged();
}

QString QtTsMessage::context() const
{
    return m_context;
}

} // namespace Core
