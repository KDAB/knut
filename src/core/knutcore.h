#pragma once

#include <QCommandLineParser>
#include <QObject>

class QCoreApplication;

namespace Core {

class KnutCore : public QObject
{
    Q_OBJECT

public:
    explicit KnutCore(QObject *parent = nullptr);

    void process(const QStringList &arguments);

protected:
    // Used to disambiguate the internal constructor
    struct InternalTag
    {
    };
    explicit KnutCore(InternalTag, QObject *parent = nullptr);

    virtual void initParser(QCommandLineParser &parser) const;
    virtual void doParse(const QCommandLineParser &parser) const;

private:
    void initialize(bool isTesting);

    bool m_initialized = false;
};

} // namespace Core
