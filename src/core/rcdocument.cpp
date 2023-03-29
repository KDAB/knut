#include "rcdocument.h"

#include "logger.h"

#include "rccore/rcfile.h"

#include <QBuffer>
#include <QFile>
#include <QUiLoader>
#include <QWidget>

#include <kdalgorithms.h>
#include <spdlog/spdlog.h>

namespace Core {

/*!
 * \qmltype RcDocument
 * \brief Provides access to the content of a RC file (MFC resource file).
 * \inqmlmodule Script
 * \ingroup RcDocument/@first
 * \since 1.0
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
 * \qmlproperty array<Asset> RcDocument::actions
 * \since 1.1
 * This read-only property holds the list of actions in the RC file.
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
 * \qmlproperty array<string> RcDocument::strings
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
/*!
 * \qmlproperty string RcDocument::language
 * This property holds the current language used for the data in the RC file. All other properties or method will work
 * on the data for this specific language.
 */

RcDocument::RcDocument(QObject *parent)
    : Document(Type::Rc, parent)
{
}

bool RcDocument::isValid() const
{
    return m_rcFile.isValid;
}

QVector<RcCore::Asset> RcDocument::assets() const
{
    if (m_rcFile.isValid) {
        if (m_cacheAssets.isEmpty())
            return data().assets;
        return m_cacheAssets;
    }
    return {};
}

QVector<RcCore::Action> RcDocument::actions() const
{
    if (m_rcFile.isValid) {
        if (m_cacheActions.isEmpty())
            const_cast<RcDocument *>(this)->convertActions();
        return m_cacheActions;
    }
    return {};
}

/*!
 * \qmlmethod ToolBar RcDocument::action(string id)
 * \since 1.1
 * Returns the action for the given `id`.
 */
RcCore::Action RcDocument::action(const QString &id) const
{
    LOG("RcDocument::action", id);

    // Make sure the action vector is populated by calling actions
    if (m_rcFile.isValid && !actions().isEmpty()) {
        auto result = kdalgorithms::find_if(m_cacheActions, [id](const auto &data) {
            return data.id == id;
        });
        if (!result)
            return {};
        return *result;
    }
    return {};
}

/*!
 * \qmlmethod array<Action> RcDocument::actionsFromMenu(string menuId)
 * \since 1.1
 * Returns all actions used in the menu `menuId`.
 */
QVector<RcCore::Action> RcDocument::actionsFromMenu(const QString &menuId) const
{
    LOG("RcDocument::actionsFromMenu", menuId);

    if (!m_rcFile.isValid)
        return {};

    QVector<RcCore::Action> actions;
    if (auto menu = data().menu(menuId)) {
        const auto actionIds = menu->actionIds();
        for (const auto &id : actionIds)
            actions.push_back(action(id));
    }
    return actions;
}

/*!
 * \qmlmethod array<Action> RcDocument::actionsFromToolbar(string toolBarId)
 * \since 1.1
 * Returns all actions used in the toolbar `toolBarId`.
 */
QVector<RcCore::Action> RcDocument::actionsFromToolbar(const QString &toolBarId) const
{
    LOG("RcDocument::actionsFromMenu", toolBarId);

    if (!m_rcFile.isValid)
        return {};

    QVector<RcCore::Action> actions;
    if (auto toolbar = data().toolBar(toolBarId)) {
        const auto actionIds = toolbar->actionIds();
        for (const auto &id : actionIds)
            actions.push_back(action(id));
    }
    return actions;
}

QVector<RcCore::ToolBar> RcDocument::toolBars() const
{
    if (m_rcFile.isValid)
        return data().toolBars;
    return {};
}

/*!
 * \qmlmethod ToolBar RcDocument::toolBar(string id)
 * Returns the toolbar for the given `id`.
 */
RcCore::ToolBar RcDocument::toolBar(const QString &id) const
{
    LOG("RcDocument::toolBar", id);

    if (m_rcFile.isValid) {
        if (auto tb = data().toolBar(id))
            return *tb;
    }
    return {};
}

/*!
 * \qmlmethod Widget RcDocument::dialog(string id, int flags, real scaleX, real scaleY)
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
    LOG("RcDocument::dialog", id, flags, scaleX, scaleY);

    SET_DEFAULT_VALUE(RcDialogFlags, static_cast<ConversionFlags>(flags));
    SET_DEFAULT_VALUE(RcDialogScaleX, scaleX);
    SET_DEFAULT_VALUE(RcDialogScaleY, scaleY);
    if (m_rcFile.isValid) {
        if (auto dialog = data().dialog(id))
            return RcCore::convertDialog(data(), *dialog, static_cast<RcCore::Widget::ConversionFlags>(flags), scaleX,
                                         scaleY);
    }
    return {};
}

/*!
 * \qmlmethod Menu RcDocument::menu(string id)
 * Returns the menu for the given `id`.
 */
RcCore::Menu RcDocument::menu(const QString &id) const
{
    LOG("RcDocument::menu", id);

    if (m_rcFile.isValid) {
        if (auto menu = data().menu(id))
            return *menu;
    }
    return {};
}

QStringList RcDocument::dialogIds() const
{
    LOG("RcDocument::dialogIds");
    if (m_rcFile.isValid) {
        const auto &dialogs = data().dialogs;
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
    LOG("RcDocument::menuIds");
    if (m_rcFile.isValid) {
        const auto &menus = data().menus;
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
    LOG("RcDocument::acceleratorIds");
    if (m_rcFile.isValid) {
        const auto &accelerators = data().acceleratorTables;
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
    LOG("RcDocument::toolbarIds");
    if (m_rcFile.isValid) {
        const auto &toolbars = data().toolBars;
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
    LOG("RcDocument::stringIds");
    if (m_rcFile.isValid)
        return data().strings.keys();
    return {};
}

QList<RcCore::String> RcDocument::strings() const
{
    if (m_rcFile.isValid) {
        const auto &strings = data().strings;
        return strings.values();
    }
    return {};
}

/*!
 * \qmlmethod string RcDocument::text(string id)
 * Return the string for the given `id`.
 */
QString RcDocument::string(const QString &id) const
{
    LOG("RcDocument::string", id);

    if (m_rcFile.isValid)
        return data().strings.value(id).text;
    return {};
}

const RcCore::Data &RcDocument::data() const
{
    Q_ASSERT(m_rcFile.data.contains(m_language));
    return const_cast<RcCore::RcFile *>(&m_rcFile)->data[m_language];
}

QString RcDocument::language() const
{
    return m_language;
}

void RcDocument::setLanguage(const QString &language)
{
    LOG("Document::setLanguage", language);

    if (!m_rcFile.data.contains(language)) {
        spdlog::warn("RcDocument::setLanguage: language {} does not exist in the rc file.", language.toStdString());
        return;
    }

    if (m_language == language)
        return;

    m_language = language;
    m_cacheAssets.clear();
    m_cacheActions.clear();
    emit languageChanged();
}

const RcCore::RcFile &RcDocument::file() const
{
    return m_rcFile;
}

QVector<RcCore::Menu> RcDocument::menus() const
{
    if (m_rcFile.isValid)
        return data().menus;
    return {};
}

/*!
 * \qmlmethod RcDocument::convertAssets(int flags)
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
    LOG("RcDocument::convertAssets", flags);

    SET_DEFAULT_VALUE(RcAssetFlags, static_cast<ConversionFlags>(flags));
    if (m_rcFile.isValid) {
        m_cacheAssets = RcCore::convertAssets(data(), static_cast<RcCore::Asset::ConversionFlags>(flags));
        emit fileNameChanged();
    }
}

/*!
 * \qmlmethod void RcDocument::convertActions(int flags)
 * \todo
 * Convert all actions using the `flags`.
 *
 * The `flags` are used to fill the iconPath of the action:
 *
 * - `RcDocument.RemoveUnknown`: remove the unknown assets
 * - `RcDocument.SplitToolBar`: split toolbar's strips into individual icon, one per action
 * - `RcDocument.ConvertToPng`: convert BMPs to PNGs, needed if we want to also change the transparency
 * - `RcDocument.AllFlags`: combination of all above
 */
void RcDocument::convertActions(int flags)
{
    LOG("RcDocument::convertActions", flags);

    SET_DEFAULT_VALUE(RcAssetFlags, static_cast<ConversionFlags>(flags));
    if (m_rcFile.isValid) {
        m_cacheActions = RcCore::convertActions(data(), static_cast<RcCore::Asset::ConversionFlags>(flags));
        emit fileNameChanged();
    }
}

/*!
 * \qmlmethod bool RcDocument::writeAssetsToImage(int flags)
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
    LOG("RcDocument::writeAssetsToImage", flags);

    SET_DEFAULT_VALUE(RcAssetColors, static_cast<ConversionFlags>(flags));
    if (m_cacheAssets.isEmpty())
        convertAssets();
    RcCore::writeAssetsToImage(m_cacheAssets, static_cast<RcCore::Asset::TransparentColors>(flags));
    return true;
}

/*!
 * \qmlmethod bool RcDocument::writeAssetsToQrc(string fileName)
 * \sa RcDocument::convertAssets
 * Writes a qrc file with the given `fileName`. Returns `true` if no issues.
 *
 * Before writing the qrc file, you first need to convert them using RcDocument::convertAssets.
 */
bool RcDocument::writeAssetsToQrc(const QString &fileName)
{
    LOG("RcDocument::writeAssetsToQrc", fileName);

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
 * \qmlmethod bool RcDocument::writeDialogToUi(Widget dialog, string fileName)
 * \sa RcDocument::dialog
 * Writes a ui file for the given `dialog`, to the given `fileName`. Return `true` if no issues.
 */
bool RcDocument::writeDialogToUi(const RcCore::Widget &dialog, const QString &fileName)
{
    LOG("RcDocument::writeDialogToUi", dialog.id, fileName);

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        RcCore::writeDialogToUi(dialog, &file);
        return true;
    }
    return false;
}

/*!
 * \qmlmethod bool RcDocument::previewDialog(Widget dialog )
 * \sa RcDocument::dialog
 * Preview the result of the conversion RC->UI
 */
void RcDocument::previewDialog(const RcCore::Widget &dialog) const
{
    LOG("RcDocument::previewDialog", dialog.id);

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

/*!
 * \qmlmethod bool RcDocument::mergeAllLanguages(string newLanguage = "default")
 * Merges all languages data into one.
 */
void RcDocument::mergeAllLanguages(const QString &newLanguage)
{
    RcCore::mergeAllLanguages(m_rcFile, newLanguage);
    setLanguage(newLanguage);
}

bool RcDocument::doSave(const QString &fileName)
{
    Q_UNUSED(fileName);
    // nothing to do
    return true;
}

bool RcDocument::doLoad(const QString &fileName)
{
    m_rcFile = RcCore::parse(fileName);
    m_language = m_rcFile.data.begin().key();
    return true;
}

} // namespace Core

#include "moc_rcdocument.cpp"
