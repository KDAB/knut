#pragma once

#include "symbol.h"
#include "textdocument.h"

namespace Core {

class CppDocument : public TextDocument
{
    Q_OBJECT
    Q_PROPERTY(bool isHeader READ isHeader CONSTANT)

public:
    explicit CppDocument(QObject *parent = nullptr);

    bool isHeader() const;

    Q_INVOKABLE QString correspondingHeaderSource() const;

    Q_INVOKABLE QVector<Core::Symbol> symbols() const;

public slots:
    Core::CppDocument *openHeaderSource();

    QVariantMap mfcExtractDDX(const QString &className);
};

} // namespace Core
