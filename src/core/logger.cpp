/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "logger.h"
#include "scriptrunner.h"
#include "settings.h"
#include "textdocument_p.h"

#include <QHash>

namespace Core {

LoggerObject::LoggerObject()
    : m_firstLogger(m_canLog)
{
}

LoggerObject::~LoggerObject()
{
    if (m_firstLogger)
        m_canLog = true;
}

void LoggerObject::log(QString &&string)
{
    spdlog::trace(string);
    m_canLog = false;
}

HistoryModel::HistoryModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    LoggerObject::m_model = this;
}

HistoryModel::~HistoryModel()
{
    LoggerObject::m_model = nullptr;
}

int Core::HistoryModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(m_data.size());
}

int HistoryModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case NameCol:
            return m_data.at(index.row()).name;
        case ParamCol: {
            const auto &params = m_data.at(index.row()).params;
            QStringList paramStrings;
            for (const auto &param : params) {
                QString text = param.value;
                if (!param.name.isEmpty())
                    text.prepend(QString("%1: ").arg(param.name));
                paramStrings.push_back(text);
            }
            const QString returnVariable = m_data.at(index.row()).returnArg.name;
            return paramStrings.join(", ") + (returnVariable.isEmpty() ? "" : (" => " + returnVariable));
        }
        }
    }
    return {};
}

QVariant HistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical || role != Qt::DisplayRole)
        return {};

    switch (section) {
    case NameCol:
        return tr("API name");
    case ParamCol:
        return tr("Parameters");
    }
    return {};
}

void HistoryModel::clear()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
}

QString HistoryModel::createScript(int start, int end)
{
    const auto settings = Core::Settings::instance()->value<Core::TabSettings>(Core::Settings::Tab);
    const auto tab = settings.insertSpaces ? QString(settings.tabSize, ' ') : QString('\t');

    std::tie(start, end) = std::minmax(start, end);
    Q_ASSERT(start >= 0 && start <= end && end < static_cast<int>(m_data.size()));

    QString scriptText = "// Description of the script\n\nfunction main() {\n";

    QHash<QString, QVariant> returnVariables;

    for (int row = start; row <= end; ++row) {
        const auto &data = m_data.at(row);
        QString apiCall = data.name;
        const bool isProperty = ScriptRunner::isProperty(apiCall);

        // Check if we need to create the document, and change the API call as it's not a singleton
        if (data.name.contains("Document::")) {
            if (!returnVariables.contains("document"))
                scriptText += tab + "var document = Project.currentDocument\n";
            returnVariables["document"] = {};
            apiCall = "document." + apiCall.mid(apiCall.indexOf("::") + 2);
        } else {
            apiCall.replace("::", ".");
        }

        // Set the return value
        QString returnValue;
        if (!data.returnArg.isEmpty()) {
            const auto &name = data.returnArg.name;
            returnValue = (returnVariables.contains(name) ? "" : "var ") + name + " = ";
            returnVariables[name] = data.returnArg.value;
        }

        // Pass the parameters
        QStringList paramStrings;
        for (const auto &param : data.params) {
            if (!param.name.isEmpty() && returnVariables.value(param.name) == param.value) {
                paramStrings.append(param.name);
                continue;
            }

            paramStrings.push_back(param.value);
        }

        if (isProperty) {
            if (paramStrings.isEmpty())
                scriptText += tab + returnValue + apiCall + '\n';
            else
                scriptText += tab + returnValue + QString("%1 = %2\n").arg(apiCall, paramStrings.first());
        } else {
            scriptText += tab + returnValue + QString("%1(%2)\n").arg(apiCall, paramStrings.join(", "));
        }
    }

    scriptText += "}\n";
    return scriptText;
}

QString HistoryModel::createScript(const QModelIndex &startIndex, const QModelIndex &endIndex)
{
    Q_ASSERT(checkIndex(startIndex, CheckIndexOption::IndexIsValid)
             && checkIndex(endIndex, CheckIndexOption::IndexIsValid));
    return createScript(startIndex.row(), endIndex.row());
}

void HistoryModel::logData(const QString &name)
{
    addData(LogData {name, {}, {}}, false);
}

void HistoryModel::addData(LogData &&data, bool merge)
{
    if (!merge || m_data.empty() || m_data.back().name != data.name) {
        beginInsertRows({}, static_cast<int>(m_data.size()), static_cast<int>(m_data.size()));
        m_data.push_back(std::move(data));
        endInsertRows();
        return;
    }

    auto &lastData = m_data.back();
    // Add parameters together
    for (size_t i = 0; i < data.params.size(); ++i) {
        const auto &param = data.params[i];
        auto &lastParam = lastData.params[i];
        switch (static_cast<QMetaType::Type>(param.type)) {
        case QMetaType::Int:
            lastParam.value = QString::number(lastParam.value.toInt() + param.value.toInt());
            break;
        case QMetaType::QString:
            lastParam.value = lastParam.value.chopped(1) + param.value.sliced(1);
            break;
        case QMetaType::QStringList:
            lastParam.value = lastParam.value.chopped(1) + ", " + param.value.sliced(1);
            break;
        default:
            Q_UNREACHABLE();
        }
    }
    auto lastIndex = index(static_cast<int>(m_data.size()) - 1, ParamCol);
    emit dataChanged(lastIndex, lastIndex);
}

LoggerDisabler::LoggerDisabler(bool silenceAll)
    : m_originalCanLog(LoggerObject::m_canLog)
    , m_silenceAll(silenceAll)
{
    LoggerObject::m_canLog = false;
    if (m_silenceAll) {
        m_level = spdlog::default_logger()->level();
        spdlog::default_logger()->set_level(spdlog::level::off);
    }
}

LoggerDisabler::~LoggerDisabler()
{
    LoggerObject::m_canLog = m_originalCanLog;
    if (m_silenceAll)
        spdlog::default_logger()->set_level(m_level);
}

} // namespace Core
