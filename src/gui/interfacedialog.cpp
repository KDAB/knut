#include "interfacedialog.h"
#include "ui_interfacedialog.h"

#include "interfacesettings.h"

#ifdef USE_SYNTAX_HIGHLIGHTING
#include <repository.h>
#include <theme.h>
#endif

#include <algorithm>

namespace Gui {

InterfaceDialog::InterfaceDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InterfaceDialog)
{
    ui->setupUi(this);

    ui->styleCombo->addItems({"[Defaut]", "Fusion Light", "Fusion Dark"});

#ifdef USE_SYNTAX_HIGHLIGHTING
    KSyntaxHighlighting::Repository repository;
    const auto themes = repository.themes();
    QStringList themeNames = {"[Default]"};
    auto themeToString = [](const auto &theme) {
        return theme.name();
    };
    std::transform(std::begin(themes), std::end(themes), std::back_inserter(themeNames), themeToString);
    ui->themeCombo->addItems(themeNames);
#endif

    auto setStyle = [this](int idx) {
        m_settings->setStyle(static_cast<InterfaceSettings::Style>(idx));
    };
    connect(ui->styleCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, setStyle);

    auto setTheme = [this](int idx) {
        m_settings->setTheme(idx == 0 ? "" : ui->themeCombo->currentText());
    };
    connect(ui->themeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, setTheme);

    auto setFontFamily = [this](const QString &text) {
        m_settings->setFontFamily(text);
    };
    connect(ui->fontCombo, &QComboBox::currentTextChanged, this, setFontFamily);

    auto setFontSize = [this](const QString &text) {
        m_settings->setFontSize(text.toInt());
    };
    connect(ui->fontSizeCombo, &QComboBox::currentTextChanged, this, setFontSize);

    auto setWordWrap = [this](bool checked) {
        m_settings->setWordWrap(checked);
    };
    connect(ui->wordWrapCheck, &QCheckBox::toggled, this, setWordWrap);
}

InterfaceDialog::~InterfaceDialog() = default;

void InterfaceDialog::initialize(InterfaceSettings *settings)
{
    Q_ASSERT(settings);

    m_settings = settings;

    ui->styleCombo->setCurrentIndex(static_cast<int>(settings->style()));
    const int themeIdx = ui->themeCombo->findText(settings->theme());
    ui->themeCombo->setCurrentIndex(themeIdx == -1 ? 0 : themeIdx);

    ui->fontCombo->setCurrentText(settings->fontFamily());
    ui->fontSizeCombo->setCurrentText(QString::number(settings->fontSize()));
    ui->wordWrapCheck->setChecked(settings->isWordWrap());
}

} // namespace Gui
