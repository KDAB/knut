#include "cppdocument.h"

#include "project.h"
#include "settings.h"

#include "lsp/client.h"
#include "lsp/types.h"

#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QVariantMap>

#include <spdlog/spdlog.h>

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
 * \inherits TextDocument
 */

/*!
 * \qmlproperty bool CppDocument::isHeader
 * Return true if the current document is a header.
 */

CppDocument::CppDocument(QObject *parent)
    : TextDocument(Type::Cpp, parent)
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
 * \qmlmethod vector<Symbol> CppDocument::symbols()
 * Returns the list of symbols in the current document.
 */
QVector<Core::Symbol> CppDocument::symbols() const
{
    spdlog::trace("CppDocument::symbols");

    if (!client())
        return {};

    // TODO cache the data
    Lsp::DocumentSymbolParams params;
    params.textDocument.uri = toUri();
    auto result = client()->documentSymbol(std::move(params));
    if (!result)
        return {};

    // Wee only supports Lsp::DocumentSymbol for now
    Q_ASSERT(std::holds_alternative<std::vector<Lsp::DocumentSymbol>>(result.value()));
    const auto lspSymbols = std::get<std::vector<Lsp::DocumentSymbol>>(result.value());
    QVector<Symbol> symbols;

    // Create a recusive lambda to flatten the hierarchy
    // Add the full symbol name (with namespaces/classes)
    const std::function<void(const std::vector<Lsp::DocumentSymbol> &, QString)> fillSymbols =
        [this, &symbols, &fillSymbols](const std::vector<Lsp::DocumentSymbol> &lspSymbols, QString context) {
            for (const auto &lspSymbol : lspSymbols) {
                const QString description = lspSymbol.detail ? QString::fromStdString(lspSymbol.detail.value()) : "";
                QString name = QString::fromStdString(lspSymbol.name);
                if (!context.isEmpty())
                    name = context + "::" + name;
                symbols.push_back(Symbol {name, description, static_cast<Core::Symbol::Kind>(lspSymbol.kind),
                                          toRange(lspSymbol.range), toRange(lspSymbol.selectionRange)});
                if (lspSymbol.children) {
                    if (lspSymbol.kind == Lsp::SymbolKind::String) // case for BEGIN_MESSAGE_MAP
                        fillSymbols(lspSymbol.children.value(), context);
                    else
                        fillSymbols(lspSymbol.children.value(), name);
                }
            }
        };
    fillSymbols(lspSymbols, "");

    return symbols;
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
