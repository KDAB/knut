#include "palette.h"
#include "ui_palette.h"

#include "gui_constants.h"
#include "mainwindow.h"

#include "core/logger.h"
#include "core/lspdocument.h"
#include "core/project.h"
#include "core/scriptmanager.h"
#include "core/symbol.h"
#include "core/textdocument.h"
#include "core/utils.h"

#include <QAbstractTableModel>
#include <QAction>
#include <QDirIterator>
#include <QFileInfo>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QSortFilterProxyModel>
#include <QTimer>

#include <algorithm>

namespace Gui {

//=============================================================================
// Model listing all files in the project
//=============================================================================
class FileModel : public QAbstractTableModel
{
public:
    using QAbstractTableModel::QAbstractTableModel;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        return static_cast<int>(m_files.size());
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        return 2;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

        switch (role) {
        case Qt::DisplayRole:
            if (index.column() == 0 && role == Qt::DisplayRole) {
                return m_files.at(index.row()).fileName;
            } else if (index.column() == 1) {
                QDir dir(Core::Project::instance()->root());
                return dir.relativeFilePath(m_files.at(index.row()).path);
            }
            break;
        case Qt::ToolTipRole:
        case Qt::UserRole:
            return m_files.at(index.row()).path;
        }

        return {};
    }

    void resetFileInfo()
    {
        if (Core::Project::instance()->root().isEmpty())
            return;

        beginResetModel();

        m_files.clear();
        QDirIterator it(Core::Project::instance()->root(), QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            auto fi = it.fileInfo();
            if (fi.isFile())
                m_files.push_back({fi.fileName(), fi.absoluteFilePath()});
        }
        auto byFileName = [](const auto &fi1, const auto &fi2) {
            return fi1.fileName < fi2.fileName;
        };
        std::ranges::sort(m_files, byFileName);

        endResetModel();
    }

private:
    struct FileInfo
    {
        QString fileName;
        QString path;
    };
    std::vector<FileInfo> m_files;
};

//=============================================================================
// Model listing all scripts
//=============================================================================
class ScriptModel : public QAbstractTableModel
{
public:
    using QAbstractTableModel::QAbstractTableModel;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        const auto &list = Core::ScriptManager::instance()->scriptList();
        return static_cast<int>(list.size());
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        return 2;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

        const auto &list = Core::ScriptManager::instance()->scriptList();

        switch (role) {
        case Qt::DisplayRole:
            if (index.column() == 0 && role == Qt::DisplayRole) {
                return list.at(index.row()).name;
            } else if (index.column() == 1) {
                return list.at(index.row()).description;
            }
            break;
        case Qt::ToolTipRole:
        case Qt::UserRole:
            return list.at(index.row()).fileName;
        }

        return {};
    }
};

//=============================================================================
// Model listing all symbols
//=============================================================================
class SymbolModel : public QAbstractTableModel
{
public:
    using QAbstractTableModel::QAbstractTableModel;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        return m_symbols.count();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        return 2;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            if (index.column() == 0 && role == Qt::DisplayRole) {
                return m_symbols.at(index.row())->name();
            } else if (index.column() == 1) {
                return m_symbols.at(index.row())->description();
            }
            break;
        case Qt::UserRole:
            return m_symbols.at(index.row())->name();
        }
        return {};
    }

    void resetSymbols()
    {
        Core::LoggerDisabler ld;
        beginResetModel();
        m_symbols.clear();
        if (auto lspDocument = qobject_cast<Core::LspDocument *>(Core::Project::instance()->currentDocument()))
            m_symbols = lspDocument->symbols();
        endResetModel();
    }

private:
    QVector<Core::Symbol *> m_symbols;
};

//=============================================================================
// Model listing all actions
//=============================================================================
class ActionModel : public QAbstractTableModel
{
public:
    using QAbstractTableModel::QAbstractTableModel;

