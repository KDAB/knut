#include "logger.h"

#include <spdlog/spdlog.h>

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
    spdlog::trace(string.toStdString());
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
                QString text = param.value.toString();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                if (static_cast<QMetaType::Type>(param.value.type()) == QMetaType::QString) {
#else
                if (static_cast<QMetaType::Type>(param.value.typeId()) == QMetaType::QString) {
#endif
                    text.replace('\n', "\\n");
                    text.replace('\t', "\\t");
                    text.append('"');
                    text.prepend('"');
                }
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
    std::tie(start, end) = std::minmax(start, end);
    Q_ASSERT(start > 0 && start <= end && end < static_cast<int>(m_data.size()));

    QString scriptText = "// Description of the script\nfunction main() {\n";

    QHash<QString, QVariant> returnVariables;

    for (int row = start; row <= end; ++row) {
        const auto &data = m_data.at(row);
        auto apiCall = data.name;

        // Set the return value
        if (!data.returnArg.isEmpty()) {
            const auto &name = data.returnArg.name;
            scriptText += (returnVariables.contains(name) ? "" : "var ") + name + " = ";
            returnVariables[name] = data.returnArg.value;
        }

        // Check if we need to create the document, and change the API call as it's not a singleton
        if (data.name.contains("Document::")) {
            if (!returnVariables.contains("document"))
                scriptText += "var document = Project.currentDocument\n";
            returnVariables["document"] = {};
            apiCall = "document." + apiCall.mid(apiCall.indexOf("::") + 2);
        }
        scriptText += apiCall;

        // Pass the parameters
        QStringList paramStrings;
        for (const auto &param : data.params) {
            if (!param.name.isEmpty() && returnVariables.value(param.name) == param.value) {
                paramStrings.append(param.name);
                continue;
            }

            QString text = param.value.toString();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            if (static_cast<QMetaType::Type>(param.value.type()) == QMetaType::QString) {
#else
            if (static_cast<QMetaType::Type>(param.value.typeId()) == QMetaType::QString) {
#endif
                text.replace('\n', "\\n");
                text.replace('\t', "\\t");
                text.append('"');
                text.prepend('"');
            }
            paramStrings.push_back(text);
        }

        scriptText.append(QString("(%1)\n").arg(paramStrings.join(", ")));
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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        switch (static_cast<QMetaType::Type>(param.value.type())) {
#else
        switch (static_cast<QMetaType::Type>(param.value.typeId())) {
#endif
        case QMetaType::Int:
            lastParam.value = lastParam.value.toInt() + param.value.toInt();
            break;
        case QMetaType::QString:
            lastParam.value = lastParam.value.toString() + param.value.toString();
            break;
        case QMetaType::QStringList:
            lastParam.value = lastParam.value.toStringList() + param.value.toStringList();
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
