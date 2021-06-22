#pragma once

#include "data.h"

#include <QStringList>

class QIODevice;

namespace RcCore {

constexpr double defaultDialogScaleX = 1.5;
constexpr double defaultDialogScaleY = 1.65;
constexpr Widget::ConversionFlags defaultDialogFlags = Widget::AllFlags;
constexpr Asset::ConversionFlags defaultAssetFlags = Asset::AllFlags;
constexpr Asset::TransparentColors defaultColorFlags = Asset::AllColors;

// Parse method
Data parse(const QString &fileName);

// Conversion methods
QVector<Asset> convertAssets(const Data &data, Asset::ConversionFlags flags = defaultAssetFlags);

Widget convertDialog(const Data &data, const Data::Dialog &dialog, Widget::ConversionFlags flags = defaultDialogFlags,
                     double scaleX = defaultDialogScaleX, double scaleY = defaultDialogScaleY);

QVector<Action> convertActions(const Data &data, const QStringList &menus, const QStringList &accelerators = {},
                               const QStringList &toolBars = {}, Asset::ConversionFlags flags = defaultAssetFlags);

// Write methods
void writeAssetsToImage(const QVector<Asset> &assets, Asset::TransparentColors colors = defaultColorFlags);

void writeAssetsToQrc(const QVector<Asset> &assets, QIODevice *device, const QString &fileName);

void writeDialogToUi(const Widget &widget, QIODevice *device);

} // namespace RcCore
