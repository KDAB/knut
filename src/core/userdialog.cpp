/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "userdialog.h"
#include "logger.h"

#include <QApplication>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QQmlEngine>

namespace Core {

/*!
 * \qmltype UserDialog
 * \brief Singleton with methods to display common dialog to the user.
 * \inqmlmodule Script
 * \ingroup Utilities
 *
 * The `UserDialog` singleton provides methods to display common dialog that could be used in
 * scripts. If the user cancel the dialog, it will return a null value you can test directly:
 *
 * ```js
 * let s = UserDialog.getText("Get text", "Value")
 * if (s !== null)
 *     Message.log("Text: " + s)
 * else
 *     Message.log("Cancelled")
 * ```
 */

UserDialog::UserDialog(QQmlEngine *parent)
    : QObject(parent)
{
}

/*!
 * \qmlmethod string UserDialog::getOpenFileName(string caption, string dir = "", string filters = "")
 * Returns an existing file selected by the user. Returns `null` if the user cancel.
 *
 * The dialog's caption is set to `caption`, the initial directory set to `dir` and only files
 * that matches the given `filters` are shown.
 */
QJSValue UserDialog::getOpenFileName(const QString &caption, const QString &dir, const QString &filters)
{
    LOG("UserDialog::getOpenFileName", caption, dir, filters);
    const QString s = QFileDialog::getOpenFileName(dialogParent(), caption, dir, filters);
    if (!s.isEmpty()) {
        spdlog::debug("UserDialog::getOpenFileName returns {}", s);
        return s;
    }
    return QJSValue(QJSValue::NullValue);
}

/*!
 * \qmlmethod string UserDialog::getSaveFileName(string caption, string dir = "", string filters = "")
 * Returns a file name selected by the user. Returns `null` if the user cancel.
 *
 * The dialog's caption is set to `caption`, the initial directory set to `dir` and only files
 * that matches the given `filters` are shown.
 */
QJSValue UserDialog::getSaveFileName(const QString &caption, const QString &dir, const QString &filters)
{
    LOG("UserDialog::getSaveFileName", caption, dir, filters);
    const QString s = QFileDialog::getSaveFileName(dialogParent(), caption, dir, filters);
    if (!s.isEmpty()) {
        spdlog::debug("UserDialog::getSaveFileName returns {}", s);
        return s;
    }
    return QJSValue(QJSValue::NullValue);
}

/*!
 * \qmlmethod string UserDialog::getExistingDirectory(string caption, string dir = "")
 * Returns an existing directory selected by the user. Returns `null` if the user cancel.
 *
 * The dialog's caption is set to `caption`, the initial directory set to `dir`.
 */
QJSValue UserDialog::getExistingDirectory(const QString &caption, const QString &dir)
{
    LOG("UserDialog::getExistingDirectory", caption, dir);
    const QString s = QFileDialog::getExistingDirectory(dialogParent(), caption, dir);
    if (!s.isEmpty()) {
        spdlog::debug("UserDialog::getExistingDirectory returns {}", s);
        return s;
    }
    return QJSValue(QJSValue::NullValue);
}

// clang-format off
/*!
 * \qmlmethod string UserDialog::getItem(string title, string label, array<string> items, int current = 0, bool editable = false)
 * Returns a string selected among a pre-defined list. Returns `null` if the user cancel.
 *
 * The dialog's caption is set to `title`, the `label` is displayed and `items` contain the
 * initial list of values. The value `current` is selected at the start, and if `editable` is
 * true, the user will be able to enter his own value.
 */
// clang-format on
QJSValue UserDialog::getItem(const QString &title, const QString &label, const QStringList &items, int current,
                             bool editable)
{
    LOG("UserDialog::getItem", title, label, items, current, editable);
    bool ok;
    const QString ret = QInputDialog::getItem(dialogParent(), title, label, items, current, editable, &ok);
    if (ok) {
        spdlog::debug("UserDialog::getItem returns {}", ret);
        return ret;
    }
    return QJSValue(QJSValue::NullValue);
}

// clang-format off
/*!
 * \qmlmethod double UserDialog::getDouble(string title, string label, real value = 0.0, int decimals = 1, real step = 1.0, real min = -2147483647.0, real max = 2147483647.0)
 * Returns a double entered by the user. Returns `null` if the user cancel.
 *
 * The dialog's caption is set to `title`, the `label` is displayed and the initial value is
 * `value`. The dialog will show `decimals` number of decimals.
 * `min` and `max` are the minimum and maximum values the user may choose.
 * `step` is the amount by which the values change when incrementing or decrementing the value.
 */
// clang-format on
QJSValue UserDialog::getDouble(const QString &title, const QString &label, double value, int decimals, double step,
                               double min, double max)
{
    LOG("UserDialog::getDouble", title, label, value, decimals, step, min, max);
    bool ok;
    const double ret =
        QInputDialog::getDouble(dialogParent(), title, label, value, min, max, decimals, &ok, Qt::WindowFlags(), step);
    if (ok) {
        spdlog::debug("UserDialog::getDouble returns {}", ret);
        return ret;
    }
    return QJSValue(QJSValue::NullValue);
}

// clang-format off
/*!
 * \qmlmethod int UserDialog::getInt(string title, string label, int value = 0, int step = 1, int min = -2147483647, int max = 2147483647)
 * Returns an integer entered by the user. Returns `null` if the user cancel.
 *
 * The dialog's caption is set to `title`, the `label` is displayed and the initial value is
 * `value`. `min` and `max` are the minimum and maximum values the user may choose.
 * `step` is the amount by which the values change when incrementing or decrementing the value.
 */
// clang-format on
QJSValue UserDialog::getInt(const QString &title, const QString &label, int value, int step, int min, int max)
{
    LOG("UserDialog::getInt", title, label, value, step, min, max);
    bool ok;
    const int ret = QInputDialog::getInt(dialogParent(), title, label, value, min, max, step, &ok);
    if (ok) {
        spdlog::debug("UserDialog::getInt returns {}", ret);
        return ret;
    }
    return QJSValue(QJSValue::NullValue);
}

/*!
 * \qmlmethod string UserDialog::getText(string title, string label, string text = "")
 * Returns a string entered by the user. Returns `null` if the user cancel.
 *
 * The dialog's caption is set to `title`, the `label` is displayed and the initial value is
 * `text`.
 */
QJSValue UserDialog::getText(const QString &title, const QString &label, const QString &text)
{
    LOG("UserDialog::getText", title, label, text);
    bool ok;
    const QString ret = QInputDialog::getText(dialogParent(), title, label, QLineEdit::Normal, text, &ok);
    if (ok) {
        spdlog::debug("UserDialog::getText returns {}", ret);
        return ret;
    }
    return QJSValue(QJSValue::NullValue);
}

/*!
 * \qmlmethod UserDialog::information(string title, string text )
 * Show an information dialog.
 */
void UserDialog::information(const QString &title, const QString &text)
{
    LOG("UserDialog::information", title, text);
    QMessageBox::information(dialogParent(), title, text);
}

/*!
 * \qmlmethod UserDialog::warning(string title, string text )
 * Show a warning dialog.
 */
void UserDialog::warning(const QString &title, const QString &text)
{
    LOG("UserDialog::warning", title, text);
    QMessageBox::warning(dialogParent(), title, text);
}

/*!
 * \qmlmethod UserDialog::critical(string title, string text )
 * Show a critical dialog.
 */
void UserDialog::critical(const QString &title, const QString &text)
{
    LOG("UserDialog::critical", title, text);
    QMessageBox::critical(dialogParent(), title, text);
}

QWidget *UserDialog::dialogParent() const
{
    return parent() && parent()->property("scriptWindow").toBool() ? nullptr : QApplication::activeWindow();
}

} // namespace Core