    void setActions(const QList<QAction *> &actions)
    {
        beginResetModel();
        m_actions = actions;
        endResetModel();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        return m_actions.count();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        return 2;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

        switch (role) {
        case Qt::DisplayRole:
            if (index.column() == 0)
                return m_actions.at(index.row())->text().remove('&');
            else
                return m_actions.at(index.row())->shortcut().toString(QKeySequence::NativeText);
        case Qt::ToolTipRole:
            return m_actions.at(index.row())->text();
        case Qt::UserRole:
            return QVariant::fromValue(m_actions.at(index.row()));
        }
        return {};
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

        if (!m_actions.at(index.row())->isEnabled())
            return {};
        return QAbstractTableModel::flags(index);
    }

private:
    QList<QAction *> m_actions;
};

//=============================================================================
// Proxy model used to sort and display the data
//=============================================================================
class SortFilterProxyModel : public QSortFilterProxyModel
{
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        switch (role) {
        case Qt::ForegroundRole:
            if (index.column() != 0)
                return QVariant::fromValue(QBrush(Qt::gray));
            break;
        case Qt::FontRole:
            if (index.column() != 0) {
                auto font = QApplication::font();
                font.setItalic(true);
                return font;
            }
            break;
        }

        return QSortFilterProxyModel::data(index, role);
    }
};

//=============================================================================
// Palette
//=============================================================================

Palette::Palette(QMainWindow *parent)
    : QFrame(parent, Qt::Popup)
    , ui(new Ui::Palette)
{
    ui->setupUi(this);
    setFixedWidth(Constants::PaletteWidth);

    m_proxyModel = new SortFilterProxyModel(this);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->treeView->setModel(m_proxyModel);

    ui->treeView->setStyleSheet("QTreeView::item { padding: 5px }");
    ui->treeView->setRootIsDecorated(false);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &Palette::changeText);
    connect(ui->treeView, &QTreeView::clicked, this, &Palette::clickItem);
    ui->lineEdit->installEventFilter(this);

    auto init = [this]() {
        addFileSelector();
        addLineSelector();
        addScriptSelector();
        addSymbolSelector();
        addActionSelector();
    };
    QTimer::singleShot(0, init);
}

Palette::~Palette() = default;

bool Palette::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    if (event->type() == QEvent::KeyPress) {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Up) {
            int row = ui->treeView->currentIndex().row();
            do {
                row = std::max(row - 1, 0);
                auto index = ui->treeView->model()->index(row, 0);
                if (ui->treeView->model()->flags(index) & Qt::ItemFlag::ItemIsEnabled) {
                    ui->treeView->setCurrentIndex(index);
                    break;
                }
            } while (row != 0);
        } else if (keyEvent->key() == Qt::Key_Down) {
            int row = ui->treeView->currentIndex().row();
            do {
                row = std::min(row + 1, ui->treeView->model()->rowCount() - 1);
                auto index = ui->treeView->model()->index(row, 0);
                if (ui->treeView->model()->flags(index) & Qt::ItemFlag::ItemIsEnabled) {
                    ui->treeView->setCurrentIndex(index);
                    break;
                }
            } while (row != ui->treeView->model()->rowCount() - 1);
        } else if (keyEvent->key() == Qt::Key_Return) {
            clickItem(ui->treeView->currentIndex());
        }
    }
    return false;
}

void Palette::showPalette(const QString &prefix)
{
    const int x = (parentWidget()->width() - width()) / 2;
    const int y = qobject_cast<QMainWindow *>(parentWidget())->menuBar()->height() - 1;

    move(parentWidget()->mapToGlobal(QPoint {x, y}));
    show();
    raise();
    if (!prefix.isEmpty())
        ui->lineEdit->setText(prefix);
}

void Palette::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        hide();
}

void Palette::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // Set default
    {
        QSignalBlocker sb(ui->lineEdit);
        ui->lineEdit->clear();
    }
    ui->lineEdit->setFocus();

    m_currentSelector = 0;
    m_selectors.front().resetFunc();
    m_proxyModel->setFilterWildcard("");
    setSourceModel(m_selectors.front().model.get());
    ui->treeView->setCurrentIndex({});
    updateListHeight();
}

