#include "rcfile.h"

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QIODevice>
#include <QImage>
#include <QSet>
#include <QXmlStreamWriter>

namespace RcCore {

//=============================================================================
// Asset writing
//=============================================================================
static QImage convertBmpImage(Asset asset, Asset::TransparentColors colors)
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
        QString fileName = asset.fileName;
        if (asset.exist)
            fileName = fi.absoluteDir().relativeFilePath(asset.fileName);

        w.writeCharacters(fileName);
        w.writeEndElement();
    }

    w.writeEndElement();
    w.writeEndElement();
}

//=============================================================================
// Dialog writing
//=============================================================================
static void writePropery(QXmlStreamWriter &w, const QString &id, const QString &name, const QVariant &value)
{
    switch (static_cast<QMetaType::Type>(value.typeId())) {
    case QMetaType::Bool:
        w.writeStartElement("property");
        w.writeAttribute("name", name);
        w.writeTextElement("bool", value.toBool() ? "true" : "false");
        w.writeEndElement();
        break;
    case QMetaType::Int:
        w.writeStartElement("property");
        w.writeAttribute("name", name);
        w.writeTextElement("number", QString::number(value.toInt()));
        w.writeEndElement();
        break;
    case QMetaType::QString: {
        w.writeStartElement("property");
        w.writeAttribute("name", name);
        const auto text = value.toString();
        if (name == "alignment") {
            w.writeTextElement("set", text);
        } else if (name == "text") {
            w.writeStartElement("string");
            w.writeAttribute("comment", id);
            w.writeCharacters(text);
            w.writeEndElement();
        } else {
            if (text.contains("::") && !text.contains(' '))
                w.writeTextElement("enum", text);
            else
                w.writeTextElement("string", text);
        }
        w.writeEndElement();
        break;
    }
    case QMetaType::QStringList: {
        const auto values = value.toStringList();
        for (const auto &text : values) {
            w.writeStartElement("item");
            w.writeStartElement("property");
            w.writeAttribute("name", name);
            w.writeTextElement("string", text);
            w.writeEndElement();
            w.writeEndElement();
        }
        break;
    }
    default:
        Q_UNREACHABLE();
    }
}

static void writeWidget(QXmlStreamWriter &w, const Widget &widget, int &staticCount)
{
    w.writeStartElement("widget");
    w.writeAttribute("class", widget.className);

    QString id = widget.id;
    if (widget.id == "IDC_STATIC") {
        if (staticCount)
            id += QString::number(staticCount);
        ++staticCount;
    }
    w.writeAttribute("name", id);

    {
        w.writeStartElement("property");
        w.writeAttribute("name", "geometry");
        w.writeStartElement("rect");
        w.writeTextElement("x", QString::number(widget.geometry.x()));
        w.writeTextElement("y", QString::number(widget.geometry.y()));
        w.writeTextElement("width", QString::number(widget.geometry.width()));
        w.writeTextElement("height", QString::number(widget.geometry.height()));

        w.writeEndElement();
        w.writeEndElement();
    }

    auto itEnd = widget.properties.constEnd();
    for (auto it = widget.properties.constBegin(); it != itEnd; ++it)
        writePropery(w, id, it.key(), it.value());

    const bool isMainWindow = (widget.className == "QMainWindow");
    if (isMainWindow) {
        w.writeStartElement("widget");
        w.writeAttribute("class", "QWidget");
        w.writeAttribute("name", "centralwidget");
    }

    for (const auto &child : widget.children)
        writeWidget(w, child, staticCount);

    if (isMainWindow)
        w.writeEndElement();

    w.writeEndElement();
}

void writeDialogToUi(const Widget &widget, QIODevice *device)
{
    QXmlStreamWriter w(device);

    w.setAutoFormatting(true);
    w.writeStartDocument();

    w.writeStartElement("ui");
    w.writeAttribute("version", "4.0");

    w.writeTextElement("class", widget.id);
    int staticCount = 0;
    writeWidget(w, widget, staticCount);
    w.writeEmptyElement("resources");
    w.writeEmptyElement("connections");

    w.writeEndElement();
}

} // namespace RcCore
