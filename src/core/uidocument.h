#pragma once

#include "document.h"

#include <pugixml.hpp>

namespace Core {

class UiDocument;

class UiWidget : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString className READ className WRITE setClassName)
    Q_PROPERTY(bool isRoot READ isRoot CONSTANT)

public:
    UiWidget(pugi::xml_node widget, bool isRoot = false, QObject *parent = nullptr);

    QString name() const;
    void setName(const QString &newName);

    QString className() const;
    void setClassName(const QString &newClassName);

    bool isRoot() const { return m_isRoot; }

private:
    friend UiDocument;
    pugi::xml_node m_widget;
    bool m_isRoot = false;
};

class UiDocument : public Document
{
    Q_OBJECT
    Q_PROPERTY(QVector<Core::UiWidget *> widgets READ widgets NOTIFY fileNameChanged)

public:
    explicit UiDocument(QObject *parent = nullptr);

    QVector<Core::UiWidget *> widgets() const { return m_widgets; }
    Q_INVOKABLE Core::UiWidget *findWidget(const QString &name) const;

public slots:
    void preview();

protected:
    bool doSave(const QString &fileName) override;
    bool doLoad(const QString &fileName) override;

private:
    friend UiWidget;
    pugi::xml_document m_document;
    QVector<UiWidget *> m_widgets;
};

} // namespace Core
