/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "treesitterinspector.h"
#include "core/codedocument.h"
#include "core/logger.h"
#include "core/project.h"
#include "transformpreviewdialog.h"
#include "treesitter/languages.h"
#include "treesitter/predicates.h"
#include "treesitter/transformation.h"
#include "ui_treesitterinspector.h"
#include "utils/log.h"

#include <QMessageBox>
#include <QPalette>
#include <QTextEdit>

namespace Gui {

QueryErrorHighlighter::QueryErrorHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
}

void QueryErrorHighlighter::highlightBlock(const QString &text)
{
    auto previousLength = previousBlockState();
    if (previousLength == -1) {
        previousLength = 0;
    }
    const auto utf8Text = text.toStdString();
    const auto newLength = previousLength + static_cast<int>(utf8Text.length()) + 1 /* the \n character */;
    setCurrentBlockState(newLength);

    if (m_errorUtf8Position != -1 && previousLength <= m_errorUtf8Position && m_errorUtf8Position < newLength) {
        // error is in this block
        auto innerPosition = m_errorUtf8Position - previousLength;

        // get the error position in UTF-16
        auto textBeforeError = QString::fromUtf8(utf8Text.substr(0, innerPosition));
        QTextCharFormat errorFormat;
        errorFormat.setFontWeight(QFont::Bold);
        errorFormat.setFontUnderline(true);
        errorFormat.setUnderlineColor(QColorConstants::Red);
        errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);

        // Unfortunately, TreeSitter doesn't tell us how "long" the error is, only where it starts.
        // So extend the highlight to span one full "keyword".
        //
        // Regex to find the first non-alphanumeric character.
        // Add '#' to highlight the entire predicate, if its a predicate error.
        static QRegularExpression regex("[^\\w#]");
        auto to = text.indexOf(regex, textBeforeError.size());
        if (to == -1) {
            to = text.size();
        }
        auto count = to - textBeforeError.size();
        if (count <= 0) {
            count = 1;
        }

        setFormat(textBeforeError.size(), count, errorFormat);
    }
}

void QueryErrorHighlighter::setUtf8Position(int position)
{
    m_errorUtf8Position = position;
    rehighlight();
}

TreeSitterInspector::TreeSitterInspector(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TreeSitterInspector)
    , m_parser(nullptr)
    , m_errorHighlighter(nullptr)
    , m_document(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    m_errorHighlighter = new QueryErrorHighlighter(ui->query->document());

    connect(Core::Project::instance(), &Core::Project::currentDocumentChanged, this,
            &TreeSitterInspector::changeCurrentDocument);

    ui->treeInspector->setModel(&m_treemodel);

    connect(ui->treeInspector->selectionModel(), &QItemSelectionModel::currentChanged, this,
            &TreeSitterInspector::changeTreeSelection);

    connect(ui->query, &QPlainTextEdit::textChanged, this, &TreeSitterInspector::changeQuery);

    changeCurrentDocument(Core::Project::instance()->currentDocument());

    connect(ui->previewButton, &QPushButton::clicked, this, &TreeSitterInspector::previewTransformation);
    connect(ui->runButton, &QPushButton::clicked, this, &TreeSitterInspector::runTransformation);

    connect(ui->enableUnnamed, &QCheckBox::toggled, this, &TreeSitterInspector::showUnnamedChanged);
}

TreeSitterInspector::~TreeSitterInspector()
{
    delete ui;
}

void TreeSitterInspector::changeQueryState()
{
    if (m_treemodel.hasQuery()) {
        int patternCount = m_treemodel.patternCount();
        int matchCount = m_treemodel.matchCount();

        const QColor col = palette().color(QPalette::ColorGroup::Normal, QPalette::Highlight);

        ui->queryInfo->setText(tr("<span style='color:%1'>%2 Patterns - %3 Matches - %4 Captures</span>")
                                   .arg(patternCount == 0 || matchCount == 0 ? col.name() : "green")
                                   .arg(patternCount)
                                   .arg(matchCount)
                                   .arg(m_treemodel.captureCount()));
    }
}

void TreeSitterInspector::changeCurrentDocument(Core::Document *document)
{
    setDocument(qobject_cast<Core::CodeDocument *>(document));
}

void TreeSitterInspector::changeQuery()
{
    const auto text = ui->query->toPlainText();
    // rehighlight causes another textChanged signal to be emitted.
    // So check whether any characters actually changed.
    if (m_queryText == text) {
        return;
    }
    m_queryText = text;

    if (text.isEmpty()) {
        m_treemodel.setQuery({}, makePredicates());
        ui->queryInfo->setText("");
        m_errorHighlighter->setUtf8Position(-1);
        return;
    }

    try {
        auto lang = treesitter::Parser::getLanguage(m_document->type());
        auto query = std::make_shared<treesitter::Query>(lang, ui->query->toPlainText());
        m_treemodel.setQuery(query, makePredicates());
        m_errorHighlighter->setUtf8Position(-1);

        changeQueryState();
    } catch (treesitter::Query::Error &error) {
        m_treemodel.setQuery({}, nullptr);
        ui->queryInfo->setText(highlightQueryError(error));

        // The error may be behind the last character, which couldn't be highlighted
        // So move back by one character in that case.
        if (error.utf8_offset == text.toStdString().size()) {
            error.utf8_offset--;
        }
        m_errorHighlighter->setUtf8Position(error.utf8_offset);
    }
}

