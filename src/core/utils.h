#pragma once

#include "string_utils.h"

#include <QHash>
#include <QObject>

namespace Core {

class Utils : public QObject
{
    Q_OBJECT

public:
    enum Case {
        CamelCase = static_cast<int>(::Core::Case::CamelCase),
        PascalCase = static_cast<int>(::Core::Case::PascalCase),
        SnakeCase = static_cast<int>(::Core::Case::SnakeCase),
        UpperCase = static_cast<int>(::Core::Case::UpperCase),
        KebabCase = static_cast<int>(::Core::Case::KebabCase),
        TitleCase = static_cast<int>(::Core::Case::TitleCase),
    };
    Q_ENUM(Case)

    explicit Utils(QObject *parent = nullptr);
    ~Utils() override;

public slots:
    QString getEnv(const QString &varName) const;

    QString getGlobal(const QString &varName) const;
    void setGlobal(const QString &varName, const QString &value);

    void addScriptPath(const QString &path) const;
    void runScript(const QString &path, bool log = false) const;

    void sleep(int msecs) const;

    QString mktemp(const QString &pattern) const;

    QString convertCase(const QString &str, Case from, Case to);

private:
    static QHash<QString, QString> m_globals;
};

}
