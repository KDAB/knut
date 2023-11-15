#pragma once

#include <QObject>
#include <QXmlStreamWriter>

#include <QHash>
#include <QStack>
#include <QVector>

namespace RcCore {

class UiWriter : public QObject
{
    Q_OBJECT

public:
    UiWriter(QIODevice *device, QObject *parent = nullptr);
    ~UiWriter() override;

public slots:
    void setClassName(const QString &className);

    void addCustomWidget(const QString &className, const QString &baseClassName, const QString &header,
                         bool isContainer = false);

    void startWidget(const QString &className, const QString &name, const QRect &geometry);
    void endWidget();

    void addProperty(const QString &name, const QVariant &value);

private:
    QXmlStreamWriter m_writer;

    struct CustomWidget
    {
        QString className;
        QString baseClassName;
        QString header;
        bool isGlobal = false;
        bool isContainer = false;
    };
    QHash<QString, CustomWidget> m_customWidgets;
    QHash<QString, int> m_widgetName;

    QStack<bool> m_isMainWindow;
    QString m_currentId;
};

} // namespace RcCore
