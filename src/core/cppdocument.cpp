#include "cppdocument.h"

#include "project.h"
#include "settings.h"

#include <spdlog/spdlog.h>

#include <QFileInfo>
#include <QHash>

namespace Core {

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
 * \qmlmethod string correspondingHeaderSource()
 * Returns the corresponding source or header file path.
 */
QString CppDocument::correspondingHeaderSource() const
{
    static QHash<QString, QString> cache;

    const QString cacheData = cache.value(fileName());
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
            spdlog::trace("Cpp Document Corresponding Header/Source setting {} => {}", fileName().toStdString(),
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
        spdlog::trace("Cpp Document Corresponding Header/Source setting {} => {}", fileName().toStdString(),
                      bestFileName.toStdString());
        return bestFileName;
    }

    return {};
}

/*!
 * \qmlmethod CppDocument openHeaderSource()
 * Opens the corresponding source or header files, the current document is the new file.
 * If no files have been found, it's a no-op.
 */
CppDocument *CppDocument::openHeaderSource()
{
    const QString fileName = correspondingHeaderSource();
    if (!fileName.isEmpty())
        return dynamic_cast<CppDocument *>(Project::instance()->open(fileName));
    return nullptr;
}

} // namespace Core
