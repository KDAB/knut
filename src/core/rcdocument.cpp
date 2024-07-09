/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "rcdocument.h"
#include "logger.h"
#include "rccore/rcfile.h"
#include "utils/log.h"

#include <QBuffer>
#include <QFile>
#include <QUiLoader>
#include <QWidget>
#include <kdalgorithms.h>

namespace Core {

/*!
 * \qmltype RcDocument
 * \brief Provides access to the content of a RC file (MFC resource file).
 * \inqmlmodule Knut
 * \ingroup RcDocument/@first
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
/*!
 * \qmlproperty array<string> RcDocument::languages
 * This read-only property holds the list of languages available in the file.
 */

RcDocument::RcDocument(QObject *parent)
    : Document(Type::Rc, parent)
{
}

bool RcDocument::isValid() const
{
    return m_rcFile.isValid;
}

QList<RcCore::Asset> RcDocument::assets() const
{
    if (isDataValid()) {
        if (m_cacheAssets.isEmpty())
            return data().assets;
        return m_cacheAssets;
    }
    return {};
}

QList<RcCore::Action> RcDocument::actions() const
{
    if (isDataValid()) {
        if (m_cacheActions.isEmpty())
            const_cast<RcDocument *>(this)->convertActions();
        return m_cacheActions;
    }
    return {};
}

/*!
 * \qmlmethod ToolBar RcDocument::action(string id)
 * Returns the action for the given `id`.
 */
RcCore::Action RcDocument::action(const QString &id) const
{
    LOG("RcDocument::action", id);

    // Make sure the action vector is populated by calling actions
    if (isDataValid() && !actions().isEmpty()) {
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
 * Returns all actions used in the menu `menuId`.
 */
RcCore::ActionList RcDocument::actionsFromMenu(const QString &menuId) const
{
    LOG("RcDocument::actionsFromMenu", menuId);

    if (!isDataValid())
        return {};

    RcCore::ActionList actions;
    if (auto menu = data().menu(menuId)) {
        const auto actionIds = menu->actionIds();
        for (const auto &id : actionIds)
            actions.push_back(action(id));
    }
    return actions;
}

/*!
 * \qmlmethod array<Action> RcDocument::actionsFromToolbar(string toolBarId)
 * Returns all actions used in the toolbar `toolBarId`.
 */
RcCore::ActionList RcDocument::actionsFromToolbar(const QString &toolBarId) const
{
    LOG("RcDocument::actionsFromMenu", toolBarId);

    if (!isDataValid())
        return {};

    RcCore::ActionList actions;
    if (auto toolbar = data().toolBar(toolBarId)) {
        const auto actionIds = toolbar->actionIds();
        for (const auto &id : actionIds)
            actions.push_back(action(id));
    }
    return actions;
}

QList<RcCore::ToolBar> RcDocument::toolBars() const
{
    if (isDataValid())
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

    if (isDataValid()) {
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
 * - `RcDocument.UpdateHierarchy`: create a hierarchy of parent-children, by default there are none in MFC
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
    if (isDataValid()) {
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

    if (isDataValid()) {
        if (auto menu = data().menu(id))
            return *menu;
    }
    return {};
}

/*!
 * \qmlmethod Menu RcDocument::ribbon(string id)
 * Returns the ribbon for the given `id`.
 */
RcCore::Ribbon RcDocument::ribbon(const QString &id) const
{
    LOG("RcDocument::ribbon", id);

    if (isDataValid()) {
        if (auto ribbon = data().ribbon(id)) {
            const_cast<RcCore::Ribbon *>(ribbon)->load();
            return *ribbon;
        }
    }
    return {};
}

QStringList RcDocument::dialogIds() const
{
    LOG("RcDocument::dialogIds");
    if (isDataValid()) {
        const auto &dialogs = data().dialogs;
        QStringList result;
        result.reserve(dialogs.size());
        std::ranges::transform(dialogs, std::back_inserter(result), [](const auto &dialog) {
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
    if (isDataValid()) {
        const auto &menus = data().menus;
        QStringList result;
        result.reserve(menus.size());
        std::ranges::transform(menus, std::back_inserter(result), [](const auto &menu) {
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
    if (isDataValid()) {
        const auto &accelerators = data().acceleratorTables;
        QStringList result;
        result.reserve(accelerators.size());
        std::ranges::transform(accelerators, std::back_inserter(result), [](const auto &accelerator) {
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
    if (isDataValid()) {
        const auto &toolbars = data().toolBars;
        QStringList result;
        result.reserve(toolbars.size());
        std::ranges::transform(toolbars, std::back_inserter(result), [](const auto &toolbar) {
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
    if (isDataValid())
        return data().strings.keys();
    return {};
}

QStringList RcDocument::ribbonIds() const
{
    LOG("RcDocument::ribbonIds");
    if (isDataValid()) {
        const auto &ribbons = data().ribbons;
        QStringList result;
        result.reserve(ribbons.size());
        std::ranges::transform(ribbons, std::back_inserter(result), [](const auto &ribbon) {
            return ribbon.id;
        });
        result.sort();
        return result;
    }
    return {};
}

/*!
 * \qmlmethod array<String> RcDocument::stringsForLanguage(string language)
 * Returns translated string for specific `language`.
 */
QList<RcCore::String> RcDocument::stringsForLanguage(const QString &language) const
{
    LOG("RcDocument::stringsForLanguage", language);
    if (m_rcFile.isValid && m_rcFile.data.contains(language)) {
        const RcCore::Data data = const_cast<RcCore::RcFile *>(&m_rcFile)->data[language];
        const auto &strings = data.strings;
        return strings.values();
    } else {
        return {};
    }
}

/*!
 * \qmlmethod string RcDocument::stringForLanguage(string language, string id)
 * Return the string for the given `id` in language `language`.
 */
QString RcDocument::stringForLanguage(const QString &language, const QString &id) const
{
    LOG("RcDocument::stringForLanguage", language, id);

    if (m_rcFile.isValid && m_rcFile.data.contains(language)) {
        const RcCore::Data data = const_cast<RcCore::RcFile *>(&m_rcFile)->data[language];
        const auto &strings = data.strings;
        return strings.value(id).text;
    } else {
        spdlog::warn("RcDocument::stringForLanguage: language {} does not exist in the rc file.", language);
        return {};
    }
}

QList<RcCore::String> RcDocument::strings() const
{
    if (isDataValid()) {
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

    if (isDataValid())
        return data().strings.value(id).text;
    return {};
}

std::optional<RcCore::Data::Control> findControlWithId(const RcCore::Data::Dialog *dialog, const QString &id)
{
    const auto controls = dialog->controls;
    auto isSameId = [id](const auto &control) {
        return control.id == id;
    };
    auto result = kdalgorithms::find_if(controls, isSameId);
    if (!result)
        return {};
    return *result;
}

QString extractStringForDialog(const RcCore::Data::Dialog *dialog, const QString &id)
{
    if (dialog) {
        const auto control = findControlWithId(dialog, id);
        if (!control) {
            spdlog::warn("RcDocument::stringForDialogAndLanguage: control from id {} does not exist in the rc file.",
                         id);
            return {};
        }
        return control.value().text;
    } else {
        spdlog::warn("RcDocument::stringForDialogAndLanguage: id {} does not exist in the rc file.", id);
        return {};
    }
}

/*!
 * \qmlmethod string RcDocument::stringForDialogAndLanguage(string language, string dialogId, string id)
 * Return the string for the given `language`, `dialogid` and id.
 */
QString RcDocument::stringForDialogAndLanguage(const QString &language, const QString &dialogId,
                                               const QString &id) const
{
    LOG("RcDocument::stringForDialogAndLanguage", language, dialogId, id);

    if (m_rcFile.isValid && m_rcFile.data.contains(language)) {
        const RcCore::Data data = const_cast<RcCore::RcFile *>(&m_rcFile)->data[language];
        const auto dialog = data.dialog(dialogId);
        return extractStringForDialog(dialog, id);
    } else {
        spdlog::warn("RcDocument::stringForDialogAndLanguage: language {} does not exist in the rc file.", language);
        return {};
    }
}

/*!
 * \qmlmethod string RcDocument::stringForDialog(string dialogId, string id)
 * Return the string for the given `dialogid` and id.
 */
QString RcDocument::stringForDialog(const QString &dialogId, const QString &id) const
{
    LOG("RcDocument::stringForDialog", dialogId, id);
    if (isDataValid()) {
        const auto dialog = data().dialog(dialogId);
        return extractStringForDialog(dialog, id);
    }
    return {};
}

const RcCore::Data &RcDocument::data() const
{
    Q_ASSERT(m_rcFile.data.contains(m_language));
    return const_cast<RcCore::RcFile *>(&m_rcFile)->data[m_language];
}

QString RcDocument::language() const
{
    LOG("RcDocument::language");
    LOG_RETURN("language", m_language);
}

void RcDocument::setLanguage(const QString &language)
{
    LOG("RcDocument::setLanguage", language);

    if (!m_rcFile.data.contains(language)) {
        spdlog::warn("RcDocument::setLanguage: language {} does not exist in the rc file.", language);
        return;
    }

    if (m_language == language)
        return;

    m_language = language;
    m_cacheAssets.clear();
    m_cacheActions.clear();
    emit languageChanged();
    emit dataChanged();
}

QStringList RcDocument::languages() const
{
    LOG("RcDocument::languages");

    QStringList langs;
    if (m_rcFile.isValid) {
        langs = m_rcFile.data.keys();
        kdalgorithms::sort(langs);
    }
    LOG_RETURN("languages", langs);
}

const RcCore::RcFile &RcDocument::file() const
{
    return m_rcFile;
}

QList<RcCore::Menu> RcDocument::menus() const
{
    if (isDataValid())
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
 * - `RcDocument.SplitToolBar`: split toolbars strip into individual icon, one per action
 * - `RcDocument.ConvertToPng`: convert BMPs to PNGs, needed if we want to also change the transparency
 * - `RcDocument.AllFlags`: combination of all above
 */
void RcDocument::convertAssets(int flags)
{
    LOG("RcDocument::convertAssets", flags);

    SET_DEFAULT_VALUE(RcAssetFlags, static_cast<ConversionFlags>(flags));
    if (isDataValid()) {
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
    if (isDataValid()) {
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
 * \qmlmethod bool RcDocument::mergeAllLanguages(string language = "[default]")
 * Merges all languages data into one.
 */
void RcDocument::mergeAllLanguages(const QString &language)
{
    LOG("RcDocument::mergeAllLanguages", language);

    m_rcFile.mergeLanguages(m_rcFile.data.keys(), language);
    {
        // Even if the newLanguage is set, we want to send the signals unconditionally
        QSignalBlocker sb(this);
        setLanguage(language);
    }
    emit languagesChanged();
    emit languageChanged();
    emit dataChanged();
}

/*!
 * \qmlmethod bool RcDocument::mergeLanguages()
 * Merges languages based on the language map in the settings.
 *
 * The language map gives for each language a resulting language, and if multiple source languages have the same
 * resulting language they will be merged together.
 */
void RcDocument::mergeLanguages()
{
    LOG("RcDocument::mergeLanguages");

    const auto languageMap = Settings::instance()->value<std::map<std::string, std::string>>(Settings::RcLanguageMap);

    // Find all the merges to do
    std::unordered_map<QString, QStringList> merges;
    const QStringList languageList = m_rcFile.data.keys();
    for (const auto &lang : languageList) {
        // Either there's LANG;SUBLANG, or just LANG in the languageMap
        auto it = languageMap.find(lang.toStdString());
        if (it == languageMap.end()) {
            it = languageMap.find(lang.split(';').first().toStdString());
        }
        if (it != languageMap.end()) {
            const auto newLang = QString::fromStdString(it->second.empty() ? DefaultLanguage : it->second);
            merges[newLang].append(lang);
        }
    }

    // Do all the merges
    for (const auto &[lang, values] : merges)
        m_rcFile.mergeLanguages(values, lang);

    {
        // Even if the newLanguage is set, we want to send the signals unconditionally
        QSignalBlocker sb(this);
        setLanguage(languages().constFirst());
    }
    emit languagesChanged();
    emit languageChanged();
    emit dataChanged();
}

/*!
 * \qmlmethod string RcDocument::convertLanguageToCode(string language)
 * Returns language code as defined by the ISO 639 for language name
 */
QString RcDocument::convertLanguageToCode(const QString &language)
{
    LOG("RcDocument::convertLanguageToCode", language);
    if (language == DefaultLanguage) {
        return {};
    }
    return RcCore::convertLanguageToCode(language);
}

bool RcDocument::doSave(const QString &fileName)
{
    Q_UNUSED(fileName)
    // nothing to do
    return true;
}

bool RcDocument::doLoad(const QString &fileName)
{
    m_rcFile = RcCore::parse(fileName);

    // There should always be one language in a RC file. If not, bail out.
    if (m_rcFile.data.isEmpty())
        return false;

    mergeLanguages();

    return true;
}

bool RcDocument::isDataValid() const
{
    return m_rcFile.isValid && m_rcFile.data.contains(m_language);
}
} // namespace Core

#include "moc_rcdocument.cpp"
