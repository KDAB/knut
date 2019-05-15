#include "dialogmodel.h"

#include "global.h"

#include <QColor>

DialogModel::DialogModel(const Data::Dialog &dialog, QObject *parent)
    : QAbstractTableModel(parent)
    , m_controls(dialog.controls)
{
}

int DialogModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_controls.size();
}

int DialogModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 6;
}

QVariant DialogModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        const auto &control = m_controls.at(index.row());
        switch (index.column()) {
        case Type:
            return control.type;
        case ID:
            return control.id;
        case Geometry:
            return QStringLiteral("(%1, %2 %3x%4)")
                .arg(control.geometry.x())
                .arg(control.geometry.y())
                .arg(control.geometry.width())
                .arg(control.geometry.height());
        case Text:
            return control.text;
        case ClassName:
            return control.className;
        case Styles:
            return control.styles.join(QLatin1Char(','));
        }
    }

    if (role == Qt::ForegroundRole) {
        const auto &control = m_controls.at(index.row());
        if (control.id.isEmpty())
            return QVariant::fromValue(QColor(Qt::red));
    }

    if (role == Knut::LineRole) {
        const auto &control = m_controls.at(index.row());
        return control.line;
    }

    return {};
}

QVariant DialogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QStringList headers = {"Type", "Id", "Geometry", "Text", "Class Name", "Styles"};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return headers.value(section);
    }
    return {};
}
