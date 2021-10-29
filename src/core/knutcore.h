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

    void process(const QCoreApplication &app);
    void process(const QStringList &arguments);

protected:
    virtual void initParser(QCommandLineParser &parser) const;
    virtual void doParse(const QCommandLineParser &parser) const;
};

} // namespace Core
