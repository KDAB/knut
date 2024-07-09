/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "rcfile.h"
#include "utils/log.h"
#include "utils/qtuiwriter.h"

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
void writeAssetsToImage(const QList<Asset> &assets, Asset::TransparentColors colors)
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
void writeAssetsToQrc(const QList<Asset> &assets, QIODevice *device, const QString &fileName)
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
static void writeWidget(Utils::QtUiWriter &writer, const Widget &widget, pugi::xml_node parent = {})
{
    auto widgetNode = writer.addWidget(widget.className, widget.id, parent);

    writer.addWidgetProperty(widgetNode, "mfc_id", widget.id, {{"notr", "true"}}, false);
    writer.addWidgetProperty(widgetNode, "geometry", widget.geometry);

    for (const auto &property : widget.properties.asKeyValueRange()) {
        const auto &propertyName = property.first;
        if (propertyName == "text")
            writer.addWidgetProperty(widgetNode, property.first, property.second, {{"comment", widget.id}});
        else
            writer.addWidgetProperty(widgetNode, property.first, property.second);
    }

    for (const auto &child : widget.children)
        writeWidget(writer, child, widgetNode);
}

void writeDialogToUi(const Widget &widget, QIODevice *device)
{
    pugi::xml_document doc;
    Utils::QtUiWriter writer(doc);

    writeWidget(writer, widget);

    device->write(writer.dump().toUtf8());
}

} // namespace RcCore
