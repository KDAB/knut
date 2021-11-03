# RcDocument

Provides access to the content of a RC file (MFC resource file). [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 4.0</td></tr>
</table>

## Properties

| | Name |
|-|-|
|array<string>|**[acceleratorIds](#acceleratorIds)**|
|array<[Asset](../script/asset.md)>|**[assets](#assets)**|
|array<string>|**[dialogIds](#dialogIds)**|
|array<string>|**[menuIds](#menuIds)**|
|array<[Menu](../script/menu.md)>|**[menus](#menus)**|
|array<string>|**[stringIds](#stringIds)**|
|array<[String](../script/string.md)>|**[strings](#strings)**|
|array<string>|**[toolbarIds](#toolbarIds)**|
|array<[ToolBar](../script/toolbar.md)>|**[toolbars](#toolbars)**|
|bool|**[valid](#valid)**|

## Methods

| | Name |
|-|-|
||**[convertAssets](#convertAssets)**(int flags)|
|[Widget](../script/widget.md) |**[dialog](#dialog)**(string id, int flags, real scaleX, real scaleY)|
|[Menu](../script/menu.md) |**[menu](#menu)**(string id)|
|bool |**[previewDialog](#previewDialog)**([Widget](../script/widget.md) dialog)|
|string |**[text](#text)**(string id)|
|[ToolBar](../script/toolbar.md) |**[toolBar](#toolBar)**(string id)|
|bool |**[writeAssetsToImage](#writeAssetsToImage)**(int flags)|
|bool |**[writeAssetsToQrc](#writeAssetsToQrc)**(string fileName)|
|bool |**[writeDialogToUi](#writeDialogToUi)**([Widget](../script/widget.md) dialog, string fileName)|

## Property Documentation

#### <a name="acceleratorIds"></a>array<string> **acceleratorIds**

This read-only property holds the list of accelerator's ids in the RC file.

#### <a name="assets"></a>array<[Asset](../script/asset.md)> **assets**

This read-only property holds the list of assets in the RC file.

#### <a name="dialogIds"></a>array<string> **dialogIds**

This read-only property holds the list of dialog's ids in the RC file.

#### <a name="menuIds"></a>array<string> **menuIds**

This read-only property holds the list of menu's ids in the RC file.

#### <a name="menus"></a>array<[Menu](../script/menu.md)> **menus**

This read-only property holds the list of menus in the RC file.

#### <a name="stringIds"></a>array<string> **stringIds**

This read-only property holds the list of string's ids in the RC file.

#### <a name="strings"></a>array<[String](../script/string.md)> **strings**

This read-only property holds the list of strings in the RC file.

#### <a name="toolbarIds"></a>array<string> **toolbarIds**

This read-only property holds the list of toolbar's ids in the RC file.

#### <a name="toolbars"></a>array<[ToolBar](../script/toolbar.md)> **toolbars**

This read-only property holds the list of toolbars in the RC file.

#### <a name="valid"></a>bool **valid**

This read-only property defines if the RC file is valid for our parser.

Note that the RC file may be valid, and our parser needs to be updated.

## Method Documentation

#### <a name="convertAssets"></a>**convertAssets**(int flags)

Convert all assets using the `flags`.

- `RcDocument.RemoveUnknown`: remove the unknown assets
- `RcDocument.SplitToolBar`: split oolbars strip into individual icon, one per action
- `RcDocument.ConvertToPng`: convert BMPs to PNGs, needed if we want to also change the transparency
- `RcDocument.AllFlags`: combination of all above

#### <a name="dialog"></a>[Widget](../script/widget.md) **dialog**(string id, int flags, real scaleX, real scaleY)

Returns the dialog for the given `id`.

To do the conversion, the `flags` and scale factor `scaleX` and `scaleY` are used. Default
values are coming from the project settings.

Flags could be:

- `RcDocument.UpdateHierachy`: create a hierarchy of parent-children, by default there are none in MFC
- `RcDocument.UpdateGeometry`: use the scale factor to change the dialog size
- `RcDocument.UseIdForPixmap`: use the id as a resource value for the pixmaps in labels
- `RcDocument.AllFlags`: combination of all above

#### <a name="menu"></a>[Menu](../script/menu.md) **menu**(string id)

Returns the menu for the given `id`.

#### <a name="previewDialog"></a>bool **previewDialog**([Widget](../script/widget.md) dialog)

Preview the result of the conversion RC->UI

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
