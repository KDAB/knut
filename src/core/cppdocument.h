/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "codedocument.h"
#include "dataexchange.h"
#include "messagemap.h"
#include "symbol.h"

#ifndef Q_MOC_RUN
#define API_EXECUTOR
#endif

namespace Core {

class CppDocument : public CodeDocument
{
    Q_OBJECT
    Q_PROPERTY(bool isHeader READ isHeader CONSTANT)

public:
    explicit CppDocument(QObject *parent = nullptr);
    ~CppDocument() override;

    enum Position { StartOfMethod, EndOfMethod };
    Q_ENUM(Position)

    enum AccessSpecifier { Public, Protected, Private };
    Q_ENUM(AccessSpecifier)

    const QHash<AccessSpecifier, QString> accessSpecifierMap = {{AccessSpecifier::Public, "public"},
                                                                {AccessSpecifier::Private, "private"},
                                                                {AccessSpecifier::Protected, "protected"}};

    bool isHeader() const;

    Q_INVOKABLE QString correspondingHeaderSource() const;

    Q_INVOKABLE void removeLines(const Core::RangeMark &rangeMark);

    Q_INVOKABLE Core::QueryMatch queryClassDefinition(const QString &className);
    Q_INVOKABLE Core::QueryMatchList queryMethodDeclaration(const QString &className, const QString &functionName);
    Q_INVOKABLE Core::QueryMatch queryMember(const QString &className, const QString &memberName);
    Q_INVOKABLE Core::QueryMatchList queryMethodDefinition(const QString &scope, const QString &functionName);
    Q_INVOKABLE Core::QueryMatchList queryFunctionCall(const QString &functionName,
                                                       const QStringList &argumentCaptures);
    Q_INVOKABLE Core::QueryMatchList queryFunctionCall(const QString &functionName);

    Q_INVOKABLE Core::DataExchange mfcExtractDDX(const QString &className);
    Q_INVOKABLE Core::MessageMap mfcExtractMessageMap(const QString &className = "");

    Q_INVOKABLE QStringList keywords() const;
    Q_INVOKABLE QStringList primitiveTypes() const;

    bool changeBaseClass(CppDocument *header, CppDocument *source, const QString &className,
                         const QString &newClassBaseName);

    QList<treesitter::Range> includedRanges() const override;

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

    bool changeBaseClass(const QString &className, const QString &newClassBaseName);

private:
    QList<Core::QueryMatch> internalQueryFunctionCall(const QString &functionName, const QString &argumentsQuery);

    enum class MemberOrMethodAdditionResult { Success, ClassNotFound };
    MemberOrMethodAdditionResult addMemberOrMethod(const QString &memberInfo, const QString &className,
                                                   Core::CppDocument::AccessSpecifier specifier);

    void deleteMethodLocal(const QString &methodName, const QString &signature = "");

    int moveBlock(int startPos, QTextCursor::MoveOperation direction);

    bool addSpecifierSection(const QString &memberInfoText, const QString &className,
                             Core::CppDocument::AccessSpecifier specifier);
    bool changeBaseClassHeader(const QString &className, const QString &originalClassBaseName,
                               const QString &newClassBaseName);
    bool changeBaseClassSource(const QString &className, const QString &originalClassBaseName,
                               const QString &newClassBaseName);
    void changeBaseClassForwardInclude(const QString &originalClassBaseName, const QString &newClassBaseName);

    friend class IncludeHelper;
};

} // namespace Core
