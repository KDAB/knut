#pragma once

#include "document.h"
#include "settings.h"

#include "rccore/rcfile.h"

namespace Core {

class RcDocument : public Document
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ isValid NOTIFY fileNameChanged)
    Q_PROPERTY(QVector<RcCore::Asset> assets READ assets NOTIFY dataChanged)
    Q_PROPERTY(QVector<RcCore::Action> actions READ actions NOTIFY dataChanged)
    Q_PROPERTY(QVector<RcCore::ToolBar> toolBars READ toolBars NOTIFY dataChanged)
    Q_PROPERTY(QVector<RcCore::Menu> menus READ menus NOTIFY dataChanged)
    Q_PROPERTY(QList<RcCore::String> strings READ strings NOTIFY dataChanged)
    Q_PROPERTY(QStringList dialogIds READ dialogIds NOTIFY dataChanged)
    Q_PROPERTY(QStringList menuIds READ menuIds NOTIFY dataChanged)
    Q_PROPERTY(QStringList acceleratorIds READ acceleratorIds NOTIFY dataChanged)
    Q_PROPERTY(QStringList toolbarIds READ toolbarIds NOTIFY dataChanged)
    Q_PROPERTY(QStringList stringIds READ stringIds NOTIFY dataChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QStringList languages READ languages NOTIFY languagesChanged)

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
        // Dialog conversion
        UpdateHierarchy = RcCore::Widget::UpdateHierarchy,
        UpdateGeometry = RcCore::Widget::UpdateGeometry,
        UseIdForPixmap = RcCore::Widget::UseIdForPixmap,
    };
    Q_ENUM(ConversionFlag)
    Q_DECLARE_FLAGS(ConversionFlags, ConversionFlag)
    Q_FLAG(ConversionFlags)

    static inline constexpr char DefaultLanguage[] = "[default]";

public:
    explicit RcDocument(QObject *parent = nullptr);

    bool isValid() const;

    QVector<RcCore::Asset> assets() const;
    QVector<RcCore::Action> actions() const;
    Q_INVOKABLE RcCore::Action action(const QString &id) const;
    Q_INVOKABLE RcCore::ActionList actionsFromMenu(const QString &menuId) const;
    Q_INVOKABLE RcCore::ActionList actionsFromToolbar(const QString &toolBarId) const;

    QVector<RcCore::ToolBar> toolBars() const;
    Q_INVOKABLE RcCore::ToolBar toolBar(const QString &id) const;

    Q_INVOKABLE RcCore::Widget dialog(const QString &id, int flags = DEFAULT_VALUE(ConversionFlags, RcDialogFlags),
                                      double scaleX = DEFAULT_VALUE(double, RcDialogScaleX),
                                      double scaleY = DEFAULT_VALUE(double, RcDialogScaleY)) const;

    QVector<RcCore::Menu> menus() const;
    Q_INVOKABLE RcCore::Menu menu(const QString &id) const;

    QStringList dialogIds() const;
    QStringList menuIds() const;
    QStringList acceleratorIds() const;
    QStringList toolbarIds() const;
    QStringList stringIds() const;

    QList<RcCore::String> strings() const;
    Q_INVOKABLE QString string(const QString &id) const;

    QString language() const;
    void setLanguage(const QString &language);
    QStringList languages() const;

    const RcCore::RcFile &file() const;

public slots:
    void convertAssets(int flags = DEFAULT_VALUE(ConversionFlag, RcAssetFlags));
    void convertActions(int flags = DEFAULT_VALUE(ConversionFlags, RcAssetFlags));
    bool writeAssetsToImage(int flags = DEFAULT_VALUE(ConversionFlags, RcAssetColors));
    bool writeAssetsToQrc(const QString &fileName);
    bool writeDialogToUi(const RcCore::Widget &dialog, const QString &fileName);
    void previewDialog(const RcCore::Widget &dialog) const;
    void mergeAllLanguages(const QString &language = DefaultLanguage);
    void mergeLanguages();

signals:
    void languageChanged();
    void languagesChanged();
    void dataChanged();

protected:
    bool doSave(const QString &fileName) override;
    bool doLoad(const QString &fileName) override;

private:
    const RcCore::Data &data() const;
    bool isDataValid() const;

    RcCore::RcFile m_rcFile;
    QString m_language;
    QVector<RcCore::Asset> m_cacheAssets;
    QVector<RcCore::Action> m_cacheActions;
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
                                 {RcDocument::UpdateHierarchy, "UpdateHierarchy"},
                                 {RcDocument::UpdateGeometry, "UpdateGeometry"},
                                 {RcDocument::UseIdForPixmap, "UseIdForPixmap"},
                             })

} // namespace Core

Q_DECLARE_OPERATORS_FOR_FLAGS(Core::RcDocument::ConversionFlags)