void TreeSitterInspector::showUnnamedChanged()
{
    // technically the text didn't change, but this will force
    // a complete re-parse and re-build of the entire tree.
    changeText();
}

void TreeSitterInspector::changeText()
{
    QString text;
    {
        Core::LoggerDisabler disableLogging;
        text = m_document->text();
    }
    auto tree = m_parser.parseString(text);
    if (tree.has_value()) {
        m_treemodel.setTree(std::move(tree.value()), makePredicates(), ui->enableUnnamed->isChecked());
        ui->treeInspector->expandAll();
        for (int i = 0; i < 2; i++) {
            ui->treeInspector->resizeColumnToContents(i);
        }
        changeQueryState();
    } else {
        m_treemodel.clear();
    }
}

void TreeSitterInspector::changeCursor()
{
    int position;
    {
        Core::LoggerDisabler disableLogging;
        position = m_document->position();
    }
    m_treemodel.setCursorPosition(position);
}

void TreeSitterInspector::setDocument(Core::CodeDocument *document)
{
    if (m_document) {
        m_document->disconnect(this);
    }

    m_document = document;
    if (m_document) {
        m_parser = treesitter::Parser::getLanguage(document->type());
        connect(m_document, &Core::CodeDocument::textChanged, this, &TreeSitterInspector::changeText);
        connect(m_document, &Core::CodeDocument::positionChanged, this, &TreeSitterInspector::changeCursor);

        changeCursor();
        changeText();
    } else {
        m_parser = {nullptr};
        m_treemodel.clear();
    }
}

QString TreeSitterInspector::preCheckTransformation() const
{
    if (m_document == nullptr) {
        return tr("You need to open a C++ document!");
    }

    if (m_queryText.isEmpty()) {
        return tr("You need to specify a query!");
    }

    if (ui->target->toPlainText().isEmpty()) {
        return tr("You need to specify a transformation target!");
    }

    return {};
}

void TreeSitterInspector::previewTransformation()
{
    prepareTransformation([this](auto &transformation) {
        auto result = transformation.run();

        TransformPreviewDialog dialog(m_document, result, transformation.replacementsMade(), this);
        if (dialog.exec() == QDialog::Accepted) {
            m_document->setText(result);
        }
    });
}

QString TreeSitterInspector::highlightQueryError(const treesitter::Query::Error &error) const
{
    return tr("<span style='color:red'><b>%1</b> at character: %2</span>")
        .arg(error.description)
        .arg(error.utf8_offset);
}

void TreeSitterInspector::runTransformation()
{
    prepareTransformation([this](auto &transformation) {
        m_document->setText(transformation.run());

        QMessageBox msgBox;
        msgBox.setText(tr("%1 Replacements made").arg(transformation.replacementsMade()));
    });
}

void TreeSitterInspector::prepareTransformation(
    const std::function<void(treesitter::Transformation &transformation)> &runFunction)
{
    const auto errorMessage = preCheckTransformation();
    if (!errorMessage.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText(errorMessage);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    try {
        auto lang = treesitter::Parser::getLanguage(m_document->type());
        auto query = std::make_shared<treesitter::Query>(lang, m_queryText);
        treesitter::Parser parser(lang);

        treesitter::Transformation transformation(m_document->text(), std::move(parser), query,
                                                  ui->target->toPlainText());

        runFunction(transformation);
    } catch (treesitter::Query::Error &error) {
        QMessageBox msgBox;
        msgBox.setText(tr("Error in Query"));
        msgBox.setInformativeText(highlightQueryError(error));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    } catch (treesitter::Transformation::Error &error) {
        QMessageBox msgBox;
        msgBox.setText(tr("Error performing Transformation"));
        msgBox.setInformativeText(error.description);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
}

std::unique_ptr<treesitter::Predicates> TreeSitterInspector::makePredicates()
{
    if (m_document) {
        // No need to always log the call to TextDocument::text
        Core::LoggerDisabler disabler;
        return std::make_unique<treesitter::Predicates>(m_document->text());
    } else {
        return nullptr;
    }
}

void TreeSitterInspector::changeTreeSelection(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)

    const auto node = m_treemodel.tsNode(current);
    if (node.has_value()) {
        m_document->selectRegion(node->startPosition(), node->endPosition());
    }
}

} // namespace Gui
