#include "optionsdialog.h"
#include "ui_optionsdialog.h"

#include "guisettings.h"

#ifdef USE_SYNTAX_HIGHLIGHTING
#include <repository.h>
#include <theme.h>
#endif

#include <algorithm>

namespace Gui {

OptionsDialog::OptionsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OptionsDialog)
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
    initialize();

    auto setStyle = [](int idx) {
        GuiSettings::instance()->setStyle(static_cast<GuiSettings::Style>(idx));
    };
    connect(ui->styleCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, setStyle);

    auto setTheme = [this](int idx) {
        GuiSettings::instance()->setTheme(idx == 0 ? "" : ui->themeCombo->currentText());
    };
    connect(ui->themeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, setTheme);

    auto setFontFamily = [](const QString &text) {
        GuiSettings::instance()->setFontFamily(text);
    };
    connect(ui->fontCombo, &QComboBox::currentTextChanged, this, setFontFamily);

    auto setFontSize = [](const QString &text) {
        GuiSettings::instance()->setFontSize(text.toInt());
    };
    connect(ui->fontSizeCombo, &QComboBox::currentTextChanged, this, setFontSize);

    auto setWordWrap = [](bool checked) {
        GuiSettings::instance()->setWordWrap(checked);
    };
    connect(ui->wordWrapCheck, &QCheckBox::toggled, this, setWordWrap);
}

OptionsDialog::~OptionsDialog() = default;

void OptionsDialog::initialize()
{
    ui->styleCombo->setCurrentIndex(static_cast<int>(GuiSettings::instance()->style()));
    const int themeIdx = ui->themeCombo->findText(GuiSettings::instance()->theme());
    ui->themeCombo->setCurrentIndex(themeIdx == -1 ? 0 : themeIdx);

    ui->fontCombo->setCurrentText(GuiSettings::instance()->fontFamily());
    ui->fontSizeCombo->setCurrentText(QString::number(GuiSettings::instance()->fontSize()));
    ui->wordWrapCheck->setChecked(GuiSettings::instance()->isWordWrap());
}

} // namespace Gui
