#include "logpanel.h"

#include "guisettings.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMetaObject>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QToolButton>

#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>

#include <mutex>

namespace Gui {

class LogHighlighter : public QSyntaxHighlighter
{
public:
    LogHighlighter(QTextDocument *parent = nullptr)
        : QSyntaxHighlighter(parent)
    {
        auto getFormat = [](const QColor &color, const QColor &background = {}) {
            QTextCharFormat format;
            format.setForeground(color);
            if (background.isValid())
                format.setBackground(background);
            return format;
        };
        m_rules = {
            {"[trace]", getFormat(QColor(128, 128, 128))},
            {"[debug]", getFormat(Qt::cyan)},
            {"[info]", getFormat(Qt::green)},
            {"[warning]", getFormat(QColor(255, 220, 0))},
            {"[error]", getFormat(Qt::red)},
            {"[critical]", getFormat(Qt::white, Qt::red)},
        };
    }

protected:
    void highlightBlock(const QString &text) override
    {
        for (const auto &rule : std::as_const(m_rules)) {
            if (int start = text.indexOf(rule.keyword); start != -1)
                setFormat(start + 1, rule.keyword.length() - 2, rule.format);
        }
    }

private:
    struct HighlightingRule
    {
        QString keyword;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> m_rules;
};

template <typename Mutex>
class qt_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
    qt_sink(QPlainTextEdit *textEdit)
        : spdlog::sinks::base_sink<Mutex>()
        , m_textEdit(textEdit)
    {
        Q_ASSERT(textEdit);
    }

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        // In case it's coming from different thread, we can't call appendPlainText directly
        auto string = QString::fromStdString(fmt::to_string(formatted));
        QMetaObject::invokeMethod(m_textEdit, "appendPlainText", Qt::AutoConnection,
                                  Q_ARG(QString, string.simplified()));
    }

    void flush_() override
    {
        // Nothing to do here
    }

private:
    QPlainTextEdit *const m_textEdit;
};
using qt_sink_mt = qt_sink<std::mutex>;

LogPanel::LogPanel(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_toolBar(new QWidget)
{
    setWindowTitle("Log Output");
    setObjectName("LogPanel");

    auto logger = spdlog::default_logger();
    logger->sinks().push_back(std::shared_ptr<spdlog::sinks::sink>(new qt_sink_mt(this)));

    // Setup text edit
    new LogHighlighter(document());
    setWordWrapMode(QTextOption::NoWrap);
    GuiSettings::setupTextEdit(this);

    // Setup titlebar
    auto layout = new QHBoxLayout(m_toolBar);
    layout->setContentsMargins({});
    layout->addWidget(new QLabel("Level:"));

    auto levelCombo = new QComboBox(m_toolBar);
    levelCombo->addItems({"trace", "debug", "info", "warning", "error", "critical"});
    levelCombo->setCurrentIndex(logger->level());
    layout->addWidget(levelCombo);
    QObject::connect(levelCombo, qOverload<int>(&QComboBox::currentIndexChanged), levelCombo, [logger](int index) {
        logger->set_level(static_cast<spdlog::level::level_enum>(index));
    });

    auto clearButton = new QToolButton(m_toolBar);
    clearButton->setText("clear"); // TODO add an icon
    clearButton->setAutoRaise(true);
    layout->addWidget(clearButton);
    QObject::connect(clearButton, &QToolButton::clicked, this, &QPlainTextEdit::clear);
}

LogPanel::~LogPanel() = default;

QWidget *LogPanel::toolBar() const
{
    return m_toolBar;
}

} // namespace Gui