void Palette::changeText(const QString &text)
{
    // TODO would be better to use std::views::reverse, but it doesn't seem to compile on MSVC2019
    int index = 0;
    if (!text.isEmpty()) {
        auto byPrefix = [&text](const auto &selector) {
            return !selector.prefix.isEmpty() && text.startsWith(selector.prefix);
        };
        auto it = std::ranges::find_if(m_selectors, byPrefix);
        if (it != m_selectors.end())
            index = std::distance(m_selectors.begin(), it);
    }

    if (m_currentSelector != index) {
        m_currentSelector = index;
        if (auto resetFunc = m_selectors.at(index).resetFunc)
            resetFunc();
        setSourceModel(m_selectors.at(index).model.get());
    }

    const auto search = text.mid(m_selectors.at(m_currentSelector).prefix.length()).simplified();
    m_proxyModel->setFilterWildcard(search);
    updateListHeight();
}

void Palette::clickItem(const QModelIndex &index)
{
    const auto &selector = m_selectors.at(m_currentSelector);
    Q_ASSERT(selector.selectionFunc);

    if (!index.isValid()) {
        const auto text = ui->lineEdit->text().mid(selector.prefix.length()).simplified();
        selector.selectionFunc(text);
    } else {
        m_selectors.at(m_currentSelector).selectionFunc(index.data(Qt::UserRole));
    }
    hide();
}

void Palette::setSourceModel(QAbstractItemModel *model)
{
    m_proxyModel->setSourceModel(model);

    const int maxCols = model ? model->columnCount() : 0;
    for (int i = 0; i < maxCols - 1; ++i)
        ui->treeView->header()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    if (maxCols)
        ui->treeView->header()->setSectionResizeMode(maxCols - 1, QHeaderView::Stretch);
}

void Palette::updateListHeight()
{
    const int rows = std::min(m_proxyModel->rowCount(), 15);
    if (rows) {
        const auto &rect = ui->treeView->visualRect(m_proxyModel->index(0, 0));
        ui->treeView->setFixedHeight(rows * rect.height());
    } else {
        ui->treeView->setFixedHeight(0);
    }
    setFixedHeight(minimumSizeHint().height());
}

void Palette::addFileSelector()
{
    auto fileModel = std::make_unique<FileModel>();
    auto resetFiles = [model = fileModel.get()]() {
        model->resetFileInfo();
    };
    auto selectFile = [](const QVariant &path) {
        Core::Project::instance()->open(path.toString());
    };
    m_selectors.push_back(Selector {"", std::move(fileModel), resetFiles, selectFile});
}

void Palette::addLineSelector()
{
    auto gotoLine = [](const QVariant &value) {
        bool isInt;
        const int line = value.toInt(&isInt);
        if (isInt) {
            if (auto textDocument = qobject_cast<Core::TextDocument *>(Core::Project::instance()->currentDocument())) {
                textDocument->gotoLine(line);
                textDocument->textEdit()->setFocus(Qt::OtherFocusReason);
                textDocument->textEdit()->centerCursor();
            }
        }
    };
    m_selectors.push_back(Selector {":", nullptr, {}, gotoLine});
}

void Palette::addScriptSelector()
{
    auto runScript = [](const QVariant &fileName) {
        Core::Utils::runScript(fileName.toString(), true);
    };
    m_selectors.push_back(Selector {".", std::make_unique<ScriptModel>(), {}, runScript});
}

void Palette::addSymbolSelector()
{
    auto symbolModel = std::make_unique<SymbolModel>();
    auto resetSymbols = [model = symbolModel.get()]() {
        model->resetSymbols();
    };
    auto gotoSymbol = [](const QVariant &symbolName) {
        if (auto lspDocument = qobject_cast<Core::LspDocument *>(Core::Project::instance()->currentDocument())) {
            lspDocument->selectSymbol(symbolName.toString());
            lspDocument->textEdit()->setFocus(Qt::OtherFocusReason);
            lspDocument->textEdit()->centerCursor();
        }
    };
    m_selectors.push_back(Selector {"@", std::move(symbolModel), resetSymbols, gotoSymbol});
}

void Palette::addActionSelector()
{
    auto actionModel = std::make_unique<ActionModel>();

    actionModel->setActions(qobject_cast<MainWindow *>(parentWidget())->menuActions());

    auto runAction = [](const QVariant &action) {
        auto val = action.value<QAction *>();
        if (val && val->isEnabled())
            val->trigger();
    };
    m_selectors.push_back(Selector {">", std::move(actionModel), {}, runAction});
}

} // namespace Gui
