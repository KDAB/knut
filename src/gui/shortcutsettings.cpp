#include "shortcutsettings.h"
#include "ui_shortcutsettings.h"

#include "mainwindow.h"
#include "shortcutmanager.h"

#include <QAbstractTableModel>
#include <QKeySequence>
#include <QSortFilterProxyModel>

#include <algorithm>
#include <vector>

namespace Gui {

//=============================================================================
// Model listing all "shortcutable" objects
//=============================================================================
class ShortcutModel : public QAbstractItemModel
{
public:
    enum Columns { IdCol = 0, ShortcutCol, DescriptionCol, ColumnsCount };

    enum Roles {
        ShortcutRole = Qt::UserRole,
        IdRole,
    };

    using QAbstractItemModel::QAbstractItemModel;

    void initialize(ShortcutManager *shortcutManager)
    {
        Q_ASSERT(shortcutManager);
        m_shortcutManager = shortcutManager;
        beginResetModel();
        const auto &shortcuts = shortcutManager->shortcuts();
        auto it = std::ranges::find_if(shortcuts, [](const auto &shortcut) {
            return !shortcut.isAction;
        });
        m_shortcuts[0] = std::vector<ShortcutManager::Shortcut>(shortcuts.cbegin(), it);
        m_shortcuts[1] = std::vector<ShortcutManager::Shortcut>(it, shortcuts.cend());
        endResetModel();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (!parent.isValid())
            return 2;
        else if (static_cast<int>(parent.internalId()) == -1)
            return static_cast<int>(m_shortcuts[parent.row()].size());
        return {};
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return ColumnsCount;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

        if (index.parent().isValid()) {
            const auto &shortcut = m_shortcuts[index.internalId()].at(index.row());
            switch (role) {
            case Qt::DisplayRole:
                switch (index.column()) {
                case IdCol:
                    return shortcut.id;
                case ShortcutCol:
                    return shortcut.shortcut;
                case DescriptionCol:
                    return shortcut.description;
                }
                break;
            case ShortcutRole:
                return shortcut.shortcut;
            case IdRole:
                return shortcut.id;
            }
        } else {
            if (role == Qt::DisplayRole && index.column() == IdCol)
                return index.row() == 0 ? tr("Actions") : tr("Scripts");
            if (role == Qt::FontRole) {
                QFont font;
                font.setBold(true);
                return QVariant::fromValue(font);
            }
        }
        return {};
    }

    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) override
    {
        if (!idx.isValid() || role != Qt::EditRole)
            return false;

        const auto newShortcut = value.value<QKeySequence>();
        m_shortcutManager->setShortcut(idx.data(IdRole).toString(), newShortcut);
        shortcutForIndex(idx).shortcut = newShortcut.toString(QKeySequence::NativeText);
        emit dataChanged(createIndex(idx.row(), ShortcutCol, idx.internalId()),
                         createIndex(idx.row(), ShortcutCol, idx.internalId()));
        return true;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    {
        if (orientation == Qt::Vertical || role != Qt::DisplayRole)
            return {};

        switch (section) {
        case IdCol:
            return tr("Name");
        case ShortcutCol:
            return tr("Shortcut");
        case DescriptionCol:
            return tr("Description");
        }
        return {};
    }

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid()) {
            return createIndex(row, column, static_cast<quintptr>(parent.row()));
        } else {
            return createIndex(row, column, static_cast<quintptr>(-1));
        }
    }

    QModelIndex parent(const QModelIndex &child) const override
    {
        if (!child.isValid())
            return {};
        const auto id = child.internalId();
        if (id == 0)
            return createIndex(0, 0, static_cast<quintptr>(-1));
        else if (id == 1)
            return createIndex(1, 0, static_cast<quintptr>(-1));
        else
            return {};
    }

    void resetAll()
    {
        m_shortcutManager->resetAll();
        const auto &shortcuts = m_shortcutManager->shortcuts();
        for (int i = 0; i < static_cast<int>(shortcuts.size()); ++i) {
            const int index = i < static_cast<int>(m_shortcuts[0].size()) ? 0 : 1;
            const int row = index == 0 ? i : i - static_cast<int>(m_shortcuts[0].size());
            m_shortcuts[index][row].shortcut = shortcuts[i].shortcut;
        }
        emit dataChanged(
            createIndex(0, ShortcutCol, static_cast<quintptr>(0)),
            createIndex(static_cast<int>(m_shortcuts[0].size()) - 1, ShortcutCol, static_cast<quintptr>(0)));
        emit dataChanged(
            createIndex(0, ShortcutCol, static_cast<quintptr>(1)),
            createIndex(static_cast<int>(m_shortcuts[0].size()) - 1, ShortcutCol, static_cast<quintptr>(1)));
    }
    void resetOne(const QModelIndex &idx)
    {
        auto newShortcut = m_shortcutManager->resetShortcut(idx.data(IdRole).toString());
        shortcutForIndex(idx).shortcut = newShortcut.toString(QKeySequence::NativeText);
        emit dataChanged(createIndex(idx.row(), ShortcutCol, idx.internalId()),
                         createIndex(idx.row(), ShortcutCol, idx.internalId()));
    }

