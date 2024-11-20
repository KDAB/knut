/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "lsp_utils.h"
#include "codedocument.h"
#include "logger.h"
#include "project.h"
#include "textdocument.h"

#include <QPlainTextEdit>
#include <QTextBlock>

namespace Core::Utils {

::Lsp::Position lspFromPos(const TextDocument &textDocument, int pos)
{
    Lsp::Position position;

    auto cursor = textDocument.textEdit()->textCursor();
    cursor.setPosition(pos, QTextCursor::MoveAnchor);

    position.line = cursor.blockNumber();
    position.character = cursor.positionInBlock();
    return position;
}

int lspToPos(const TextDocument &textDocument, const Lsp::Position &pos)
{
    auto document = textDocument.textEdit()->document();
    // Internally, columns are 0-based, like in LSP
    const int blockNumber = qMin((int)pos.line, document->blockCount() - 1);
    const QTextBlock &block = document->findBlockByNumber(blockNumber);
    if (block.isValid()) {
        QTextCursor cursor(block);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.character);
        return cursor.position();
    }
    return 0;
}

RangeMark lspToRange(const TextDocument &textDocument, const Lsp::Range &range)
{
    LoggerDisabler disabler;
    // I know, ugly, but that's the easiest to do that here.
    auto document = const_cast<TextDocument *>(&textDocument);
    return document->createRangeMark(lspToPos(textDocument, range.start), lspToPos(textDocument, range.end));
}

RangeMarkList lspToRangeMarkList(const std::vector<Lsp::Location> &locations)
{
    RangeMarkList rangeMarks;
    rangeMarks.reserve(locations.size());

    for (const auto &location : locations) {
        const auto url = QUrl::fromEncoded(QByteArray::fromStdString(location.uri));
        if (!url.isLocalFile()) {
            continue;
        }
        const auto filepath = url.toLocalFile();

        if (auto document = qobject_cast<TextDocument *>(Project::instance()->get(filepath))) {
            rangeMarks.push_back(lspToRange(*document, location.range));
        }
    }

    return rangeMarks;
}

QString removeTypeAliasInformation(const QString &typeInfo)
{

    auto result = typeInfo;
    qsizetype index;
    while ((index = result.indexOf("(aka ")) != -1) {
        qsizetype start = index;
        int braceCount = 0;
        // find the correct matching ")"
        for (; index < result.size() && braceCount >= 0; index++) {
            if (result[index] == '(') {
                braceCount++;
            } else if (result[index] == ')') {
                braceCount--;
            }
        }
        qsizetype end = index;
        result.erase(result.cbegin() + start, result.cbegin() + end);
    }
    return result.trimmed();
}

} // namespace Core::Utils
