#include "cppdocument.h"

#include "project.h"
#include "settings.h"

#include "lsp/client.h"
#include "lsp/types.h"

#include <QFileInfo>
#include <QHash>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QVariantMap>
#include <spdlog/spdlog.h>

#include <ranges>

namespace Core {

/*!
 * \qmltype Symbol
 * \brief Represent a symbol in the current file
 * \instantiates Core::Symbol
 * \inqmlmodule Script
 * \since 4.0
 */

/*!
 * \qmlproperty string Symbol::name
 * Return the name of this symbol.
 */
/*!
 * \qmlproperty string Symbol::description
 * Return more detail for this symbol, e.g the signature of a function.
 */
/*!
 * \qmlproperty Kind Symbol::kind
 * Return the kind of this symbol. Available symbole kinds are:
 *
 * - `Symbol.File`
 * - `Symbol.Module`
 * - `Symbol.Namespace`
 * - `Symbol.Package`
 * - `Symbol.Class`
 * - `Symbol.Method`
 * - `Symbol.Property`
 * - `Symbol.Field`
 * - `Symbol.Constructor`
 * - `Symbol.Enum`
 * - `Symbol.Interface`
 * - `Symbol.Function`
 * - `Symbol.Variable`
 * - `Symbol.Constant`
 * - `Symbol.String`
 * - `Symbol.Number`
 * - `Symbol.Boolean`
 * - `Symbol.Array`
 * - `Symbol.Object`
 * - `Symbol.Key`
 * - `Symbol.Null`
 * - `Symbol.EnumMember`
 * - `Symbol.Struct`
 * - `Symbol.Event`
 * - `Symbol.Operator`
 * - `Symbol.TypeParameter`
 */
/*!
 * \qmlproperty TextRange Symbol::range
 * The range enclosing this symbol not including leading/trailing whitespace
 * but everything else like comments. This information is typically used to
 * determine if the clients cursor is inside the symbol to reveal in the
 * symbol in the UI.
 */
/*!
 * \qmlproperty TextRange Symbol::selectionRange
 * The range that should be selected and revealed when this symbol is being
 * picked, e.g. the name of a function. Must be contained by the `range`.
 */

/*!
 * \qmltype CppDocument
 * \brief Document object for a C++ file (source or header)
 * \instantiates Core::CppDocument
 * \inqmlmodule Script
 * \since 4.0
 * \inherits LspDocument
 */

/*!
 * \qmlproperty bool CppDocument::isHeader
 * Return true if the current document is a header.
 */

CppDocument::CppDocument(QObject *parent)
    : LspDocument(Type::Cpp, parent)
{
}

static bool isHeaderSuffix(const QString &suffix)
{
    // Good enough for now, headers starts with h or hpp
    return suffix.startsWith('h');
}

bool CppDocument::isHeader() const
{
    QFileInfo fi(fileName());
    return isHeaderSuffix(fi.suffix());
}

static QStringList matchingSuffixes(bool header)
{
    static const char MimeTypes[] = "/mime_types";
    static const auto mimeTypes = Settings::instance()->value<std::map<std::string, Document::Type>>(MimeTypes);

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
    int length = qMin(s1.length(), s2.length());
    for (int i = 0; i < length; ++i) {
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
    static QHash<QString, QString> cache;

    QString cacheData = cache.value(fileName());
    if (!cacheData.isEmpty())
        return cacheData;

    const bool header = isHeader();
    const QStringList suffixes = matchingSuffixes(header);

    QFileInfo fi(fileName());
    QStringList candidates = candidateFileNames(fi.completeBaseName(), suffixes);

    // Search in the current directory
    for (const auto &candidate : candidates) {
        const QString testFileName = fi.absolutePath() + '/' + candidate;
        if (QFile::exists(testFileName)) {
            cache[fileName()] = testFileName;
            cache[testFileName] = fileName();
            spdlog::trace("CppDocument::correspondingHeaderSource {} => {}", fileName().toStdString(),
                          testFileName.toStdString());
            return testFileName;
        }
    }

    // Search in the whole project, and find the possible files
    QStringList fullPathNames = Project::instance()->allFilesWithExtensions(suffixes, Project::FullPath);
    auto checkIfPathNeedToBeRemoved = [&](const auto &path) {
        for (const auto &fileName : candidates) {
            if (path.endsWith(fileName, Qt::CaseInsensitive))
                return false;
        }
        return true;
    };
    fullPathNames.erase(std::remove_if(fullPathNames.begin(), fullPathNames.end(), checkIfPathNeedToBeRemoved),
                        fullPathNames.end());

    // Find the file having the most common path with fileName
    QString bestFileName;
    int compareValue = 0;
    for (const auto &path : fullPathNames) {
        int value = commonFilePathLength(path, fileName());
        if (value > compareValue) {
            compareValue = value;
            bestFileName = path;
        }
    }

    if (!bestFileName.isEmpty()) {
        cache[fileName()] = bestFileName;
        cache[bestFileName] = fileName();
        spdlog::trace("CppDocument::correspondingHeaderSource {} => {}", fileName().toStdString(),
                      bestFileName.toStdString());
        return bestFileName;
    }

    spdlog::warn("CppDocument::correspondingHeaderSource {} - not found ", fileName().toStdString());
    return {};
}

/*!
 * \qmlmethod CppDocument CppDocument::openHeaderSource()
 * Opens the corresponding source or header files, the current document is the new file.
 * If no files have been found, it's a no-op.
 */
CppDocument *CppDocument::openHeaderSource()
{
    spdlog::trace("CppDocument::openHeaderSource {}", fileName().toStdString());
    const QString fileName = correspondingHeaderSource();
    if (!fileName.isEmpty())
        return dynamic_cast<CppDocument *>(Project::instance()->open(fileName));
    return nullptr;
}

/*!
 * \qmlmethod void CppDocument::insertForwardDeclaration(string fwddecl)
 * Inserts the forward declaration `fwddecl` into the current file.
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
bool CppDocument::insertForwardDeclaration(const QString &fwddecl)
{
    spdlog::trace("CppDocument::insertForwardDeclaration {}", fwddecl.toStdString());
    if (!isHeader()) {
        spdlog::warn("CppDocument::insertForwardDeclaration: {} - is not a header file. ", fileName().toStdString());
        return false;
    }

    int spacePos = fwddecl.indexOf(' ');
    auto classOrStruct = fwddecl.leftRef(spacePos);
    if (fwddecl.isEmpty() || (classOrStruct != "class" && classOrStruct != "struct")) {
        spdlog::warn("CppDocument::insertForwardDeclaration: {} - should start with 'class ' or 'struct '. ",
                     fwddecl.toStdString());
        return false;
    }

    auto qualifierList = fwddecl.midRef(spacePos + 1).split("::");
    std::ranges::reverse(qualifierList);

    // Get the un-qualified declaration
    QString result = QString("%1 %2;").arg(classOrStruct).arg(qualifierList.first());
    qualifierList.pop_front();

    // Check if the declaration already exists
    QTextDocument *doc = textEdit()->document();
    QTextCursor cursor(doc);
    cursor = doc->find(result, cursor, QTextDocument::FindWholeWords);
    if (!cursor.isNull()) {
        spdlog::warn("CppDocument::insertForwardDeclaration: '{}' - already exists in file.", fwddecl.toStdString());
        return false;
    }

    for (const auto &qualifier : std::as_const(qualifierList))
        result = QString("namespace %1 {\n%2\n}").arg(qualifier, result);

    cursor = QTextCursor(doc);
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    cursor = doc->find(QRegularExpression(QStringLiteral(R"(^#include\s*)")), cursor, QTextDocument::FindBackward);
    if (!cursor.isNull()) {
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
        cursor.insertText("\n" + result + "\n");
        cursor.endEditBlock();
        return true;
    }

    return false;
}

/*!
 * \qmlmethod map<string, string> CppDocument::mfcExtractDDX()
 * Extract the DDX information from a MFC class.
 *
 * The DDX information gives the mapping between the ID and the member variables in the class.
 */
QVariantMap CppDocument::mfcExtractDDX(const QString &className)
{
    spdlog::trace("CppDocument::mfcExtractDDX {}", className.toStdString());

    QVariantMap map;

    // TODO: use semantic information coming from LSP instead of regexp to find the method

    const QString source = text();
    const QRegularExpression searchFunctionExpression(QString(R"*(void\s*%1\s*::DoDataExchange\s*\()*").arg(className),
                                                      QRegularExpression::MultilineOption);
    QRegularExpressionMatch match = searchFunctionExpression.match(source);

    if (match.hasMatch()) {
        const int capturedStart = match.capturedStart(0);
        const int capturedEnd = match.capturedEnd(0);
        int bracketCount = 0;
        int positionEnd = -1;
        for (int i = capturedEnd; i < source.length(); ++i) {
            if (source.at(i) == QLatin1Char('{')) {
                bracketCount++;
            } else if (source.at(i) == QLatin1Char('}')) {
                bracketCount--;
                if (bracketCount == 0) {
                    positionEnd = i;
                    break;
                }
            }
        }

        if (positionEnd == -1)
            return {};

        const QString ddxText = source.mid(capturedStart, (positionEnd - capturedStart + 1));
        static const QRegularExpression doDataExchangeExpression(R"*(DDX_.*\(.*,\s*(.*)\s*,\s*(.*)\))*");
        QRegularExpressionMatchIterator userIteratorWidgetExpression = doDataExchangeExpression.globalMatch(ddxText);
        while (userIteratorWidgetExpression.hasNext()) {
            const QRegularExpressionMatch match = userIteratorWidgetExpression.next();
            map.insert(match.captured(1), match.captured(2));
        }
    }
    return map;
}

} // namespace Core
