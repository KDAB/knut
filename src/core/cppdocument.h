#pragma once

#include "lspdocument.h"
#include "messagemap.h"

#include <memory>

#ifndef Q_MOC_RUN
#define API_EXECUTOR
#endif

namespace Core {

class CppDocument : public LspDocument
{
    Q_OBJECT
    Q_PROPERTY(bool isHeader READ isHeader CONSTANT)

public:
    explicit CppDocument(QObject *parent = nullptr);
    virtual ~CppDocument();

    enum Position { StartOfMethod, EndOfMethod };
    Q_ENUM(Position)

    enum AccessSpecifier { Public, Protected, Private };
    Q_ENUM(AccessSpecifier)

    const QHash<AccessSpecifier, QString> accessSpecifierMap = {{AccessSpecifier::Public, "public"},
                                                                {AccessSpecifier::Private, "private"},
                                                                {AccessSpecifier::Protected, "protected"}};

    bool isHeader() const;

    Q_INVOKABLE QString correspondingHeaderSource() const;

    Q_INVOKABLE QVector<Core::QueryMatch> queryMethodDefinition(const QString &scope, const QString &functionName);
    Q_INVOKABLE QVector<Core::QueryMatch> queryFunctionCall(const QString &functionName,
                                                            const QVector<QString> &argumentCaptures);
    Q_INVOKABLE QVector<Core::QueryMatch> queryFunctionCall(const QString &functionName);

    Q_INVOKABLE QVariantMap mfcExtractDDX(const QString &className);
    Q_INVOKABLE Core::MessageMap mfcExtractMessageMap(const QString &className = "");
    Q_INVOKABLE bool mfcReplaceAfxMsgDeclaration(const QString &afxMsgName, const QString &newDeclaration);
    Q_INVOKABLE QVector<Core::QueryMatch> mfcFindAfxMsgDeclaration(const QString &afxMsgName);

public slots:
    Core::CppDocument *openHeaderSource();

    bool insertCodeInMethod(const QString &methodName, QString code,
                            Core::CppDocument::Position insertAt = Position::StartOfMethod);
    API_EXECUTOR bool insertForwardDeclaration(const QString &forwardDeclaration);
    void commentSelection();

    int gotoBlockStart(int count = 1);
    int gotoBlockEnd(int count = 1);

    int selectBlockStart(int count = 1);
    int selectBlockEnd(int count = 1);
    int selectBlockUp(int count = 1);

    void toggleSection();

    API_EXECUTOR bool addMember(const QString &member, const QString &className,
                                Core::CppDocument::AccessSpecifier specifier);
    API_EXECUTOR bool addMethodDeclaration(const QString &method, const QString &className,
                                           Core::CppDocument::AccessSpecifier specifier);
    API_EXECUTOR bool addMethodDefinition(const QString &method, const QString &className, const QString &body = "");
    bool addMethod(const QString &declaration, const QString &className, Core::CppDocument::AccessSpecifier specifier,
                   const QString &body = "");
    API_EXECUTOR bool insertInclude(const QString &include, bool newGroup = false);
    API_EXECUTOR bool removeInclude(const QString &include);
    void deleteMethod();
    API_EXECUTOR void deleteMethod(const QString &method, const QString &signature);
    void deleteMethod(const QString &methodName);

private:
    QVector<Core::QueryMatch> internalQueryFunctionCall(const QString &functionName, const QString &argumentsQuery);

    enum class MemberOrMethodAdditionResult { Success, ClassNotFound };
    MemberOrMethodAdditionResult addMemberOrMethod(const QString &memberInfo, const QString &className,
                                                   Core::CppDocument::AccessSpecifier specifier);
    RangeMark findClassBody(const QString &className);

    void deleteMethodLocal(const QString &methodName, const QString &signature = "");

    int moveBlock(int startPos, QTextCursor::MoveOperation direction);

    bool addSpecifierSection(const QString &memberInfoText, const QString &className,
                             Core::CppDocument::AccessSpecifier specifier);

    friend class IncludeHelper;
};

} // namespace Core
