#pragma once

#include "lspdocument.h"
#include "messagemap.h"

#include <memory>

namespace Core {

class CppCache;

class CppDocument : public LspDocument
{
    Q_OBJECT
    Q_PROPERTY(bool isHeader READ isHeader CONSTANT)

public:
    explicit CppDocument(QObject *parent = nullptr);
    virtual ~CppDocument();

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
    Core::MessageMap mfcExtractMessageMap(const QString &className = "");

    int gotoBlockStart(int count = 1);
    int gotoBlockEnd(int count = 1);

    int selectBlockStart(int count = 1);
    int selectBlockEnd(int count = 1);
    int selectBlockUp(int count = 1);

    void toggleSection();

    bool insertInclude(const QString &include, bool newGroup = false);
    bool removeInclude(const QString &include);
    void deleteMethod();
    void deleteMethod(const QString &methodName, const QString &signature);
    void deleteMethod(const QString &methodName);

private:
    void deleteMethodLocal(const QString &methodName, const QString &signature = "");

    int moveBlock(int startPos, QTextCursor::MoveOperation direction);

    friend CppCache;
    std::unique_ptr<CppCache> m_cache;
};

} // namespace Core
