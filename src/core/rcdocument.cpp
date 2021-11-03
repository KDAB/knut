#include "rcdocument.h"

#include "rccore/rcfile.h"

#include <QBuffer>
#include <QFile>
#include <QUiLoader>
#include <QWidget>

#include <algorithm>

namespace Core {

/*!
 * \qmltype RcDocument
 * \brief Provides access to the content of a RC file (MFC resource file).
 * \instantiates Core::RcDocument
 * \inqmlmodule Script
 * \since 4.0
 */

/*!
 * \qmlproperty bool RcDocument::valid
 * This read-only property defines if the RC file is valid for our parser.
 *
 * Note that the RC file may be valid, and our parser needs to be updated.
 */
/*!
 * \qmlproperty array<Asset> RcDocument::assets
 * This read-only property holds the list of assets in the RC file.
 */
/*!
 * \qmlproperty array<ToolBar> RcDocument::toolbars
 * This read-only property holds the list of toolbars in the RC file.
 */
/*!
 * \qmlproperty array<Menu> RcDocument::menus
 * This read-only property holds the list of menus in the RC file.
 */
/*!
 * \qmlproperty array<String> RcDocument::strings
 * This read-only property holds the list of strings in the RC file.
 */
/*!
 * \qmlproperty array<string> RcDocument::dialogIds
 * This read-only property holds the list of dialog's ids in the RC file.
 */
/*!
 * \qmlproperty array<string> RcDocument::menuIds
 * This read-only property holds the list of menu's ids in the RC file.
 */
/*!
 * \qmlproperty array<string> RcDocument::acceleratorIds
 * This read-only property holds the list of accelerator's ids in the RC file.
 */
/*!
 * \qmlproperty array<string> RcDocument::toolbarIds
 * This read-only property holds the list of toolbar's ids in the RC file.
 */
/*!
 * \qmlproperty array<string> RcDocument::stringIds
 * This read-only property holds the list of string's ids in the RC file.
 */

RcDocument::RcDocument(QObject *parent)
    : Document(Type::Rc, parent)
{
}

bool RcDocument::isValid() const
{
    return m_data.isValid;
}

QVector<RcCore::Asset> RcDocument::assets() const
{
    if (m_data.isValid) {
        if (m_cacheAssets.isEmpty())
            return m_data.assets;
        return m_cacheAssets;
    }
    return {};
}

QVector<RcCore::ToolBar> RcDocument::toolBars() const
{
    if (m_data.isValid)
        return m_data.toolBars;
    return {};
}

/*!
 * \qmlmethod ToolBar RcDocument::toolBar( string id)
 * Returns the toolbar for the given `id`.
 */
RcCore::ToolBar RcDocument::toolBar(const QString &id) const
{
    if (m_data.isValid) {
        if (auto tb = m_data.toolBar(id))
            return *tb;
    }
    return {};
}

/*!
 * \qmlmethod Widget RcDocument::dialog( string id, int flags, real scaleX, real scaleY)
 * \sa RcDocument::writeDialogToUi
 * Returns the dialog for the given `id`.
 *
 * To do the conversion, the `flags` and scale factor `scaleX` and `scaleY` are used. Default
 * values are coming from the project settings.
 *
 * Flags could be:
 *
 * - `RcDocument.UpdateHierachy`: create a hierarchy of parent-children, by default there are none in MFC
 * - `RcDocument.UpdateGeometry`: use the scale factor to change the dialog size
 * - `RcDocument.UseIdForPixmap`: use the id as a resource value for the pixmaps in labels
 * - `RcDocument.AllFlags`: combination of all above
 */
RcCore::Widget RcDocument::dialog(const QString &id, int flags, double scaleX, double scaleY) const
{
    SET_DEFAULT_VALUE(RcDialogFlags, static_cast<ConversionFlags>(flags));
    SET_DEFAULT_VALUE(RcDialogScaleX, scaleX);
    SET_DEFAULT_VALUE(RcDialogScaleY, scaleY);
    if (m_data.isValid) {
        if (auto dialog = m_data.dialog(id))
            return RcCore::convertDialog(m_data, *dialog, static_cast<RcCore::Widget::ConversionFlags>(flags), scaleX,
                                         scaleY);
    }
    return {};
}

/*!
 * \qmlmethod Menu RcDocument::menu( string id)
 * Returns the menu for the given `id`.
 */
RcCore::Menu RcDocument::menu(const QString &id) const
{
    if (m_data.isValid) {
        if (auto tb = m_data.menu(id))
            return *tb;
    }
    return {};
}

QStringList RcDocument::dialogIds() const
{
    if (m_data.isValid) {
        const auto &dialogs = m_data.dialogs;
        QStringList result;
        result.reserve(dialogs.size());
        std::transform(std::cbegin(dialogs), std::cend(dialogs), std::back_inserter(result), [](const auto &dialog) {
            return dialog.id;
        });
        result.sort();
        return result;
    }
    return {};
}

QStringList RcDocument::menuIds() const
{
    if (m_data.isValid) {
        const auto &menus = m_data.menus;
        QStringList result;
        result.reserve(menus.size());
        std::transform(std::cbegin(menus), std::cend(menus), std::back_inserter(result), [](const auto &menu) {
            return menu.id;
        });
        result.sort();
        return result;
    }
    return {};
}

QStringList RcDocument::acceleratorIds() const
{
    if (m_data.isValid) {
        const auto &accelerators = m_data.acceleratorTables;
        QStringList result;
        result.reserve(accelerators.size());
        std::transform(std::cbegin(accelerators), std::cend(accelerators), std::back_inserter(result),
                       [](const auto &accelerator) {
                           return accelerator.id;
                       });
        result.sort();
        return result;
    }
    return {};
}

QStringList RcDocument::toolbarIds() const
{
    if (m_data.isValid) {
        const auto &toolbars = m_data.toolBars;
        QStringList result;
        result.reserve(toolbars.size());
        std::transform(std::cbegin(toolbars), std::cend(toolbars), std::back_inserter(result), [](const auto &toolbar) {
            return toolbar.id;
        });
        result.sort();
        return result;
    }
    return {};
}

QStringList RcDocument::stringIds() const
{
    if (m_data.isValid)
        return m_data.strings.keys();
    return {};
}

QList<RcCore::String> RcDocument::strings() const
{
    if (m_data.isValid) {
        const auto &strings = m_data.strings;
        return strings.values();
    }
    return {};
}

/*!
 * \qmlmethod string RcDocument::text( string id)
 * Return the string for the given `id`.
 */
QString RcDocument::string(const QString &id) const
{
    if (m_data.isValid)
        return m_data.strings.value(id).text;
    return {};
}

const RcCore::Data &RcDocument::data() const
{
    return m_data;
}

QVector<RcCore::Menu> RcDocument::menus() const
{
    if (m_data.isValid)
        return m_data.menus;
    return {};
}

/*!
 * \qmlmethod RcDocument::convertAssets( int flags)
 * \sa RcDocument::writeAssetsToImage
 * \sa RcDocument::writeAssetsToQrc
 *
 * Convert all assets using the `flags`.
 *
 * - `RcDocument.RemoveUnknown`: remove the unknown assets
 * - `RcDocument.SplitToolBar`: split oolbars strip into individual icon, one per action
 * - `RcDocument.ConvertToPng`: convert BMPs to PNGs, needed if we want to also change the transparency
 * - `RcDocument.AllFlags`: combination of all above
 */
void RcDocument::convertAssets(int flags)
{
    SET_DEFAULT_VALUE(RcAssetFlags, static_cast<ConversionFlags>(flags));
    if (m_data.isValid) {
        m_cacheAssets = RcCore::convertAssets(m_data, static_cast<RcCore::Asset::ConversionFlags>(flags));
        emit fileNameChanged();
    }
}

/*!
 * \qmlmethod array<Action> RcDocument::convertActions( array<string> menus, array<string> accelerators, array<string>
 * toolBars, int flags) Returns a list of actions fomr the given `menus`' ids, `accelerators`' ids and `toolBars`' ids.
 *
 * The `flags` are used to fill the iconPath of the action:
 *
 * - `RcDocument.RemoveUnknown`: remove the unknown assets
 * - `RcDocument.SplitToolBar`: split oolbars strip into individual icon, one per action
 * - `RcDocument.ConvertToPng`: convert BMPs to PNGs, needed if we want to also change the transparency
 * - `RcDocument.AllFlags`: combination of all above
 */
QVector<RcCore::Action> RcDocument::convertActions(const QStringList &menus, const QStringList &accelerators,
                                                   const QStringList &toolBars, int flags)
{
    SET_DEFAULT_VALUE(RcAssetFlags, static_cast<ConversionFlags>(flags));
    if (m_data.isValid)
        return RcCore::convertActions(m_data, menus, accelerators, toolBars,
                                      static_cast<RcCore::Asset::ConversionFlags>(flags));
    return {};
}

/*!
 * \qmlmethod bool RcDocument::writeAssetsToImage( int flags)
 * \sa RcDocument::convertAssets
 * Writes the assets to images, using `flags` for transparency settings. Returns `true` if no issues.
 *
 * Before writing the assets to disk, you first need to convert them using RcDocument::convertAssets.
 * BMPs assets don't have transparency, but some specific colors are used for that in MFC. When
 * writing the assets to PNGs, the `flags` define how to handle transparency:
 *
 * - `RcDocument.NoColors`: no transparent color
 * - `RcDocument.Gray`: rgb(192, 192, 192) is used as a transparent color
 * - `RcDocument.Magenta`: rgb(255, 0, 255) is used as a transparent color
 * - `RcDocument.BottomLeftPixel`: the color of the bottom left pixel is used as transparent
 * - `RcDocument.AllColors`: combination of all above
 */
bool RcDocument::writeAssetsToImage(int flags)
{
    SET_DEFAULT_VALUE(RcAssetColors, static_cast<ConversionFlags>(flags));
    if (m_cacheAssets.isEmpty())
        convertAssets();
    RcCore::writeAssetsToImage(m_cacheAssets, static_cast<RcCore::Asset::TransparentColors>(flags));
    return true;
}

/*!
 * \qmlmethod bool RcDocument::writeAssetsToQrc( string fileName)
 * \sa RcDocument::convertAssets
 * Writes a qrc file with the given `fileName`. Returns `true` if no issues.
 *
 * Before writing the qrc file, you first need to convert them using RcDocument::convertAssets.
 */
bool RcDocument::writeAssetsToQrc(const QString &fileName)
{
    if (m_cacheAssets.isEmpty())
        convertAssets();

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        RcCore::writeAssetsToQrc(m_cacheAssets, &file, fileName);
        return true;
    }
    return false;
}

