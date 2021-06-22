#pragma once

#include "data.h"

#include <QStringList>

class QIODevice;

namespace RcCore {

// Parse method
Data parse(const QString &fileName);

// Conversion methods
QVector<Asset> convertAssets(const Data &data, Asset::ConversionFlags flags = Asset::AllFlags);

Widget convertDialog(const Data &data, const Data::Dialog &dialog,
                     Widget::ConversionFlags flags = Widget::UpdateGeometry, double scaleX = 1.5, double scaleY = 1.65);

QVector<Action> convertActions(const Data &data, const QStringList &menus, const QStringList &accelerators = {},
                               const QStringList &toolBars = {}, Asset::ConversionFlags flags = Asset::AllFlags);

// Write methods
void writeAssetsToImage(const QVector<Asset> &assets, Asset::TransparentColors colors = Asset::AllColors);

void writeAssetsToQrc(const QVector<Asset> &assets, QIODevice *device, const QString &fileName);

void writeDialogToUi(const Widget &widget, QIODevice *device);

} // namespace RcCore
