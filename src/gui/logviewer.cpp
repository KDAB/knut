#include "logviewer.h"

#include <QMetaObject>
#include <QPlainTextEdit>

#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>

#include <mutex>

namespace Gui {

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

LogViewer::LogViewer(QWidget *parent)
    : QPlainTextEdit(parent)
{
    auto logger = spdlog::default_logger();
    logger->sinks().push_back(std::shared_ptr<spdlog::sinks::sink>(new qt_sink_mt(this)));
    setWindowTitle("Log Viewer");
}

LogViewer::~LogViewer() = default;

} // namespace Gui
