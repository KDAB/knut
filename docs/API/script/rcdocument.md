# RcDocument

Provides access to the content of a RC file (MFC resource file). [More...](#detailed-description)

```qml
import Script
```

## Properties

| | Name |
|-|-|
|array&lt;string>|**[acceleratorIds](#acceleratorIds)**|
|array&lt;[Asset](../script/asset.md)>|**[actions](#actions)**|
|array&lt;[Asset](../script/asset.md)>|**[assets](#assets)**|
|array&lt;string>|**[dialogIds](#dialogIds)**|
|string|**[language](#language)**|
|array&lt;string>|**[languages](#languages)**|
|array&lt;string>|**[menuIds](#menuIds)**|
|array&lt;[Menu](../script/menu.md)>|**[menus](#menus)**|
|array&lt;string>|**[stringIds](#stringIds)**|
|array&lt;string>|**[strings](#strings)**|
|array&lt;string>|**[toolbarIds](#toolbarIds)**|
|array&lt;[ToolBar](../script/toolbar.md)>|**[toolbars](#toolbars)**|
|bool|**[valid](#valid)**|

## Methods

| | Name |
|-|-|
|[ToolBar](../script/toolbar.md) |**[action](#action)**(string id)|
|array&lt;[Action](../script/action.md)> |**[actionsFromMenu](#actionsFromMenu)**(string menuId)|
|array&lt;[Action](../script/action.md)> |**[actionsFromToolbar](#actionsFromToolbar)**(string toolBarId)|
|void |**[convertActions](#convertActions)**(int flags)|
||**[convertAssets](#convertAssets)**(int flags)|
|string |**[convertLanguageToCode](#convertLanguageToCode)**(string language)|
|[Widget](../script/widget.md) |**[dialog](#dialog)**(string id, int flags, real scaleX, real scaleY)|
|[Menu](../script/menu.md) |**[menu](#menu)**(string id)|
|bool |**[mergeAllLanguages](#mergeAllLanguages)**(string language = "[default]")|
|bool |**[mergeLanguages](#mergeLanguages)**()|
|bool |**[previewDialog](#previewDialog)**([Widget](../script/widget.md) dialog)|
|[Menu](../script/menu.md) |**[ribbon](#ribbon)**(string id)|
|string |**[stringForDialog](#stringForDialog)**(string dialogId, string id)|
|string |**[stringForDialogAndLanguage](#stringForDialogAndLanguage)**(string language, string dialogId, string id)|
|string |**[stringForLanguage](#stringForLanguage)**(string language, string id)|
|array&lt;[String](../script/string.md)> |**[stringsForLanguage](#stringsForLanguage)**(string language)|
|string |**[text](#text)**(string id)|
|[ToolBar](../script/toolbar.md) |**[toolBar](#toolBar)**(string id)|
|bool |**[writeAssetsToImage](#writeAssetsToImage)**(int flags)|
|bool |**[writeAssetsToQrc](#writeAssetsToQrc)**(string fileName)|
|bool |**[writeDialogToUi](#writeDialogToUi)**([Widget](../script/widget.md) dialog, string fileName)|

## Property Documentation

#### <a name="acceleratorIds"></a>array&lt;string> **acceleratorIds**

This read-only property holds the list of accelerator's ids in the RC file.

#### <a name="actions"></a>array&lt;[Asset](../script/asset.md)> **actions**

This read-only property holds the list of actions in the RC file.

#### <a name="assets"></a>array&lt;[Asset](../script/asset.md)> **assets**

This read-only property holds the list of assets in the RC file.

#### <a name="dialogIds"></a>array&lt;string> **dialogIds**

This read-only property holds the list of dialog's ids in the RC file.

#### <a name="language"></a>string **language**

This property holds the current language used for the data in the RC file. All other properties or method will work
on the data for this specific language.

#### <a name="languages"></a>array&lt;string> **languages**

This read-only property holds the list of languages available in the file.

#### <a name="menuIds"></a>array&lt;string> **menuIds**

This read-only property holds the list of menu's ids in the RC file.

#### <a name="menus"></a>array&lt;[Menu](../script/menu.md)> **menus**

This read-only property holds the list of menus in the RC file.

#### <a name="stringIds"></a>array&lt;string> **stringIds**

This read-only property holds the list of string's ids in the RC file.

#### <a name="strings"></a>array&lt;string> **strings**

This read-only property holds the list of strings in the RC file.

#### <a name="toolbarIds"></a>array&lt;string> **toolbarIds**

This read-only property holds the list of toolbar's ids in the RC file.

#### <a name="toolbars"></a>array&lt;[ToolBar](../script/toolbar.md)> **toolbars**

This read-only property holds the list of toolbars in the RC file.

#### <a name="valid"></a>bool **valid**

This read-only property defines if the RC file is valid for our parser.

Note that the RC file may be valid, and our parser needs to be updated.

## Method Documentation

#### <a name="action"></a>[ToolBar](../script/toolbar.md) **action**(string id)

Returns the action for the given `id`.

#### <a name="actionsFromMenu"></a>array&lt;[Action](../script/action.md)> **actionsFromMenu**(string menuId)

Returns all actions used in the menu `menuId`.

#### <a name="actionsFromToolbar"></a>array&lt;[Action](../script/action.md)> **actionsFromToolbar**(string toolBarId)

Returns all actions used in the toolbar `toolBarId`.

#### <a name="convertActions"></a>void **convertActions**(int flags)


!!! Warning "Experimental API"
    The API here is still experimental, and may change in follow-up release. Use it at your own risk.

Convert all actions using the `flags`.

The `flags` are used to fill the iconPath of the action:

- `RcDocument.RemoveUnknown`: remove the unknown assets
- `RcDocument.SplitToolBar`: split toolbar's strips into individual icon, one per action
- `RcDocument.ConvertToPng`: convert BMPs to PNGs, needed if we want to also change the transparency
- `RcDocument.AllFlags`: combination of all above

#### <a name="convertAssets"></a>**convertAssets**(int flags)

Convert all assets using the `flags`.

- `RcDocument.RemoveUnknown`: remove the unknown assets
- `RcDocument.SplitToolBar`: split toolbars strip into individual icon, one per action
- `RcDocument.ConvertToPng`: convert BMPs to PNGs, needed if we want to also change the transparency
- `RcDocument.AllFlags`: combination of all above

#### <a name="convertLanguageToCode"></a>string **convertLanguageToCode**(string language)

Returns language code as defined by the ISO 639 for language name

#### <a name="dialog"></a>[Widget](../script/widget.md) **dialog**(string id, int flags, real scaleX, real scaleY)

Returns the dialog for the given `id`.

To do the conversion, the `flags` and scale factor `scaleX` and `scaleY` are used. Default
values are coming from the project settings.

Flags could be:

- `RcDocument.UpdateHierarchy`: create a hierarchy of parent-children, by default there are none in MFC
- `RcDocument.UpdateGeometry`: use the scale factor to change the dialog size
- `RcDocument.UseIdForPixmap`: use the id as a resource value for the pixmaps in labels
- `RcDocument.AllFlags`: combination of all above

#### <a name="menu"></a>[Menu](../script/menu.md) **menu**(string id)

Returns the menu for the given `id`.

#### <a name="mergeAllLanguages"></a>bool **mergeAllLanguages**(string language = "[default]")

Merges all languages data into one.

#### <a name="mergeLanguages"></a>bool **mergeLanguages**()

Merges languages based on the language map in the settings.

The language map gives for each language a resulting language, and if multiple source languages have the same
resulting language they will be merged together.

#### <a name="previewDialog"></a>bool **previewDialog**([Widget](../script/widget.md) dialog)

Preview the result of the conversion RC->UI

#### <a name="ribbon"></a>[Menu](../script/menu.md) **ribbon**(string id)

Returns the ribbon for the given `id`.

#### <a name="stringForDialog"></a>string **stringForDialog**(string dialogId, string id)

Return the string for the given `dialogid` and id.

#### <a name="stringForDialogAndLanguage"></a>string **stringForDialogAndLanguage**(string language, string dialogId, string id)

Return the string for the given `language`, `dialogid` and id.

#### <a name="stringForLanguage"></a>string **stringForLanguage**(string language, string id)

Return the string for the given `id` in language `language`.

#### <a name="stringsForLanguage"></a>array&lt;[String](../script/string.md)> **stringsForLanguage**(string language)

Returns translated string for specific `language`.

#### <a name="text"></a>string **text**(string id)

Return the string for the given `id`.

#### <a name="toolBar"></a>[ToolBar](../script/toolbar.md) **toolBar**(string id)

Returns the toolbar for the given `id`.

#### <a name="writeAssetsToImage"></a>bool **writeAssetsToImage**(int flags)

Writes the assets to images, using `flags` for transparency settings. Returns `true` if no issues.

Before writing the assets to disk, you first need to convert them using RcDocument::convertAssets.
BMPs assets don't have transparency, but some specific colors are used for that in MFC. When
writing the assets to PNGs, the `flags` define how to handle transparency:

- `RcDocument.NoColors`: no transparent color
- `RcDocument.Gray`: rgb(192, 192, 192) is used as a transparent color
- `RcDocument.Magenta`: rgb(255, 0, 255) is used as a transparent color
- `RcDocument.BottomLeftPixel`: the color of the bottom left pixel is used as transparent
- `RcDocument.AllColors`: combination of all above

#### <a name="writeAssetsToQrc"></a>bool **writeAssetsToQrc**(string fileName)

Writes a qrc file with the given `fileName`. Returns `true` if no issues.

Before writing the qrc file, you first need to convert them using RcDocument::convertAssets.

#### <a name="writeDialogToUi"></a>bool **writeDialogToUi**([Widget](../script/widget.md) dialog, string fileName)

Writes a ui file for the given `dialog`, to the given `fileName`. Return `true` if no issues.
