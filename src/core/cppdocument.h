#pragma once

#include "lspdocument.h"

namespace Core {

class CppDocument : public LspDocument
{
    Q_OBJECT
    Q_PROPERTY(bool isHeader READ isHeader CONSTANT)

public:
    explicit CppDocument(QObject *parent = nullptr);

    enum Position { StartOfMethod, EndOfMethod };
    Q_ENUM(Position)

    bool isHeader() const;

    Q_INVOKABLE QString correspondingHeaderSource() const;

public slots:
    Core::CppDocument *openHeaderSource();

    bool insertCodeInMethod(const QString &methodName, QString code,
                            Core::CppDocument::Position insertAt = Position::StartOfMethod);
    bool insertForwardDeclaration(const QString &fwddecl);
    void commentSelection();

    QVariantMap mfcExtractDDX(const QString &className);

    int gotoBlockStart(int count = 1);
    int gotoBlockEnd(int count = 1);

    int selectBlockStart(int count = 1);
    int selectBlockEnd(int count = 1);

private:
    int moveBlock(int startPos, QTextCursor::MoveOperation direction);
};

} // namespace Core