private:
    ShortcutManager::Shortcut &shortcutForIndex(const QModelIndex &index)
    {
        Q_ASSERT(index.parent().isValid());
        return m_shortcuts[index.internalId()][index.row()];
    }
    std::vector<ShortcutManager::Shortcut> m_shortcuts[2];
    ShortcutManager *m_shortcutManager;
};

//=============================================================================
// ShortcutSettings
//=============================================================================
ShortcutSettings::ShortcutSettings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShortcutSettings)
    , m_shortcutModel(new ShortcutModel(this))
{
    ui->setupUi(this);

    m_shortcutModel->initialize(qobject_cast<MainWindow *>(parentWidget())->shortcutManager());

    auto proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(m_shortcutModel);
    proxy->setFilterKeyColumn(ShortcutModel::IdCol);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setRecursiveFilteringEnabled(true);
    ui->shortcutsView->setModel(proxy);
    ui->shortcutsView->expandAll();

    ui->shortcutsView->header()->setSectionResizeMode(ShortcutModel::IdCol, QHeaderView::ResizeToContents);
    ui->shortcutsView->header()->setSectionResizeMode(ShortcutModel::ShortcutCol, QHeaderView::ResizeToContents);
    ui->shortcutsView->header()->stretchLastSection();

    connect(ui->filterLineEdit, &QLineEdit::textChanged, proxy, &QSortFilterProxyModel::setFilterWildcard);
    auto sortByColumn = [proxy](int column) {
        if (column == proxy->sortColumn())
            proxy->sort(column, proxy->sortOrder() == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder);
        else
            proxy->sort(column);
    };
    connect(ui->shortcutsView->header(), &QHeaderView::sectionClicked, proxy, sortByColumn);
    proxy->sort(ShortcutModel::IdCol);

    connect(ui->shortcutsView->selectionModel(), &QItemSelectionModel::currentChanged, this,
            &ShortcutSettings::updateCurrentItem);
    connect(ui->resetAllButton, &QPushButton::clicked, this, &ShortcutSettings::resetAll);
    connect(ui->resetButton, &QPushButton::clicked, this, &ShortcutSettings::resetSelected);
    connect(ui->recordButton, &QPushButton::clicked, this, &ShortcutSettings::recordShortcut);
}

ShortcutSettings::~ShortcutSettings() = default;

void ShortcutSettings::updateCurrentItem()
{
    auto index = ui->shortcutsView->currentIndex();
    if (index.data(ShortcutModel::ShortcutRole).isValid()) {
        ui->shortcutBox->setEnabled(true);
        ui->resetButton->setEnabled(true);
        ui->keySequenceEdit->setKeySequence(index.data(ShortcutModel::ShortcutRole).value<QKeySequence>());
    } else {
        ui->shortcutBox->setEnabled(false);
        ui->resetButton->setEnabled(false);
        ui->keySequenceEdit->clear();
    }
}

void ShortcutSettings::resetAll()
{
    m_shortcutModel->resetAll();
    auto index = ui->shortcutsView->currentIndex();
    if (index.data(ShortcutModel::ShortcutRole).isValid())
        ui->keySequenceEdit->setKeySequence(index.data(ShortcutModel::ShortcutRole).value<QKeySequence>());
}

void ShortcutSettings::resetSelected()
{
    auto index = ui->shortcutsView->currentIndex();
    auto model = qobject_cast<QSortFilterProxyModel *>(ui->shortcutsView->model());

    m_shortcutModel->resetOne(model->mapToSource(index));
    ui->keySequenceEdit->setKeySequence(index.data(ShortcutModel::ShortcutRole).value<QKeySequence>());
}

void ShortcutSettings::recordShortcut()
{
    auto index = ui->shortcutsView->currentIndex();
    auto model = ui->shortcutsView->model();

    model->setData(index, ui->keySequenceEdit->keySequence());
}

} // namespace Gui
