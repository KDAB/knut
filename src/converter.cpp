#include "converter.h"

#include "document.h"

#include <QLoggingCategory>
#include <QJsonObject>
#include <QJsonArray>
#include <QFont>

Q_LOGGING_CATEGORY(converter, "converter")

namespace {
static const auto KeyCaption = QStringLiteral("caption");
static const auto KeyChildren = QStringLiteral("children");
static const auto KeyFont = QStringLiteral("font");
static const auto KeyGeometry = QStringLiteral("geometry");
static const auto KeyId = QStringLiteral("id");
static const auto KeyStyle = QStringLiteral("style");
static const auto KeyType = QStringLiteral("type");

static const auto KeyWeight = QStringLiteral("weight");
static const auto KeyCharset = QStringLiteral("charset");
}

static QJsonObject convertWidget(const Document &doc, QJsonObject widget, const QString &id)
{
    return widget;
}

static QJsonObject convertFont(QJsonObject font, const QString &id)
{
    // Remove charset - Unused for now
    if (font.contains(KeyCharset)) {
        auto charset = font.take(KeyCharset).toString();
        qCDebug(converter)
            << QObject::tr("%1: Unused charset %2").arg(id).arg(charset);
    }

    // Transform typeface into family
    font[QStringLiteral("family")] = font.take("typeface").toString();

    // Transform weight
    if (font.contains(KeyWeight)) {
        switch (font.value(KeyWeight).toInt()) {
        case 100:
        case 200:
        case 300:
            font[KeyWeight] = QFont::Light; break;
        case 500:
        case 600:
            font[KeyWeight] = QFont::DemiBold; break;
        case 700:
            font[KeyWeight] = QFont::Bold; break;
        case 800:
            font[KeyWeight] = QFont::Black; break;
        case 0:
        case 400:
        default:
            // Normal weight - we can remove
            font.remove(KeyWeight);
        }
    }

    return font;
}

QJsonObject convertDialog(const Document &doc, QJsonObject dialog)
{
    auto id = dialog.value(KeyId).toString();

    qCDebug(converter) << QObject::tr("=== %1 ===").arg(id);

    // Sanity check
    auto type = dialog.value(KeyType).toString();
    if (type != "DIALOG" && type != "DIALOGEX") {
        qCWarning(converter)
            << QObject::tr("%1: Don't know type %2").arg(id).arg(type);
    }

    dialog["class"] = "QDialog";
    dialog.remove(KeyType);

    // Caption conversion
    if (dialog.contains(KeyCaption))
        dialog["windowTitle"] = dialog.take(KeyCaption).toString();

    // Font conversion
    if (dialog.contains(KeyFont)) {
        auto font = dialog.value(KeyFont).toObject();
        dialog[KeyFont] = convertFont(font, id);
    }

    // Widget conversion
    auto children = dialog.value(KeyChildren).toArray();
    for (int i = 0; i < children.count(); ++i) {
        auto widget = children.at(i).toObject();
        children[i] = convertWidget(doc, widget, id);
    }
    dialog[KeyChildren] = children;

    // Remove Style - Unused for now
    if (dialog.contains(KeyStyle)) {
        // Remove known styles
        auto styles = dialog.value(KeyStyle).toVariant().toStringList();
        styles.removeOne("WS_CAPTION");
        styles.removeOne("DS_SETFONT");
        styles.removeOne("DS_SHELLFONT");

        if (!styles.isEmpty()) {
            qCDebug(converter)
                << QObject::tr("%1: Unused styles: %2")
                    .arg(id).arg(styles.join(" | "));
        }
    }

    return dialog;
}
