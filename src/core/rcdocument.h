#pragma once

#include "document.h"
#include "settings.h"

#include "rccore/data.h"

namespace Core {

class RcDocument : public Document
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ isValid NOTIFY fileNameChanged)
    Q_PROPERTY(QVector<RcCore::Asset> assets READ assets NOTIFY fileNameChanged)
    Q_PROPERTY(QVector<RcCore::ToolBar> toolBars READ toolBars NOTIFY fileNameChanged)
    Q_PROPERTY(QVector<RcCore::Menu> menus READ menus NOTIFY fileNameChanged)
    Q_PROPERTY(QStringList dialogIds READ dialogIds NOTIFY fileNameChanged)
    Q_PROPERTY(QStringList menuIds READ menuIds NOTIFY fileNameChanged)
    Q_PROPERTY(QStringList acceleratorIds READ acceleratorIds NOTIFY fileNameChanged)
    Q_PROPERTY(QStringList toolbarIds READ toolbarIds NOTIFY fileNameChanged)
    Q_PROPERTY(QStringList stringIds READ stringIds NOTIFY fileNameChanged)

public:
    enum ConversionFlag {
        NoFlags = 0x0,
        // Asset conversion
        RemoveUnknown = RcCore::Asset::RemoveUnknown,
        SplitToolBar = RcCore::Asset::SplitToolBar,
        ConvertToPng = RcCore::Asset::ConvertToPng,
        // Asset transparent colors
        NoColors = RcCore::Asset::NoColors,
        Gray = RcCore::Asset::Gray,
        Magenta = RcCore::Asset::Magenta,
        BottomLeftPixel = RcCore::Asset::BottomLeftPixel,
        AllColors = Gray | Magenta | BottomLeftPixel,
        // Dialog conversion
        UpdateHierarchy = RcCore::Widget::UpdateHierarchy,
        UpdateGeometry = RcCore::Widget::UpdateGeometry,
        UseIdForPixmap = RcCore::Widget::UseIdForPixmap,
        // All flags
        AllFlags = RemoveUnknown | SplitToolBar | ConvertToPng | UpdateHierarchy | UpdateGeometry | UseIdForPixmap,
    };
    Q_DECLARE_FLAGS(ConversionFlags, ConversionFlag)
    Q_FLAG(ConversionFlags)

public:
    explicit RcDocument(QObject *parent = nullptr);

    bool isValid() const;

    QVector<RcCore::Asset> assets() const;

    QVector<RcCore::ToolBar> toolBars() const;
    Q_INVOKABLE RcCore::ToolBar toolBar(const QString &id) const;

    Q_INVOKABLE RcCore::Widget dialog(const QString &id, int flags = DEFAULT_VALUE(ConversionFlags, "/rc/dialog_flags"),
                                      double scaleX = DEFAULT_VALUE(double, "/rc/dialog_scalex"),
                                      double scaleY = DEFAULT_VALUE(double, "/rc/dialog_scaley")) const;

    QVector<RcCore::Menu> menus() const;
    Q_INVOKABLE RcCore::Menu menu(const QString &id) const;

    QStringList dialogIds() const;
    QStringList menuIds() const;
    QStringList acceleratorIds() const;
    QStringList toolbarIds() const;
    QStringList stringIds() const;

    Q_INVOKABLE QString text(const QString &id) const;

    const RcCore::Data &data() const;
    QWidget *widget() const override;

public slots:
    void convertAssets(int flags = DEFAULT_VALUE(ConversionFlag, "/rc/asset_flags"));
    QVector<RcCore::Action> convertActions(const QStringList &menus, const QStringList &accelerators,
                                           const QStringList &toolBars,
                                           int flags = DEFAULT_VALUE(ConversionFlags, "/rc/asset_flags"));
    bool writeAssetsToImage(int flags = DEFAULT_VALUE(ConversionFlags, "/rc/asset_transparent_colors"));
    bool writeAssetsToQrc(const QString &fileName);
    bool writeDialogToUi(const RcCore::Widget &dialog, const QString &fileName);
    void previewDialog(const RcCore::Widget &dialog) const;

protected:
    bool doSave(const QString &fileName) override;
    bool doLoad(const QString &fileName) override;

private:
    RcCore::Data m_data;
    QVector<RcCore::Asset> m_cacheAssets;
    bool m_valid = false;
};

NLOHMANN_JSON_SERIALIZE_ENUM(RcDocument::ConversionFlag,
                             {
                                 {RcDocument::NoFlags, "NoFlags"},
                                 {RcDocument::RemoveUnknown, "RemoveUnknown"},
                                 {RcDocument::SplitToolBar, "SplitToolBar"},
                                 {RcDocument::ConvertToPng, "ConvertToPng"},
                                 {RcDocument::NoColors, "NoColors"},
                                 {RcDocument::Gray, "Gray"},
                                 {RcDocument::Magenta, "Magenta"},
                                 {RcDocument::BottomLeftPixel, "BottomLeftPixel"},
                                 {RcDocument::AllColors, "AllColors"},
                                 {RcDocument::UpdateHierarchy, "UpdateHierarchy"},
                                 {RcDocument::UpdateGeometry, "UpdateGeometry"},
                                 {RcDocument::UseIdForPixmap, "UseIdForPixmap"},
                                 {RcDocument::AllFlags, "AllFlags"},
                             })

} // namespace Core

Q_DECLARE_OPERATORS_FOR_FLAGS(Core::RcDocument::ConversionFlags)
