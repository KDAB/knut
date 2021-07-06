#pragma once

#include <QJSValue>
#include <QObject>

class QQmlEngine;

namespace Core {

class UserDialog : public QObject
{
    Q_OBJECT
public:
    explicit UserDialog(QQmlEngine *parent = nullptr);

public slots:
    QJSValue getOpenFileName(const QString &caption, const QString &dir = QString(),
                             const QString &filters = QString());
    QJSValue getSaveFileName(const QString &caption, const QString &dir = QString(),
                             const QString &filters = QString());
    QJSValue getExistingDirectory(const QString &caption, const QString &dir = QString());

    QJSValue getItem(const QString &title, const QString &label, const QStringList &items, int current = 0,
                     bool editable = false);
    QJSValue getDouble(const QString &title, const QString &label, double value = 0, int decimals = 1, double step = 1,
                       double min = -2147483647, double max = 2147483647);
    QJSValue getInt(const QString &title, const QString &label, int value = 0, int step = 1, int min = -2147483647,
                    int max = 2147483647);
    QJSValue getText(const QString &title, const QString &label, const QString &text = QString());

    void information(const QString &title, const QString &text);
    void warning(const QString &title, const QString &text);
    void critical(const QString &title, const QString &text);

private:
    QWidget *dialogParent() const;
};

} // namespace Core
