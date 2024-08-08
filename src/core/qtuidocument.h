/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "document.h"

#include <pugixml.hpp>

namespace Utils {
class QtUiWriter;
}

namespace Core {

class QtUiDocument;

class QtUiWidget : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString className READ className WRITE setClassName NOTIFY classNameChanged)
    Q_PROPERTY(bool isRoot READ isRoot CONSTANT)

public:
    QtUiWidget(pugi::xml_node widget, bool isRoot = false, QObject *parent = nullptr);

    QString name() const;

    QString className() const;

    bool isRoot() const { return m_isRoot; }

    Q_INVOKABLE QVariant getProperty(const QString &name) const;
    Q_INVOKABLE void addProperty(const QString &name, const QVariant &value,
                                 const QHash<QString, QString> &attributes = {}, bool userProperty = false);

public slots:
    void setName(const QString &newName);
    void setClassName(const QString &newClassName);

signals:
    void nameChanged(const QString &newName);
    void classNameChanged(const QString &newClassName);

protected:
    friend QtUiDocument;
    pugi::xml_node xmlNode() const;

private:
    pugi::xml_node m_widget;
    bool m_isRoot = false;
};

class QtUiDocument : public Document
{
    Q_OBJECT
    Q_PROPERTY(QList<Core::QtUiWidget *> widgets READ widgets NOTIFY widgetsChanged)

public:
    explicit QtUiDocument(QObject *parent = nullptr);
    ~QtUiDocument() override;

    QList<Core::QtUiWidget *> widgets() const { return m_widgets; }
    Q_INVOKABLE Core::QtUiWidget *findWidget(const QString &name) const;

    Q_INVOKABLE Core::QtUiWidget *addWidget(const QString &className, const QString &name,
                                            Core::QtUiWidget *parent = nullptr);
    Q_INVOKABLE void addCustomWidget(const QString &className, const QString &baseClassName, const QString &header,
                                     bool isContainer = false);

public slots:
    void preview() const;

signals:
    void widgetsChanged();

protected:
    bool doSave(const QString &fileName) override;
    bool doLoad(const QString &fileName) override;

private:
    Utils::QtUiWriter *uiWriter();

    friend QtUiWidget;
    pugi::xml_document m_document;
    std::unique_ptr<Utils::QtUiWriter> m_writer;
    QList<QtUiWidget *> m_widgets;
};

} // namespace Core
