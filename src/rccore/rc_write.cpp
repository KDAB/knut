#include "rcfile.h"

#include "uiwriter.h"

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QIODevice>
#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QSet>
#include <QXmlStreamWriter>
#include <QtQml/private/qqmlengine_p.h>

#include <spdlog/spdlog.h>
namespace RcCore {

//=============================================================================
// Asset writing
//=============================================================================
static QImage convertBmpImage(const Asset &asset, Asset::TransparentColors colors)
{
    QImage image(asset.originalFileName);

    QSet<QRgb> transparentColors;
    if (image.format() != QImage::Format_ARGB32) {
        if (colors & Asset::Gray)
            transparentColors.insert(qRgb(192, 192, 192));
        if (colors & Asset::Magenta)
            transparentColors.insert(qRgb(255, 0, 255));
        if (colors & Asset::BottomLeftPixel)
            transparentColors.insert(image.pixel(0, image.height() - 1));
    }

    image = image.convertToFormat(QImage::Format_ARGB32);
    if (!transparentColors.isEmpty()) {
        for (int x = 0; x < image.width(); ++x) {
            for (int y = 0; y < image.height(); ++y) {
                if (transparentColors.contains(image.pixel(x, y)))
                    image.setPixel(x, y, qRgba(0, 0, 0, 0));
            }
        }
    }
    return image;
}

/**
 * @brief Write new images for assets
 * Used if there's a BMP->PNG conversion, or toolbar splitting (default).
 * @param assets list of assets
 * @param colors list of transparent colors for the conversion
 */
void writeAssetsToImage(const QVector<Asset> &assets, Asset::TransparentColors colors)
{
    QHash<QString, QImage> cache;
    for (const auto &asset : assets) {
        if (!asset.exist)
            continue;

        if (asset.isSame())
            continue;

        // Write BMP -> PNG conversion
        if (asset.iconRect.isNull()) {
            convertBmpImage(asset, colors).save(asset.fileName);

            // Write BMP -> PNG for split toolbars
        } else {
            QImage image = cache.value(asset.originalFileName);
            if (image.isNull()) {
                image = convertBmpImage(asset, colors);
                cache[asset.originalFileName] = image;
            }
            image.copy(asset.iconRect).save(asset.fileName);
        }
    }
}

/**
 * @brief Write a qrc file based on a list of assets
 * @param assets list of assets to write
 * @param device device to write on
 * @param fileName fileName of the qrc file (to have relative path to it)
 */
void writeAssetsToQrc(const QVector<Asset> &assets, QIODevice *device, const QString &fileName)
{
    Q_ASSERT(device);

    QXmlStreamWriter w(device);
    w.setAutoFormatting(true);

    w.writeStartElement("RCC");
    w.writeStartElement("qresource");
    w.writeAttribute("prefix", "/");

    const QFileInfo fi(fileName);
    for (const auto &asset : assets) {
        w.writeStartElement("file");
        w.writeAttribute("alias", asset.id);

        // Compute relative filePath for assets
        QString assetFileName = asset.fileName;
        if (asset.exist)
            assetFileName = fi.absoluteDir().relativeFilePath(asset.fileName);

        w.writeCharacters(assetFileName);
        w.writeEndElement();
    }

    w.writeEndElement();
    w.writeEndElement();
}

//=============================================================================
// Dialog writing
//=============================================================================
static void logWarnings(const QList<QQmlError> &warnings)
{
    for (const auto &warning : warnings) {
        if (warning.description().contains("error", Qt::CaseInsensitive))
            spdlog::error("{}({}): {}", warning.url().toLocalFile().toStdString(), warning.line(),
                          warning.description().toStdString());
        else
            spdlog::warn("{}({}): {}", warning.url().toLocalFile().toStdString(), warning.line(),
                         warning.description().toStdString());
    }
}

QString createConversionScript(const QString &scriptPath)
{
    ;
    const auto path = scriptPath.isEmpty() ? "qrc:/rccore/rc2ui.js"
                                           : QUrl::fromLocalFile((QFileInfo(scriptPath).absoluteFilePath())).toString();
    return QStringLiteral("import QtQml 2.12\n"
                          "import \"%1\" as MyScript\n"
                          "QtObject { function runScript(dialog, writer) {"
                          "MyScript.main(dialog, writer);"
                          "}}")
        .arg(path);
}

void writeDialogToUi(const Widget &widget, QIODevice *device, const QString &scriptPath)
{
    const QString script = createConversionScript(scriptPath);

    QQmlEngine engine;
    QObject::connect(&engine, &QQmlEngine::warnings, logWarnings);
    engine.setOutputWarningsToStandardError(false);

    QQmlComponent component(&engine);
    component.setData(script.toLatin1(), {});
    auto *result = qobject_cast<QObject *>(component.create());

    if (component.isReady() && !component.isError()) {
        UiWriter writer(device);
        QMetaObject::invokeMethod(result, "runScript", Qt::DirectConnection,
                                  Q_ARG(QVariant, QVariant::fromValue(widget)),
                                  Q_ARG(QVariant, QVariant::fromValue(&writer)));
    }
}

} // namespace RcCore
