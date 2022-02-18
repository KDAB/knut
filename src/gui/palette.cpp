#include "palette.h"
#include "ui_palette.h"

#include <core/project.h>

#include <QAbstractTableModel>
#include <QDirIterator>
#include <QFileInfo>
#include <QHeaderView>
#include <QKeyEvent>
#include <QShowEvent>
#include <QSortFilterProxyModel>

namespace Gui {

class PaletteFileModel : public QAbstractTableModel
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
        Q_ASSERT(index.isValid());

        switch (role) {
        case Qt::DisplayRole:
            if (index.column() == 0 && role == Qt::DisplayRole) {
                return m_files.at(index.row()).fileName;
            } else if (index.column() == 1) {
                QDir dir(Core::Project::instance()->root());
                return "../" + dir.relativeFilePath(m_files.at(index.row()).path);
            }
            break;
        case Qt::ForegroundRole:
            if (index.column() == 1)
                return QVariant::fromValue(QBrush(Qt::gray));
            break;
        case Qt::FontRole:
            if (index.column() == 1) {
                auto font = QApplication::font();
                font.setItalic(true);
                return font;
            }
            break;
        case Qt::ToolTipRole:
        case Qt::UserRole:
            return m_files.at(index.row()).path;
        }

        return {};
    }

    void updateFileInfo()
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
        std::sort(m_files.begin(), m_files.end(), [](const auto &fi1, const auto &fi2) {
            return fi1.fileName < fi2.fileName;
        });

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

Palette::Palette(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Palette)
    , m_fileModel(new PaletteFileModel(this))
{
    ui->setupUi(this);
    auto filterModel = new QSortFilterProxyModel(this);
    filterModel->setSourceModel(m_fileModel);
    connect(ui->lineEdit, &QLineEdit::textChanged, filterModel, &QSortFilterProxyModel::setFilterWildcard);
    ui->tableView->setModel(filterModel);

    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    connect(ui->tableView, &QTableView::clicked, this, &Palette::openDocument);

    ui->lineEdit->installEventFilter(this);
}

Palette::~Palette() = default;

bool Palette::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    if (event->type() == QEvent::KeyPress) {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Up) {
            const int newRow = std::max(ui->tableView->currentIndex().row() - 1, 0);
            ui->tableView->setCurrentIndex(ui->tableView->model()->index(newRow, 0));
        } else if (keyEvent->key() == Qt::Key_Down) {
            const int newRow =
                std::min(ui->tableView->currentIndex().row() + 1, ui->tableView->model()->rowCount() - 1);
            ui->tableView->setCurrentIndex(ui->tableView->model()->index(newRow, 0));
        } else if (keyEvent->key() == Qt::Key_Return) {
            openDocument(ui->tableView->currentIndex());
        }
    }
    return false;
}

void Palette::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        hide();
}

void Palette::showEvent(QShowEvent *event)
{
    m_fileModel->updateFileInfo();
    QWidget::showEvent(event);
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
    ui->tableView->setCurrentIndex(ui->tableView->model()->index(0, 0));
}

void Palette::openDocument(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const QString path = index.data(Qt::UserRole).toString();
    Core::Project::instance()->open(path);
    hide();
}

} // namespace Gui
