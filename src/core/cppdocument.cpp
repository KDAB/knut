/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "cppdocument.h"
#include "cppdocument_p.h"
#include "functionsymbol.h"
#include "logger.h"
#include "project.h"
#include "settings.h"
#include "utils.h"
#include "utils/log.h"

#include <QFileInfo>
#include <QHash>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>
#include <QVariantMap>
#include <algorithm>
#include <kdalgorithms.h>

namespace Core {

/*!
 * \qmltype CppDocument
 * \brief Document object for a C++ file (source or header)
 * \inqmlmodule Knut
 * \ingroup CppDocument/@first
 * \inherits CodeDocument
 */

/*!
 * \qmlproperty bool CppDocument::isHeader
 * Return true if the current document is a header.
 */

CppDocument::CppDocument(QObject *parent)
    : CodeDocument(Type::Cpp, parent)
{
}

CppDocument::~CppDocument() = default;

static bool isHeaderSuffix(const QString &suffix)
{
    // Good enough for now, headers starts with h or hpp
    return suffix.startsWith('h');
}

bool CppDocument::isHeader() const
{
    const QFileInfo fi(fileName());
    return isHeaderSuffix(fi.suffix());
}

/*!
 * \qmlmethod CppDocument::commentSelection()
 * Comments the selected lines (or current line if there's no selection) in current document.
 *
 * - If there's no selection, current line is commented using `//`.
 * - If there's a valid selection and the start and end position of the selection are before any text of the lines,
 *   all of the selected lines are commented using `//`.
 * - If there's a valid selection and the start and/or end position of the selection are between any text of the
 *   lines, all of the selected lines are commented using multi-line comment.
 * - If selection or position is invalid or out of range, or the position is on an empty line, the document remains
 *   unchanged.
 */
void CppDocument::commentSelection()
{
    LOG("CppDocument::commentSelection");

    QTextCursor cursor = textEdit()->textCursor();
    cursor.beginEditBlock();

    const int cursorPos = cursor.position();
    int selectionOffset = 0;

    if (hasSelection()) {
        int selectionStartPos = cursor.selectionStart();
        const int selectionEndPos = cursor.selectionEnd();

        // Preparing to check if the start and end positions of the selection are before any text of the lines
        cursor.setPosition(selectionStartPos);
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        const QString str1 = cursor.selectedText();
        cursor.setPosition(selectionEndPos);
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        const QString str2 = cursor.selectedText();

        if (str1.trimmed().isEmpty() && str2.trimmed().isEmpty()) {
            // Comment all lines in the selected region with "//"
            cursor.setPosition(selectionStartPos);
            cursor.movePosition(QTextCursor::StartOfLine);
            selectionStartPos = cursor.position();

            cursor.setPosition(selectionEndPos);
            // If the end of selection is at the beginning of the line, don't comment out the line the cursor is in.
            if (str2.isEmpty())
                cursor.movePosition(QTextCursor::Left);
            cursor.movePosition(QTextCursor::StartOfLine);

            do {
                cursor.insertText("//");
                selectionOffset += 2;
                cursor.movePosition(QTextCursor::Up);
                cursor.movePosition(QTextCursor::StartOfLine);
            } while (cursor.position() >= selectionStartPos);
        } else {
            // Comment the selected region using "/*" and "*/"
            cursor.setPosition(selectionEndPos);
            cursor.insertText("*/");
            selectionOffset += 2;
            cursor.setPosition(selectionStartPos);
            cursor.insertText("/*");
            selectionOffset += 2;
        }

        // Set the selection after commenting
        if (cursorPos == selectionEndPos) {
            cursor.setPosition(selectionStartPos);
            cursor.setPosition(selectionEndPos + selectionOffset, QTextCursor::KeepAnchor);
        } else {
            cursor.setPosition(selectionEndPos + selectionOffset);
            cursor.setPosition(selectionStartPos, QTextCursor::KeepAnchor);
        }
    } else {
        cursor.select(QTextCursor::LineUnderCursor);
        // If the line is not empty, then comment it using "//"
        if (!cursor.selectedText().isEmpty()) {
            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.insertText("//");
            selectionOffset += 2;
        }

        // Set the position after commenting
        cursor.setPosition(cursorPos + selectionOffset);
    }

    cursor.endEditBlock();
    textEdit()->setTextCursor(cursor);
}

static QStringList matchingSuffixes(bool header)
{
    static const auto mimeTypes =
        Settings::instance()->value<std::map<std::string, Document::Type>>(Settings::MimeTypes);

    QStringList suffixes;
    for (const auto &it : mimeTypes) {
        if (it.second == Document::Type::Cpp) {
            const QString suffix = QString::fromStdString(it.first);
            if ((header && !isHeaderSuffix(suffix)) || (!header && isHeaderSuffix(suffix)))
                suffixes.push_back(suffix);
        }
    }
    return suffixes;
}

static QStringList candidateFileNames(const QString &baseName, const QStringList &suffixes)
{
    QStringList result;
    result.reserve(suffixes.size());
    for (const auto &suffix : suffixes)
        result.push_back(baseName + '.' + suffix);
    return result;
}

static int commonFilePathLength(const QString &s1, const QString &s2)
{
    const qsizetype length = qMin(s1.length(), s2.length());
    for (qsizetype i = 0; i < length; ++i) {
        if (s1[i].toLower() != s2[i].toLower())
            return i;
    }
    return length;
}

/*!
 * \qmlmethod string CppDocument::correspondingHeaderSource()
 * Returns the corresponding source or header file path.
 */
QString CppDocument::correspondingHeaderSource() const
{
    LOG("CppDocument::correspondingHeaderSource");
    static QHash<QString, QString> cache;

    QString cacheData = cache.value(fileName());
    if (!cacheData.isEmpty())
        return cacheData;

    const bool header = isHeader();
    const QStringList suffixes = matchingSuffixes(header);

    const QFileInfo fi(fileName());
    const QStringList candidates = candidateFileNames(fi.completeBaseName(), suffixes);

    // Search in the current directory
    for (const auto &candidate : candidates) {
        const QString testFileName = fi.absolutePath() + '/' + candidate;
        if (QFile::exists(testFileName)) {
            cache[fileName()] = testFileName;
            cache[testFileName] = fileName();
            spdlog::debug("CppDocument::correspondingHeaderSource {} => {}", fileName(), testFileName);
            LOG_RETURN("path", testFileName);
        }
    }

    // Search in the whole project, and find the possible files
    QStringList fullPathNames = Project::instance()->allFilesWithExtensions(suffixes, Project::FullPath);
    auto notCandidate = [&candidates](const QString &path) {
        auto notInPath = [&path](const QString &candidate) {
            return !path.endsWith(candidate, Qt::CaseInsensitive);
        };
        return kdalgorithms::all_of(candidates, notInPath);
    };
    kdalgorithms::erase_if(fullPathNames, notCandidate);

    // Find the file having the most common path with fileName
    QString bestFileName;
    int compareValue = 0;
    for (const auto &path : std::as_const(fullPathNames)) {
        int value = commonFilePathLength(path, fileName());
        if (value > compareValue) {
            compareValue = value;
            bestFileName = path;
        }
    }

    if (!bestFileName.isEmpty()) {
        cache[fileName()] = bestFileName;
        cache[bestFileName] = fileName();
        spdlog::debug("CppDocument::correspondingHeaderSource {} => {}", fileName(), bestFileName);
        LOG_RETURN("path", bestFileName);
    }

    spdlog::warn("CppDocument::correspondingHeaderSource {} - not found ", fileName());
    return {};
}

/*!
 * \qmlmethod CppDocument CppDocument::openHeaderSource()
 * Opens the corresponding source or header files, the current document is the new file.
 * If no files have been found, it's a no-op.
 */
CppDocument *CppDocument::openHeaderSource()
{
    LOG("CppDocument::openHeaderSource");
    const QString fileName = correspondingHeaderSource();
    if (!fileName.isEmpty())
        LOG_RETURN("document", qobject_cast<CppDocument *>(Project::instance()->open(fileName)));
    return nullptr;
}

/*!
 * \qmlmethod QueryMatch CppDocument::queryClassDefinition(string className)
 *
 * Returns the class or struct definition matching the given `className`.
 *
 * The returned QueryMatch instance will have the following captures available:
 *
 * - `name` - The name of the class or struct
 * - `base` - The list of base classes/structs, if any
 * - `body` - The body of the class or struct definition (including curly-braces)
 */
Core::QueryMatch CppDocument::queryClassDefinition(const QString &className)
{
    LOG("CppDocument::queryClassDefinition", LOG_ARG("className", className));

    // clang-format off
    const auto classDefinitionQuery = QString(R"EOF(
        ; query classes or structs
        [(class_specifier
            name: (_) @name (#like? @name "%1")
            (base_class_clause
                [(type_identifier) @base _]*)?
            body: (_) @body)
        (struct_specifier
            name: (_) @name (#like? @name "%1")
            (base_class_clause
                [(type_identifier) @base _]*)?
            body: (_) @body)]
    )EOF").arg(className);
    // clang-format on

    auto matches = query(classDefinitionQuery);
    if (matches.isEmpty()) {
        spdlog::warn("CppDocument::queryClassDefinition: No class named `{}` found in `{}`", className, fileName());
        return {};
    }

    if (matches.size() > 1) {
        spdlog::error("CppDocument::queryClassDefinition: Multiple classes named `{}` found in `{}`!", className,
                      fileName());
    }
    return matches.first();
}

/*!
 * \qmlmethod array<QueryMatch> CppDocument::queryMethodDefinition(string scope, string methodName)
 *
 * Returns the list of methods definitions matching the given name and scope.
 * `scope` may be either a class name, a namespace or empty.
 *
 * Every QueryMatch returned by this function will have the following captures available:
 *
 * - `scope` - The scope of the method (if any is provided)
 * - `name` - The name of the function
 * - `definition` - The entire method definition
 * - `parameter-list` - The list of parameters
 * - `parameters` - One capture per parameter, containing the type and name of the parameter, excluding comments!
 * - `body` - The body of the method (including curly-braces)
 *
 * Please note that the return type is not available, as TreeSitter is not able to parse it easily.
 */
Core::QueryMatchList CppDocument::queryMethodDefinition(const QString &scope, const QString &functionName)
{
    LOG("CppDocument::queryMethodDefinition", LOG_ARG("scope", scope), LOG_ARG("functionName", functionName));

    // Clang-format gets confused by the raw strings
    // clang-format off
    auto identifier = QString(R"EOF(
            (identifier) @name (#eq? @name "%1")
        )EOF").arg(functionName);

    if (!scope.isEmpty()) {
        identifier = QString(R"EOF(
            (qualified_identifier
                scope: (_) @scope (#like? @scope "%1")
                %2
            )
        )EOF").arg(scope, identifier);
    }

    const auto queryFunctionName = QString(R"EOF(
        (function_declarator
            declarator: %1
            parameters: (parameter_list
                (parameter_declaration)* @parameters
            ) @parameter-list
        )
    )EOF").arg(identifier);
    // handle Type, Type *, Type &, Type *&, Type &* and Type **
    const auto queryString = QString(R"EOF(
        (function_definition
            type: (_)? @return-type
            [
                declarator: (_ (_ %1 ) )
                declarator: (_ %1 )
                declarator: %1
            ]
            body: (compound_statement) @body
        ) @definition
    )EOF").arg(queryFunctionName);
    //clang-format on

    return query(queryString);
}

QList<QueryMatch> CppDocument::internalQueryFunctionCall(const QString& functionName, const QString& argumentsQuery)
{
    const auto queryString = QString(R"EOF(
                (call_expression
                    function: (_) @name (#eq? @name "%1")
                    arguments: (argument_list
                            %2
                        ) @argument-list
                ) @call
    )EOF").arg(functionName, argumentsQuery);

    return query(queryString);
}

/*!
 * \qmlmethod array<QueryMatch> CppDocument::queryFunctionCall(string functionName, array<string> argumentCaptures)
 *
 * Returns the list of function calls to the function `functionName`.
 * Only calls that have the same number of arguments as `argumentCaptures` will be returned.
 *
 * The `argumentCaptures` list is a list of names that will be used to capture the arguments of the function call.
 * E.g. `queryFunctionCall("foo", ["first", "second"])` will return a list of calls to `foo` with two arguments,
 * where the first argument will be captured in the `first` capture, and the second in the `second` capture.
 *
 * The returned QueryMatch instances will have the following captures available:
 *
 * - `call` - The entire call expression
 * - `name` - The name of the function (the text will be equal to functionName)
 * - `argument-list` - The entire list of arguments, including the surroundg parentheses `()`
 * - a capture for every argument in `argumentCaptures`
 */
Core::QueryMatchList CppDocument::queryFunctionCall(const QString &functionName, const QStringList &argumentCaptures)
{
    LOG("queryFunctionCall", LOG_ARG("functionName", functionName), LOG_ARG("argumentCaptures", argumentCaptures));

    for (const auto &argument : argumentCaptures) {
        if (kdalgorithms::value_in(argument, {"call", "name", "argument-list"}) ){
            spdlog::warn("CppDocument::queryFunctionCall: provided capture {} is reserved!", argument);
        }
    }

    const auto arguments = kdalgorithms::transformed(argumentCaptures, [](const auto& name) {
            return QString(". (_)+ @%1").arg(name);
            });
    const auto argumentsQuery = arguments.join(" \",\"\n");


    return internalQueryFunctionCall(functionName, QString(R"EOF(
        . "("
        %1
        . ")" .
    )EOF").arg(argumentsQuery));
}

/*!
 * \qmlmethod array<QueryMatch> CppDocument::queryFunctionCall(string functionName)
 *
 * Returns the list of function calls to the function `functionName`, no matter how many arguments they were called with.
 *
 * The returned QueryMatch instances will have the following captures available:
 *
 * - `call` - The entire call expression
 * - `name` - The name of the function (the text will be equal to functionName)
 * - `argument-list` - The entire list of arguments, including the surroundg parentheses `()`
 * - `arguments` - Each argument provided to the function call, in order, excluding any comments
 */
Core::QueryMatchList CppDocument::queryFunctionCall(const QString& functionName)
{
    LOG("queryFunctionCall", LOG_ARG("functionName", functionName));
    return internalQueryFunctionCall(
            functionName,
            R"EOF([(_) @arguments ","]* (#exclude! @arguments comment))EOF");
}

/*!
 * \qmlmethod CppDocument::insertCodeInMethod(string methodName, string code, Position insertAt)
 *
 * Provides a fast way to add some code in an existing method definition. Does nothing if the method does not exist in
 * the current document.
 *
 * This method will find a method in the current file with name matching with `methodName`. If the method exists in the
 * current document, then it will insert the supplied `code` either at the beginning of the method, or at the end of the
 * method, depending on the `insertAt` argument.
 */
bool CppDocument::insertCodeInMethod(const QString &methodName, QString code, Position insertAt)
{
    LOG("CppDocument::insertCodeInMethod", methodName, code, insertAt);

    auto symbol = findSymbol(methodName);
    if (!symbol) {
        spdlog::warn("CppDocument::insertCodeInMethod: No symbol found for {}.", methodName);
        return false;
    }

    if (!symbol->isFunction()) {
        spdlog::warn("CppDocument::insertCodeInMethod: {} is not a function or a method.",
                     symbol->name());
        return false;
    }

    QTextCursor cursor = textEdit()->textCursor();
    cursor.setPosition(symbol->range().end);
    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    if (cursor.selectedText() != "}") {
        spdlog::warn("CppDocument::insertCodeInMethod: {} is not a function definition.", symbol->name());
        return false;
    }

    cursor.beginEditBlock();
    // Goto the end and move back one character
    cursor.setPosition(symbol->range().end);
    cursor.movePosition(QTextCursor::PreviousCharacter);

    const QString strTab = tab();
    if (insertAt == StartOfMethod) {
        // Goto the start of the block
        textEdit()->setTextCursor(cursor);
        cursor.setPosition(gotoBlockStart());
        // Move forward one character
        cursor.movePosition(QTextCursor::NextCharacter);
        // Insert a new line
        cursor.insertText("\n");
    }

    // Insert an indent before the first line
    cursor.insertText(strTab);
    // Insert an indent before every line in the code
    code.replace("\n", ("\n" + strTab));

    if (insertAt == EndOfMethod)
        if (!code.endsWith("\n" + strTab))
            code.append("\n");

    // If there's an extra tab at the end, chop it
    if (code.endsWith(strTab))
        code.chop(strTab.length());

    cursor.insertText(code);
    cursor.endEditBlock();

    textEdit()->setTextCursor(cursor);

    return true;
}

/*!
 * \qmlmethod CppDocument::insertForwardDeclaration(string forwardDeclaration)
 * Inserts the forward declaration `forwardDeclaration` into the current file.
 * The method will check if the file is a header file, and also that the forward declaration starts with 'class ' or
 * 'struct '. Fully qualified the forward declaration to add namespaces: `class Foo::Bar::FooBar` will result in:
 *
 * ```cpp
 * namespace Foo {
 * namespace Bar {
 * class FooBar
 * }
 * }
 * ```
 */
bool CppDocument::insertForwardDeclaration(const QString &forwardDeclaration)
{
    LOG("CppDocument::insertForwardDeclaration", LOG_ARG("text", forwardDeclaration));
    if (!isHeader()) {
        spdlog::warn("CppDocument::insertForwardDeclaration: {} - is not a header file. ", fileName());
        return false;
    }

    const int spacePos = forwardDeclaration.indexOf(' ');
    const auto classOrStruct = QStringView(forwardDeclaration).left(spacePos);
    if (forwardDeclaration.isEmpty() || (classOrStruct != QStringLiteral("class") && classOrStruct != QStringLiteral("struct"))) {
        spdlog::warn("CppDocument::insertForwardDeclaration: {} - should start with 'class ' or 'struct '. ",
                     forwardDeclaration);
        return false;
    }

    auto qualifierList = QStringView(forwardDeclaration).mid(spacePos + 1).split(QStringLiteral("::"));
    std::ranges::reverse(qualifierList);

    // Get the un-qualified declaration
    QString result = QString("%1 %2;").arg(classOrStruct).arg(qualifierList.first());
    qualifierList.pop_front();

    // Check if the declaration already exists
    QTextDocument *doc = textEdit()->document();
    QTextCursor cursor(doc);
    cursor = doc->find(result, cursor, QTextDocument::FindWholeWords);
    if (!cursor.isNull()) {
        spdlog::warn("CppDocument::insertForwardDeclaration: '{}' - already exists in file.", forwardDeclaration);
        return false;
    }

    for (const auto &qualifier : std::as_const(qualifierList))
        result = QString("namespace %1 {\n%2\n}").arg(qualifier, result);

    int pos = -1;
    if (const auto inc = query(Queries::findInclude); !inc.isEmpty()) {
        const auto def = inc.last().get("path");
        pos = def.end();
    } else if (const auto pragma = query(Queries::findPragma); !pragma.isEmpty()) {
        const auto def = pragma.at(0).get("value");
        pos = def.end();
    } else if (const auto guard = query(Queries::findHeaderGuard); !guard.isEmpty()) {
        const auto def = guard.at(0).get("value");
        pos = def.end();
    }

    if (pos != -1) {
        auto cur = textEdit()->textCursor();
        cur.setPosition(pos);
        textEdit()->setTextCursor(cur);
        cur.beginEditBlock();
        cur.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
        cur.insertText("\n\n" + result);
        cur.endEditBlock();

        return true;
    }

    return false;
}

/*!
 * \qmlmethod DataExchange CppDocument::mfcExtractDDX(string className)
 * Extracts the DDX information from a MFC class.
 *
 * The DDX information gives the mapping between the IDC and the member variables in the class.
 * \see DataExchange
 */
DataExchange CppDocument::mfcExtractDDX(const QString &className)
{
    LOG("CppDocument::mfcExtractDDX", LOG_ARG("text", className));

    auto functions = queryMethodDefinition(className, "DoDataExchange");

    if (functions.isEmpty()) {
        spdlog::warn("CppDocument::mfcExtractDDX: No DoDataExchange found in `{}`", fileName());
        return {};
    }

    if (functions.size() > 1) {
        spdlog::warn("CppDocument::mfcExtractDDX: Too many DoDataExchange methods found in `{}`", fileName());
    }

    auto &function = functions.first();

    return DataExchange(className, function);
}

/*!
 * \qmlmethod MessageMap CppDocument::mfcExtractMessageMap(string className = "")
 *
 * Extracts information contained in the MFC MESSAGE_MAP.
 * The `className` parameter can be used to ensure the result matches to a specific class.
 */
MessageMap CppDocument::mfcExtractMessageMap(const QString &className /* = ""*/)
{
    auto checkClassName = className.isEmpty() ? "" : QString("(#eq? @class \"%1\")").arg(className);

    // clang-format off
    const auto messageMapQueryString = QString(R"EOF(
        ; Search for BEGIN_MESSAGE_MAP
        (expression_statement
            (call_expression
                function: (identifier) @begin_ident
                (#eq? @begin_ident "BEGIN_MESSAGE_MAP")
                arguments: (argument_list
                        (identifier) @class
                        %1 ; If a class name is given, check if the captured class name matches
                        (identifier) @superclass)) @begin)

        ; Followed by one or more entries
        [
        (expression_statement
            (call_expression
                function: (identifier) @message-name
                arguments: (argument_list
                    [(_)* @parameter ","]*
                    (#exclude! @parameter comment))
        ))@message
        (_)
        ]*

        ; Ending with END_MESSAGE_MAP
        (expression_statement
            (call_expression
                function: (identifier) @end_ident
                (#eq? @end_ident "END_MESSAGE_MAP")) @end)
    )EOF").arg(checkClassName);
    // clang-format on

    // clang-format off
    // Assumption: the MESSAGE_MAP is either top-level or in a namespace
    // Parenthesis (around %1) are used to make sure nodes are siblings
    const auto queryString = QString(R"EOF(
        (translation_unit
            [
                (namespace_definition (_ ( %1 ) ) )
                ( %1 )
            ]
        )
    )EOF").arg(messageMapQueryString);
    // clang-format on

    // We assume there is at most one MessageMap per file.
    // This allows us to return immediately after the message map is found.
    // As the MessageMap query is quite complicated, this can significantly improve performance.
    auto match = queryFirst(queryString);
    if (match.isEmpty()) {
        spdlog::warn("CppDocument::mfcExtractMessageMap: No message map found in `{}`", fileName());
        return {};
    }

    return MessageMap(match);
}

/*!
 * \qmlmethod array<QueryMatch> CppDocument::queryMethodDeclaration(string className, string functionName)
 *
 * Finds the declaration of a method inside a class or struct definition.
 *
 * Returns a list of QueryMatch objects containing the declaration.
 * A warning will be logged if no declarations are found.
 *
 * The returned QueryMatch instances contain the following captures:
 *
 * - `declaration`: The full declaration of the method
 * - `function`: The function declaration, without the return type
 * - `name`: The name of the function
 */
Core::QueryMatchList CppDocument::queryMethodDeclaration(const QString &className, const QString &functionName)
{
    LOG("CppDocument::queryMethodDeclaration", LOG_ARG("className", className), LOG_ARG("functionName", functionName));

    auto classQuery = queryClassDefinition(className);

    // TODO: extract parameters
    // TODO: make it works with constructors and destructors

    // clang-format off
    auto queryFunctionName = QString(R"EOF(
        (function_declarator
            declarator:(field_identifier) @name (#eq? @name "%1")
        )
    )EOF").arg(functionName);
    // handle Type, Type *, Type &, Type *&, Type &* and Type **
    auto queryString = QString(R"EOF(
        (field_declaration
            type: (_)? @return-type
            [
                declarator: (_ (_ %1 ) )
                declarator: (_ %1 )
                declarator: %1
            ]
        ) @declaration
    )EOF").arg(queryFunctionName);
    // clang-format on

    auto matches = classQuery.queryIn("body", queryString);
    if (matches.isEmpty()) {
        spdlog::warn("CppDocument::queryMethodDeclaration: No method named `{}` found in `{}`", functionName,
                     fileName());
    }

    return matches;
}

/*!
 * \qmlmethod QueryMatch CppDocument::queryMember(string className, string memberName)
 *
 * Finds the member definition inside a class or struct definition.
 * Returns a QueryMatch object containing the member definition if it exists.
 *
 * The returned QueryMatch instance will have the following captures available:
 *
 * - `member`: The full definition of the member
 * - `type`: The type of the member, without `const` or any reference/pointer specifiers (i.e. `&`/`*`)
 * - `name`: The name of the member (should be equal to memberName)
 */
Core::QueryMatch CppDocument::queryMember(const QString &className, const QString &memberName)
{
    LOG("CppDocument::queryMember", LOG_ARG("className", className), LOG_ARG("memberName", memberName));

    auto classQuery = queryClassDefinition(className);

    // clang-format off
    auto queryMemberName = QString("(field_identifier) @name");
    // handle Type, Type *, Type &, Type *&, Type &* and Type **
    auto queryString = QString(R"EOF(
        (field_declaration
            type: (_) @type
            [
                declarator: (_ (_ %1 ) )
                declarator: (_ %1 )
                declarator: %1
            ]
            (#eq? @name "%2")
        ) @member
    )EOF").arg(queryMemberName, memberName);
    // clang-format on

    auto matches = classQuery.queryIn("body", queryString);
    if (matches.isEmpty()) {
        spdlog::warn("CppDocument::queryMember: No member named `{}` found in `{}`", memberName, fileName());
        return {};
    }

    if (matches.size() > 1) {
        spdlog::error("CppDocument::queryMember: Multiple members named `{}` found in `{}`!", memberName, fileName());
    }
    return matches.first();
}

/*!
 * \qmlmethod int CppDocument::gotoBlockStart(int count)
 * Moves the cursor to the start of the block it's in, and returns the new cursor position.
 * A block is defined by {} or () or [].
 * Does it `count` times.
 */
int CppDocument::gotoBlockStart(int count)
{
    LOG_AND_MERGE("CppDocument::gotoBlockStart", count);

    QTextCursor cursor = textEdit()->textCursor();
    while (count != 0) {
        cursor.setPosition(moveBlock(cursor.position(), QTextCursor::PreviousCharacter));
        --count;
    }
    textEdit()->setTextCursor(cursor);
    return cursor.position();
}

/*!
 * \qmlmethod int CppDocument::gotoBlockEnd(int count)
 * Moves the cursor to the end of the block it's in, and returns the new cursor position.
 * A block is defined by {} or () or [].
 * Does it `count` times.
 */
int CppDocument::gotoBlockEnd(int count)
{
    LOG_AND_MERGE("CppDocument::gotoBlockEnd", count);

    QTextCursor cursor = textEdit()->textCursor();
    while (count != 0) {
        cursor.setPosition(moveBlock(cursor.position(), QTextCursor::NextCharacter));
        --count;
    }
    textEdit()->setTextCursor(cursor);
    return cursor.position();
}

/*!
 * \qmlmethod int CppDocument::selectBlockStart()
 * Selects the text from current cursor position to the start of the block, and returns the new cursor position.
 * A block is defined by {} or () or [].
 * Does it `count` times.
 */
int CppDocument::selectBlockStart(int count)
{
    LOG_AND_MERGE("CppDocument::selectBlockStart", count);

    QTextCursor cursor = textEdit()->textCursor();
    const int selectionStart = std::max(cursor.selectionStart(), cursor.selectionEnd());
    while (count != 0) {
        cursor.setPosition(moveBlock(cursor.position(), QTextCursor::PreviousCharacter));
        --count;
    }
    const int blockStartPos = cursor.position();

    cursor.setPosition(selectionStart, QTextCursor::MoveAnchor);
    cursor.setPosition(blockStartPos, QTextCursor::KeepAnchor);

    textEdit()->setTextCursor(cursor);
    return blockStartPos;
}

/*!
 * \qmlmethod int CppDocument::selectBlockEnd()
 * Selects the text from current cursor position to the end of the block, and returns the new cursor position.
 * A block is defined by {} or () or [].
 * Does it `count` times.
 */
int CppDocument::selectBlockEnd(int count)
{
    LOG_AND_MERGE("CppDocument::selectBlockEnd", count);

    QTextCursor cursor = textEdit()->textCursor();
    const int selectionStart = std::min(cursor.selectionStart(), cursor.selectionEnd());
    while (count != 0) {
        cursor.setPosition(moveBlock(cursor.position(), QTextCursor::NextCharacter));
        --count;
    }
    const int blockEndPos = cursor.position();

    cursor.setPosition(selectionStart, QTextCursor::MoveAnchor);
    cursor.setPosition(blockEndPos, QTextCursor::KeepAnchor);

    textEdit()->setTextCursor(cursor);
    return blockEndPos;
}

/*!
 * \qmlmethod int CppDocument::selectBlockUp()
 * Selects the text of the block the cursor is in, and returns the new cursor position.
 * A block is defined by {} or () or [].
 * Does it `count` times.
 */
int CppDocument::selectBlockUp(int count)
{
    LOG_AND_MERGE("CppDocument::selectBlockUp", count);

    QTextCursor cursor = textEdit()->textCursor();
    while (count != 0) {
        cursor.setPosition(moveBlock(cursor.position(), QTextCursor::NextCharacter));
        --count;
    }
    const int blockEndPos = cursor.position();
    const int blockStartPos = moveBlock(cursor.position(), QTextCursor::PreviousCharacter);
    cursor.setPosition(blockStartPos, QTextCursor::MoveAnchor);
    cursor.setPosition(blockEndPos, QTextCursor::KeepAnchor);

    textEdit()->setTextCursor(cursor);
    return blockEndPos;
}

/**
 * \brief Internal method to move to the start or end of a block
 * \param startPos current cursor position
 * \param direction the iteration
 * \return position of the start or end of the block
 */
int CppDocument::moveBlock(int startPos, QTextCursor::MoveOperation direction)
{
    Q_ASSERT(direction == QTextCursor::NextCharacter || direction == QTextCursor::PreviousCharacter);

    QTextDocument *doc = textEdit()->document();
    Q_ASSERT(doc);

    const int inc = direction == QTextCursor::NextCharacter ? 1 : -1;
    const int lastPos = direction == QTextCursor::NextCharacter ? textEdit()->document()->characterCount() - 1 : 0;
    if (startPos == lastPos)
        return startPos;
    int pos = startPos + inc;
    QChar currentChar = doc->characterAt(pos);

    // Set the characters delimiter that increment or decrement the counter when iterating
    auto incCounterChar =
        direction == QTextCursor::NextCharacter ? QList<QChar> {'(', '{', '['} : QList<QChar> {')', '}', ']'};
    auto decCounterChar =
        direction == QTextCursor::PreviousCharacter ? QList<QChar> {'(', '{', '['} : QList<QChar> {')', '}', ']'};

    // If the character next is a special one, go inside the block
    if (incCounterChar.contains(currentChar))
        pos += inc;

    // Iterate to find the other side of the block
    int counter = 0;
    pos += inc;

    auto hitLastChar = [direction, lastPos](int pos) {
        return direction == QTextCursor::NextCharacter ? pos >= lastPos : pos <= lastPos;
    };

    while (!hitLastChar(pos)) {
        currentChar = doc->characterAt(pos);

        if (incCounterChar.contains(currentChar)) {
            counter++;

        } else if (decCounterChar.contains(currentChar)) {
            counter--;

            // When counter is negative, we have found the other side of the block
            if (counter < 0)
                return pos + std::max(inc, 0);
        }
        pos += inc;
    }
    return startPos;
}

/*!
 * \qmlmethod CppDocument::toggleSection()
 * Comments out a section of the code using `#ifdef` / `#endif`. The variable used is defined by the settings.
 * ```json
 * "toggle_section": {
 *     "tag": "KDAB_TEMPORARILY_REMOVED",
 *     "debug": "qDebug(\"%1 is commented out\")"
 *     "return_values": {
 *         "BOOL": "false"
 *     }
 * }
 * ```
 * `debug` is the debug line to show, if empty it won't show anything. `return_values` gives a mapping for the value
 * returned by the function. In this example, if the returned type is `BOOL`, it will return `false`. If text is
 * selected, it comment out the lines of the selected text. Otherwise, it will comment the function the cursor is in. In
 * the latter case, if the function is already commented, it will remove the commented section.
 */
void CppDocument::toggleSection()
{
    LOG("CppDocument::toggleSection");

    auto sectionSettings = Settings::instance()->value<ToggleSectionSettings>(Settings::ToggleSection);
    const auto endifString = QStringLiteral("#endif // ") + sectionSettings.tag;
    const auto ifdefString = QStringLiteral("#ifdef ") + sectionSettings.tag;
    const auto elseString = QStringLiteral("#else // ") + sectionSettings.tag;
    const auto newLine = QStringLiteral("\n");

    QTextCursor cursor = textEdit()->textCursor();
    if (cursor.hasSelection()) {
        // If there's a selection, just add #ifdef/#endif
        cursor.beginEditBlock();
        auto [min, max] = std::minmax({cursor.selectionStart(), cursor.selectionEnd()});
        int line, col;
        convertPosition(max, &line, &col);
        // Add #endif / #ifdef, starts to the end or min/max won't be right
        cursor.setPosition(position(QTextCursor::EndOfLine, col == 1 ? max - 1 : max));
        cursor.insertText(newLine + endifString);
        cursor.setPosition(position(QTextCursor::StartOfLine, min));
        cursor.insertText(ifdefString + newLine);
        // Move after the #endif
        cursor.endEditBlock();
        textEdit()->setTextCursor(cursor);
        gotoLine(line + 3);

    } else {
        // Check that we are in a function
        auto isFunction = [](const Symbol &symbol) {
            return symbol.isFunction();
        };
        auto symbol = currentSymbol(isFunction);
        if (!symbol)
            return;

        auto cursorPos = cursor.position();

        cursor.beginEditBlock();
        // Start from the end
        cursor.setPosition(symbol->range().end);
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::Up, QTextCursor::KeepAnchor);

        if (cursor.selectedText().startsWith(endifString)) {
            // The function is already commented out, remove the comments
            int start = textEdit()->document()->find(elseString, cursor, QTextDocument::FindBackward).selectionStart();
            if (start > symbol->range().start)
                cursor.setPosition(start, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            cursor.setPosition(moveBlock(cursor.position(), QTextCursor::PreviousCharacter));
            cursor.movePosition(QTextCursor::Down);
            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            cursorPos -= ifdefString.length() + 1;
        } else {
            // Comment out the function with #if/#def, make sure to return something if needed
            cursor.setPosition(symbol->range().end);
            cursor.movePosition(QTextCursor::PreviousCharacter);

            QString text = elseString + newLine;
            if (!sectionSettings.debug.isEmpty())
                text += tab() + sectionSettings.debug.arg(symbol->name()) + ";\n";
            const QString returnType = symbol->toFunction()->returnType();
            auto it = sectionSettings.return_values.find(returnType.toStdString());
            if (it != sectionSettings.return_values.end())
                text += tab() + QString("return %1;\n").arg(QString::fromStdString(it->second));
            else if (returnType.isEmpty() || returnType == "void")
                text += tab() + "return;\n";
            else if (returnType.endsWith('*'))
                text += tab() + "return nullptr;\n";
            else
                text += tab() + "return {};\n";
            text += endifString + newLine;
            cursor.insertText(text);

            cursor.setPosition(moveBlock(cursor.position(), QTextCursor::PreviousCharacter));
            cursor.movePosition(QTextCursor::NextCharacter);
            cursor.insertText(newLine + ifdefString);
            cursorPos += ifdefString.length() + 1;
        }
        cursor.endEditBlock();
        textEdit()->setTextCursor(cursor);
        setPosition(cursorPos);
    }
}

/*!
 * \qmlmethod CppDocument::insertInclude(string include, bool newGroup = false)
 * Inserts a new include line in the file. If the include is already in, do nothing (and returns true).
 *
 * The `include` string should be either `<foo.h>` or `"foo.h"`, it will returns false otherwise.
 * The method will try to find the best group of includes to insert into, a group of includes being consecutive includes
 * in the file.
 *
 * If `newGroup` is true, it will insert the include at the end, with a new line separating the other includes.
 */
bool CppDocument::insertInclude(const QString &include, bool newGroup)
{
    LOG("CppDocument::insertInclude", LOG_ARG("text", include), newGroup);

    IncludeHelper includeHelper(this);
    auto includePos = includeHelper.includePositionForInsertion(include, newGroup);
    if (!includePos) {
        spdlog::error(R"(CppDocument::insertInclude - the include '{}' is malformed, should be '<foo.h>' or '"foo.h"')",
                      include);
        return false;
    }

    if (includePos->alreadyExists()) {
        spdlog::info("CppDocument::insertInclude - the include '{}' is already included.", include);
        return true;
    }

    const QString text = (includePos->newGroup ? "\n#include " : "#include ") + include + '\n';
    insertAtLine(text, includePos->line);
    return true;
}

CppDocument::MemberOrMethodAdditionResult
CppDocument::addMemberOrMethod(const QString &memberInfo, const QString &className, AccessSpecifier specifier)
{

    QString memberText = memberInfo + ";";
    // clang-format off
    const auto queryString = QString(R"EOF(
        (field_declaration_list
            (access_specifier "%1") @access
            . [(declaration) (comment) (field_declaration)]* @field
        )
    )EOF").arg(accessSpecifierMap.value(specifier));
    // clang-format on

    const auto range = queryClassDefinition(className).get("body");
    if (!range.isValid()) {
        return MemberOrMethodAdditionResult::ClassNotFound;
    }

    auto result = queryInRange(range, queryString);
    if (!result.isEmpty()) {
        const auto &match = result.last();
        const auto fields = match.getAll("field");
        if (!fields.isEmpty()) {
            const auto &pos = fields.last();
            const auto indent = indentationAtPosition(pos.end());
            insertAtPosition("\n" + indent + memberText, pos.end());
        } else {
            const auto access = match.getAll("access");
            const auto &pos = access.last();
            const auto indent = indentationAtPosition(pos.end());
            insertAtPosition("\n" + indent + memberText, pos.end());
        }
    } else {
        const bool check = addSpecifierSection(memberText, className, specifier);
        if (!check) {
            return MemberOrMethodAdditionResult::ClassNotFound;
        }
    }

    return MemberOrMethodAdditionResult::Success;
}

/*!
 * \qmlmethod CppDocument::addMember(string member, string className, AccessSpecifier)
 * Adds a new member in a specific class under the specific access specifier.
 *
 * If the class does not exist, log error can't find the class, but if the
 * specifier is valid but does not exist in the class, we will add that specifier in the end of the
 * class and add the member under it.
 * The specifier can take these values:
 *
 * - `CppDocument.Public`
 * - `CppDocument.Protected`
 * - `CppDocument.Private`
 */
bool CppDocument::addMember(const QString &member, const QString &className, AccessSpecifier specifier)
{
    LOG("CppDocument::addMember", member, className, specifier);

    auto result = addMemberOrMethod(member, className, specifier);
    if (result == MemberOrMethodAdditionResult::ClassNotFound) {
        spdlog::error(R"(CppDocument::addMember- Can't find class '{}')", className);
    }

    return true;
}

/*!
 * \qmlmethod CppDocument::addMethodDeclaration(string method, string className, AccessSpecifier specifier)
 * Declares a new method in a specific class under the specific access specifier.
 *
 * If the class does not exist, log error can't find the class, but if the
 * specifier is valid but does not exist in the class, we will add that specifier in the end of the
 * class and declare the method under it.
 * The specifier can take these values:
 *
 * - `CppDocument.Public`
 * - `CppDocument.Protected`
 * - `CppDocument.Private`
 */
bool CppDocument::addMethodDeclaration(const QString &method, const QString &className, AccessSpecifier specifier)
{
    LOG("CppDocument::addMethodDeclaration", method, className, specifier);

    auto result = addMemberOrMethod(method, className, specifier);
    if (result == MemberOrMethodAdditionResult::ClassNotFound) {
        spdlog::error(R"(CppDocument::addMethodDeclaration - Can't find class '{}')", className);
    }

    return true;
}

/*!
 * \qmlmethod CppDocument::addMethodDefinition(string method, string className)
 * \qmlmethod CppDocument::addMethodDefinition(string method, string className, string body)
 *
 * Adds a new method definition for the method declared by the given `method` for
 * class `className` in the current file.
 * The provided `body` should not include the curly braces.
 *
 * If no body is provided, it will default to an empty body.
 */
bool CppDocument::addMethodDefinition(const QString &method, const QString &className, const QString &body /*= ""*/)
{
    LOG("CppDocument::addMethodDefinition", method, className);

    QString definition = method;

    // Remove declaration specific modifiers to make the parameter compatible with addMethodDeclaration
    const static QStringList modifiers = {"override",    "final",  "virtual", "static",
                                          "Q_INVOKABLE", "Q_SLOT", "Q_SIGNAL"};
    for (const auto &modifier : modifiers) {
        definition.remove(modifier);
    }
    definition = definition.simplified();

    // Extract the return type and method name
    int openParenIdx = definition.indexOf('(');
    int spaceIdx = definition.lastIndexOf(' ', openParenIdx);

    QString returnType = definition.left(spaceIdx);
    QString methodName = definition.mid(spaceIdx + 1, openParenIdx - spaceIdx - 1);

    // Construct the method definition
    QString methodDef = QString("%1 %2::%3").arg(returnType, className, methodName);
    QString fullBody = body.isEmpty() ? " {}" : QString(" {\n%1\n}").arg(body);
    methodDef += definition.mid(openParenIdx) + fullBody;

    QString indent = "\n\n";

    auto lastBracePos = textEdit()->toPlainText().lastIndexOf('}');

    QTextCursor cursor = textEdit()->textCursor();
    cursor.beginEditBlock();

    cursor.setPosition(lastBracePos + 1);
    cursor.movePosition(QTextCursor::EndOfBlock);

    // Add the method definition
    cursor.insertText(indent + methodDef);
    auto methodStartPos = textEdit()->toPlainText().lastIndexOf('{');
    cursor.setPosition(methodStartPos + 1); // move to position after opening brace
    cursor.endEditBlock();

    textEdit()->setTextCursor(cursor);
    return true;
}

/*!
 * \qmlmethod CppDocument::addMethod(string declaration, string className, AccessSpecifier, string body)
 * \qmlmethod CppDocument::addMethod(string declaration, string className, AccessSpecifier)
 *
 * Declares and defines a new method.
 * This method can be called on either the header or source file.
 * It will find the corresponding header/source file and add the declaration
 * to the header and the definition to the source.
 *
 * \sa addMethodDeclaration
 * \sa addMethodDefinition
 */
bool CppDocument::addMethod(const QString &declaration, const QString &className, AccessSpecifier specifier,
                            const QString &body /*= ""*/)
{
    auto header = this;
    auto source = openHeaderSource();

    if (!isHeader()) {
        std::swap(header, source);
    }

    auto result = true;

    if (header) {
        result &= header->addMethodDeclaration(declaration, className, specifier);
    } else {
        spdlog::error("CppDocument::addMethod - Can't find header file for '{}'", className);
    }

    if (source) {
        result &= source->addMethodDefinition(declaration, className, body);
    } else {
        spdlog::error("CppDocument::addMethod - Can't find source file for '{}'", className);
    }

    return result;
}

bool CppDocument::addSpecifierSection(const QString &memberText, const QString &className, AccessSpecifier specifier)
{

    // clang-format off
    const auto queryString = QString(R"EOF(
        (field_declaration_list
            (_)@pos
        )
    )EOF");
    // clang-format on

    auto range = queryClassDefinition(className).get("body");
    auto result = queryInRange(range, queryString);

    if (!result.isEmpty()) {
        const auto &match = result.last();
        const auto pos = match.get("pos");
        const auto indent = indentationAtPosition(pos.end());

        const QString newSpecifier = QString("\n\n%1:").arg(accessSpecifierMap.value(specifier));

        insertAtPosition(newSpecifier + "\n" + indent + memberText, pos.end());
    } else {
        // The class specifier is invalid
        return false;
    }
    return true;
}

/*!
 * \qmlmethod CppDocument::removeInclude(string include)
 * Remove `include` from the file. If the include is not in the file, do nothing (and returns true).
 *
 * The `include` string should be either `<foo.h>` or `"foo.h"`, it will returns false otherwise.
 */
bool CppDocument::removeInclude(const QString &include)
{
    LOG("CppDocument::removeInclude", LOG_ARG("text", include));

    IncludeHelper includeHelper(this);
    auto line = includeHelper.includePositionForRemoval(include);
    if (!line) {
        spdlog::error(R"(CppDocument::removeInclude - the include '{}' is malformed, should be '<foo.h>' or '"foo.h"')",
                      include);
        return false;
    }

    if (line.value() == -1) {
        spdlog::info("CppDocument::removeInclude - the include '{}' is not included.");
        return true;
    }

    deleteLine(line.value());
    return true;
}

/**
 * Delete the fully qualified method with the given signature.
 * If signature is empty, will delete all overloads.
 * The method will only be deleted within this document, no other documents will be modified.
 */
void CppDocument::deleteMethodLocal(const QString &methodName, const QString &signature)
{
    auto doesNotMatchMethod = [&methodName, &signature](const auto &symbol) {
        const auto isFunction = symbol->isFunction();

        if (signature.isEmpty())
            return !isFunction || symbol->name() != methodName;
        else
            return !isFunction || symbol->name() != methodName || symbol->toFunction()->signature() != signature;
    };

    auto symbolList = symbols();
    kdalgorithms::erase_if(symbolList, doesNotMatchMethod);
    if (symbolList.empty())
        return;

    // Sort the symbols so that we remove them end-to-start
    // That way removing a function won't change the position of the other functions.
    // This assumes the ranges don't overlap.
    auto byRange = [](const auto &symbol1, const auto &symbol2) {
        return symbol1->range().start > symbol2->range().start;
    };
    std::ranges::sort(symbolList, byRange);

    for (const auto &symbol : std::as_const(symbolList)) {
        spdlog::trace("CppDocument::deleteMethodLocal: Removing symbol '{}'", symbol->name());

        deleteSymbol(*symbol);
    }
}

/*!
 * \qmlmethod void CppDocument::deleteMethod(string method, string signature)
 *
 * Delete the method or function with the specified `method` and optional `signature`.
 * The method definition/declaration will be deleted from the current file,
 * as well as the corresponding header/source file.
 * References to the method will not be deleted.
 *
 * The `method` must be fully qualified, i.e. "<Namespaces>::<Class>::<Method>".
 *
 * The `signature` must be in the form: "<return type> (<first parameter type>, <second parameter type>, <...>)".
 * i.e. for a function with the following declaration:
 *
 * ``` cpp
 * void myFunction(const QString& a, int b);
 * ```
 *
 * The `signature` would be:
 *
 * ``` cpp
 * void (const QString&, int)
 * ```
 *
 * If an empty string is provided as the `signature`, all overloads of the function are deleted as well.
 */
void CppDocument::deleteMethod(const QString &method, const QString &signature)
{
    LOG("CppDocument::deleteMethod", method, signature);

    QString headerSourceName = correspondingHeaderSource();
    if (!headerSourceName.isEmpty()) {
        auto headerSource = qobject_cast<CppDocument *>(Project::instance()->get(headerSourceName));
        headerSource->deleteMethodLocal(method, signature);
    }
    deleteMethodLocal(method, signature);
}

/*!
 * \qmlmethod void CppDocument::deleteMethod(string methodName)
 *
 * Deletes a method of the specified `methodName`, without matching a specific `signature`.
 * Therefore, all overloads of the function will be deleted.
 *
 * Also see: CppDocument::deleteMethod(string methodName, string signature)
 */
void CppDocument::deleteMethod(const QString &methodName)
{
    LOG("CppDocument::deleteMethod", LOG_ARG("text", methodName));

    deleteMethod(methodName, "" /*empty string means ignore signature*/);
}

/*!
 * \qmlmethod void CppDocument::deleteMethod()
 *
 * Deletes the method/function at the current cursor position.
 * Overloads of the function will not be deleted!
 *
 * Also see: CppDocument::deleteMethod(const QString& methodName, const QString& signature)
 */
void CppDocument::deleteMethod()
{
    LOG("CppDocument::deleteMethod");

    auto isFunction = [](const auto &symbol) {
        return symbol.isFunction();
    };

    auto symbol = currentSymbol(isFunction);

    if (!symbol) {
        spdlog::error(
            "CppDocument::deleteMethod: Cursor is not currently within a function definition or declaration!");
    } else {
        deleteMethod(symbol->name(), symbol->toFunction()->signature());
    }
}

void CppDocument::changeBaseClassForwardInclude(const QString &originalClassBaseName, const QString &newClassBaseName)
{
    replaceAllRegexp(QString("#include.s*<%1.h>").arg(originalClassBaseName),
                     QString("#include <%1.h>").arg(newClassBaseName));
    replaceAllRegexp(QString("#include.s*<%1>").arg(originalClassBaseName),
                     QString("#include <%1>").arg(newClassBaseName));
    replaceAllRegexp(QString("class %1;").arg(originalClassBaseName), QString("class %1;").arg(newClassBaseName));
}

bool CppDocument::changeBaseClassHeader(const QString &className, const QString &originalClassBaseName,
                                        const QString &newClassBaseName)
{
    Q_UNUSED(className);
    changeBaseClassForwardInclude(originalClassBaseName, newClassBaseName);

    queryClassDefinition(className).get("base").replace(newClassBaseName);
    return true;
}

bool CppDocument::changeBaseClassSource(const QString &className, const QString &originalClassBaseName,
                                        const QString &newClassBaseName)
{
    Q_UNUSED(className);

    changeBaseClassForwardInclude(originalClassBaseName, newClassBaseName);
    replaceAll(QString("%1::").arg(originalClassBaseName), QString("%1::").arg(newClassBaseName));
    const auto constructors = queryMethodDefinition(className, className);
    for (const auto &constructor : constructors) {
        const auto constructorRange = constructor.get("definition");
        const auto bodyRange = constructor.get("body");
        const auto newRange = RangeMark(this, constructorRange.start(), bodyRange.start());
        replaceAllInRange(originalClassBaseName, newClassBaseName, newRange);
    }
    return true;
}

bool CppDocument::changeBaseClass(CppDocument *header, CppDocument *source, const QString &className,
                                  const QString &newClassBaseName)
{
    auto result = true;
    const QString baseClassName = header->queryClassDefinition(className).get("base").text();
    if (baseClassName.isEmpty()) {
        spdlog::error("CppDocument::changeBaseClass - Can't find base class name for class: '{}'",
                      className.toStdString());
        return false;
    }
    if (header) {
        result &= header->changeBaseClassHeader(className, baseClassName, newClassBaseName);
    } else {
        spdlog::warn("CppDocument::changeBaseClass - Can't find header file for '{}'", baseClassName);
        result = false;
    }
    if (source) {
        result &= source->changeBaseClassSource(className, baseClassName, newClassBaseName);
    } else {
        spdlog::warn("CppDocument::changeBaseClass - Can't find source file for '{}'", baseClassName);
        result = false;
    }
    return result;
}

/*!
 * \qmlmethod void CppDocument::changeBaseClass()
 *
 * Convert Class Base
 *
 * Also see: CppDocument::changeBaseClass(const QString &className, const QString &originalClassBaseName, const QString
 * &newClassBaseName)
 */
bool CppDocument::changeBaseClass(const QString &className, const QString &newClassBaseName)
{
    LOG("CppDocument::changeBaseClass");

    auto header = this;
    auto source = openHeaderSource();
    if (!source) {
        spdlog::warn("CppDocument::changeBaseClass - Can't find source file");
    } else if (!isHeader()) {
        std::swap(header, source);
    }

    return changeBaseClass(header, source, className, newClassBaseName);
}

/*!
 * \qmlmethod QStringList CppDocument::keywords()
 * Returns a list of cpp keywords
 *
 * See: Utils::cppKeywords()
 */
QStringList CppDocument::keywords() const
{
    return Utils::cppKeywords();
}

/*!
 * \qmlmethod QStringList CppDocument::primitiveTypes()
 * Returns a list of cpp primitive types
 *
 * See Utils::cppPrimitiveTypes()
 */
QStringList CppDocument::primitiveTypes() const
{
    return Utils::cppPrimitiveTypes();
}

} // namespace Core
