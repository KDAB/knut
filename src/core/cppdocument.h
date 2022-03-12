#pragma once

#include "lspdocument.h"

namespace Core {

class CppDocument : public LspDocument
{
    Q_OBJECT
    Q_PROPERTY(bool isHeader READ isHeader CONSTANT)

public:
    explicit CppDocument(QObject *parent = nullptr);

    bool isHeader() const;

    Q_INVOKABLE QString correspondingHeaderSource() const;

public slots:
    Core::CppDocument *openHeaderSource();

    bool insertForwardDeclaration(const QString &fwddecl);

    QVariantMap mfcExtractDDX(const QString &className);
};

} // namespace Core
