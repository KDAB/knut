#include "logger.h"

#include <spdlog/spdlog.h>

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
            for (const auto &param : params)
                paramStrings.push_back(param.toString());
            return paramStrings.join(", ");
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
    for (int i = 0; i < data.params.size(); ++i) {
        const auto &param = data.params[i];
        auto &lastParam = lastData.params[i];
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        switch (static_cast<QMetaType::Type>(param.type())) {
#else
        switch (static_cast<QMetaType::Type>(param.typeId())) {
#endif
        case QMetaType::Int:
            lastParam = lastParam.toInt() + param.toInt();
            break;
        case QMetaType::QString:
            lastParam = lastParam.toString() + param.toString();
            break;
        case QMetaType::QStringList:
            lastParam = lastParam.toStringList() + param.toStringList();
            break;
        default:
            Q_UNREACHABLE();
        }
    }
    auto lastIndex = index(static_cast<int>(m_data.size()) - 1, ParamCol);
    emit dataChanged(lastIndex, lastIndex);
}

LoggerDisabler::LoggerDisabler()
{
    LoggerObject::m_canLog = false;
}

LoggerDisabler::~LoggerDisabler()
{
    LoggerObject::m_canLog = true;
}

} // namespace Core
