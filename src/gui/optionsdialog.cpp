#include "optionsdialog.h"
#include "ui_optionsdialog.h"

#include "guisettings.h"

#ifdef USE_SYNTAX_HIGHLIGHTING
#include <repository.h>
#include <theme.h>
#endif

#include <algorithm>

namespace Gui {

OptionsDialog::OptionsDialog(GuiSettings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OptionsDialog)
    , m_settings(settings)
{
    Q_ASSERT(settings);

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
    initialize();

    auto setStyle = [this](int idx) {
        m_settings->setStyle(static_cast<GuiSettings::Style>(idx));
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

OptionsDialog::~OptionsDialog() = default;

void OptionsDialog::initialize()
{
    ui->styleCombo->setCurrentIndex(static_cast<int>(m_settings->style()));
    const int themeIdx = ui->themeCombo->findText(m_settings->theme());
    ui->themeCombo->setCurrentIndex(themeIdx == -1 ? 0 : themeIdx);

    ui->fontCombo->setCurrentText(m_settings->fontFamily());
    ui->fontSizeCombo->setCurrentText(QString::number(m_settings->fontSize()));
    ui->wordWrapCheck->setChecked(m_settings->isWordWrap());
}

} // namespace Gui
