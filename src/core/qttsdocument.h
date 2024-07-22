/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "textdocument.h"

#include <pugixml.hpp>

namespace Core {
class QtTsDocument;

class QtTsMessage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int line READ line WRITE setLine NOTIFY lineChanged)
    Q_PROPERTY(QString translation READ translation WRITE setTranslation NOTIFY translationChanged)
    Q_PROPERTY(QString comment READ comment WRITE setComment NOTIFY commentChanged)
    Q_PROPERTY(QString context READ context CONSTANT)
public:
    explicit QtTsMessage(QString context, pugi::xml_node message, QObject *parent = nullptr);

    QString fileName() const;
    void setFileName(const QString &name);

    int line() const;
    void setLine(int line);

    QString source() const;
    void setSource(const QString &source);

    QString translation() const;
    void setTranslation(const QString &translate);

    QString comment() const;
    void setComment(const QString &comment);

    QString context() const;

signals:
    void fileNameChanged();
    void lineChanged();
    void sourceChanged();
    void translationChanged();
    void commentChanged();
    void fileUpdated();

private:
    friend QtTsDocument;
    QString m_context;
    pugi::xml_node m_message;
};

class QtTsDocument : public Document
{
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString sourceLanguage READ sourceLanguage WRITE setSourceLanguage NOTIFY sourceLanguageChanged)
    Q_PROPERTY(QList<Core::QtTsMessage *> messages READ messages NOTIFY messagesChanged)
public:
    explicit QtTsDocument(QObject *parent = nullptr);

    Q_INVOKABLE void setSourceLanguage(const QString &lang);
    Q_INVOKABLE void setLanguage(const QString &lang);
    Q_INVOKABLE void addMessage(const QString &context, const QString &fileName, const QString &source,
                                const QString &translation, const QString &comment = QString());
    Q_INVOKABLE void setMessageContext(const QString &context, const QString &comment, const QString &source, const QString &newContext);
    QString language() const;
    QString sourceLanguage() const;
    QList<QtTsMessage *> messages() const;

protected:
    bool doSave(const QString &fileName) override;
    bool doLoad(const QString &fileName) override;

signals:
    void languageChanged();
    void sourceLanguageChanged();
    void messagesChanged();

private:
    friend QtTsMessage;
    void addMessage(pugi::xml_node contextChild, const QString &context, const QString &location, const QString &source,
                    const QString &translation, const QString &comment = QString());
    void initializeXml();
    pugi::xml_document m_document;

    pugi::xml_node findContext(const QString &context) const;
    pugi::xml_node findOrCreateContext(const QString &context);

    QList<QtTsMessage *> m_messages;
};

} // namespace Core