/*!
 * \qmlmethod bool RcDocument::writeDialogToUi( Widget dialog, string fileName)
 * \sa RcDocument::dialog
 * Writes a ui file for the given `dialog`, to the given `fileName`. Return `true` if no issues.
 */
bool RcDocument::writeDialogToUi(const RcCore::Widget &dialog, const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        RcCore::writeDialogToUi(dialog, &file);
        return true;
    }
    return false;
}

/*!
 * \qmlmethod bool RcDocument::previewDialog( Widget dialog )
 * \sa RcDocument::dialog
 * Preview the result of the conversion RC->UI
 */
void RcDocument::previewDialog(const RcCore::Widget &dialog) const
{
    QUiLoader loader;

    QBuffer buffer;
    if (buffer.open(QIODevice::WriteOnly)) {
        RcCore::writeDialogToUi(dialog, &buffer);
        buffer.close();
    }

    if (buffer.open(QIODevice::ReadOnly)) {
        QWidget *widget = loader.load(&buffer);
        widget->setAttribute(Qt::WA_DeleteOnClose);
        widget->show();
    }
}

bool RcDocument::doSave(const QString &fileName)
{
    Q_UNUSED(fileName);
    // nothing to do
    return true;
}

bool RcDocument::doLoad(const QString &fileName)
{
    m_data = RcCore::parse(fileName);
    return true;
}

} // namespace Core
