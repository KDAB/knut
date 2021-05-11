#include "specparser.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

#include <optional>

SpecParser::SpecParser() { }

/*
 * Entry point for parsing, with a state machine to parse depending on the state
 */
Data SpecParser::parse(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    QTextStream stream(&file);

    while (!stream.atEnd()) {
        QString line = stream.readLine().simplified();

        if (line.isEmpty())
            continue;
        if (readLine(line))
            continue;

        switch (state()) {
        case None:
            break;
        case InNotification:
            readNotification(line);
            break;
        case InRequest:
            readRequest(line);
            break;
        case InCode:
            readCode(line);
            break;
        case InComment:
            readComment(line);
            break;
        case InEnum:
            readEnum(line);
            break;
        case InType:
            readType(line);
            break;
        case InInterface:
            readInterface(line);
            break;
        case InInterfaceProperties:
            readInterfaceProperties(line);
            break;
        }
    }
    return m_data;
}

///////////////////////////////////////////////////////////////////////////////
// State handling
///////////////////////////////////////////////////////////////////////////////

void SpecParser::pushState(State newState)
{
    switch (newState) {
    case None:
    case InCode:
    case InComment:
    case InNotification:
    case InRequest:
    case InInterfaceProperties:
        break;
    case InEnum:
        m_enumeration.comment = comment();
        break;
    case InType:
        m_type.comment = comment();
        break;
    case InInterface:
        m_interfaces.push_back({});
        m_interfaces.back().comment = comment();
        break;
    }
    m_comment.clear();
    m_states.push_back(newState);
}

void SpecParser::popState()
{
    switch (state()) {
    case None:
    case InCode:
    case InComment:
    case InInterfaceProperties:
        break;
    case InNotification:
        m_data.notifications.push_back(m_notification);
        m_notification = {};
        break;
    case InRequest:
        m_data.requests.push_back(m_request);
        m_request = {};
        break;
    case InEnum:
        m_data.enumerations.push_back(m_enumeration);
        m_enumeration = {};
        break;
    case InType:
        m_data.types.push_back(m_type);
        m_type = {};
        break;
    case InInterface:
        if (m_interfaces.size() == 1)
            m_data.interfaces.push_back(m_interfaces[0]);
        else
            m_interfaces[m_interfaces.size() - 2].children.push_back(m_interfaces.back());
        m_interfaces.pop_back();
        break;
    }
    m_states.pop_back();
}

///////////////////////////////////////////////////////////////////////////////
// Utility methods
///////////////////////////////////////////////////////////////////////////////

/*
 * Add a dependency to type or interface
 */
void SpecParser::addDependency(const QString &name)
{
    switch (state()) {
    case SpecParser::None:
    case SpecParser::InNotification:
    case SpecParser::InRequest:
    case SpecParser::InCode:
    case SpecParser::InComment:
    case SpecParser::InEnum:
        break;
    case SpecParser::InType:
        m_type.dependencies.append(name);
        break;
    case SpecParser::InInterface:
    case SpecParser::InInterfaceProperties:
        if (m_interfaces.front().name != name)
            m_interfaces.front().dependencies.append(name);
        break;
    }
}

void SpecParser::addDependency(const QStringList &names)
{
    for (const auto &name : names)
        addDependency(name);
}
/*
 * Return the method name based on the current line
 * Used for notifications and requests
 */
QString SpecParser::extractMethod(const QString &line)
{
    auto split = line.split(QRegularExpression(R"([`'])"), Qt::SkipEmptyParts);
    Q_ASSERT_X(split.size() >= 2, "method extractions", line.toLatin1());
    return split.at(1);
}

/*
 * Decode a parameter (for example, "boolean" => "bool"
 * Used for all type of parameters
 */
QString SpecParser::decodeType(QString param)
{
    param = param.simplified();
    if (param.startsWith('(') && param.endsWith(")[]"))
        return QString("std::vector<%1>").arg(decodeType(param.mid(1, param.length() - 4)));

    if (param.startsWith('(') && param.endsWith(")"))
        param = param.mid(1, param.length() - 2).simplified();

    if (param.contains('|')) {
        auto params = param.split('|');
        for (auto &param : params) {
            param = param.simplified();
            param = decodeType(param);
        }
        return QString("std::variant<%1>").arg(params.join(','));
    }

    if (param.endsWith("[]")) {
        param.chop(2);
        return QString("std::vector<%1>").arg(decodeType(param));
    }

    if (param.startsWith("void") | param.startsWith("none") | param.startsWith("null"))
        return "std::nullptr_t";
    if (param == "number")
        return "int";
    if (param == "integer")
        return "int";
    if (param == "uinteger")
        return "unsigned int";
    if (param == "decimal")
        return "int";
    if (param == "boolean")
        return "bool";
    if (param == "string")
        return "std::string";
    if (param == "any" || param == "T" || param == "unknown")
        return "nlohmann::json";

    // If it's a string, don't add to the dependencies
    if (param.startsWith('\''))
        return param;

    addDependency(param);
    return param;
}

/*
 * Extract the parameters based on a line
 * Used for notifications and requests params and result
 */
QString SpecParser::decodeParams(QString line)
{
    int splitIndex = line.indexOf(':');
    line = line.mid(splitIndex + 1).simplified();

    // Extract all params
    auto params = line.split(R"(\|)", Qt::SkipEmptyParts);
    for (auto &param : params) {
        param = param.simplified();
        bool addSquareBrackets = param.endsWith("[]");
        if (param.contains('`')) {
            int i1 = param.indexOf('`');
            int i2 = param.indexOf('`', i1 + 1);
            param = param.mid(i1 + 1, i2 - i1 - 1) + (addSquareBrackets ? "[]" : "");
        } else if (param.contains('\'')) {
            int i1 = param.indexOf('\'');
            int i2 = param.indexOf('\'', i1 + 1);
            param = param.mid(i1 + 1, i2 - i1 - 1) + (addSquareBrackets ? "[]" : "");
        } else {
            int i = param.indexOf(" ");
            param = param.left(i - 1);
        }
        param = decodeType(param);
    }

    if (params.size() == 1)
        return params.first();
    else
        return QString("std::variant<%1>").arg(params.join(','));
}

/*
 * Return the current comment, and clear it
 */
QString SpecParser::comment()
{
    const auto text = m_comment;
    m_comment.clear();
    return text;
}

// read the current line, set the main state
bool SpecParser::readLine(const QString &line)
{
    if (line.startsWith("_Notification_")) {
        pushState(InNotification);
        return true;
    }
    if (line.startsWith("_Request_")) {
        pushState(InRequest);
        return true;
    }
    if (line.startsWith("```typescript")) {
        pushState(InCode);
        return true;
    }
    if (state() == InCode) {
        if (line.startsWith("```")) {
            popState();
            return true;
        }
    }
    if (line.contains("/**")) {
        pushState(InComment);
        readComment(line);
        return true;
    }
    if (line.startsWith("//"))
        return true;

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// Notifications
// _Notification_:
// * method: METHOD_NAME
// * params: PARAMS
///////////////////////////////////////////////////////////////////////////////
QString SpecParser::decodeNotificationParams(const QString &line)
{
    if (m_notification.method == "telemetry/event") {
        Q_ASSERT(line == R"(* params: 'object' \| 'number' \| 'boolean' \| 'string';)");
        return "nlohmann::json";
    }
    return decodeParams(line);
}

// Read a notification (method + params)
void SpecParser::readNotification(const QString &line)
{
    if (line.startsWith("* method:")) {
        m_notification.method = extractMethod(line);
    } else if (line.startsWith("* params:")) {
        m_notification.params = decodeNotificationParams(line);
        popState();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Notifications
// _Request_:
// * method: METHOD_NAME
// * params: PARAMS
// _Response_:
// * result: PARAMS
///////////////////////////////////////////////////////////////////////////////
QString SpecParser::decodeRequestResults(const QString &line)
{
    if (m_request.method == "textDocument/prepareRename" && !m_request.params.isEmpty()) {
        Q_ASSERT(line.startsWith(
            R"(* result: `Range | { range: Range, placeholder: string } | { defaultBehavior: boolean } | null`)"));

        // Create missing interfaces
        Data::Interface renamePlaceHolder;
        renamePlaceHolder.name = "RenamePlaceHolder";
        renamePlaceHolder.properties = {{"range", "Range", ""}, {"placeholder", "std::string", ""}};
        renamePlaceHolder.dependencies.push_back("Range");
        m_data.interfaces.push_back(renamePlaceHolder);
        Data::Interface renameDefaultBehavior;
        renameDefaultBehavior.name = "RenameDefaultBehavior";
        renameDefaultBehavior.properties = {{"defaultBehavior", "bool", ""}};
        m_data.interfaces.push_back(renameDefaultBehavior);

        return "std::variant<Range, RenamePlaceHolder, RenameDefaultBehavior, std::nullptr_t>";
    }
    return decodeParams(line);
}

// Read a request (method + params + results)
void SpecParser::readRequest(const QString &line)
{
    if (line.startsWith("* method:")) {
        m_request.method = extractMethod(line);
        if (m_request.method == "initialize")
            m_request.error = "InitializeError";
    } else if (line.startsWith("* params:")) {
        m_request.params = decodeParams(line);
    } else if (line.startsWith("* result:")) {
        m_request.result = decodeRequestResults(line);
        popState();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Code blocks - read comments, interfaces, enums and types from it
// ```typescript
// CODE
// ```
///////////////////////////////////////////////////////////////////////////////
void SpecParser::readCode(QString line)
{
    if (line.startsWith("export namespace ") || line.startsWith("export enum")) {
        auto words = line.split(' ', Qt::SkipEmptyParts);
        Q_ASSERT(words.count() >= 3);
        pushState(InEnum);
        m_enumeration.name = words.at(2).simplified();
        return;
    }

    if (line.startsWith("type "))
        line = "export " + line;
    if (line.startsWith("export type ")) {
        pushState(InType);
        readType(line);
        return;
    }

    if (line.startsWith("interface "))
        line = "export " + line;
    if (line.startsWith("export interface ")) {
        pushState(InInterface);
        readInterface(line);
        return;
    }
}

void SpecParser::readComment(const QString &line)
{
    m_comment += line + "\n";
    if (line.startsWith("*/") || line.endsWith("*/"))
        popState();
}

///////////////////////////////////////////////////////////////////////////////
// Enums
// [export namespace|export enum] ENUM_NAME {
//     VALUE_NAME : VALUE_VALUE,
//     ...
// }
///////////////////////////////////////////////////////////////////////////////
void SpecParser::readEnum(const QString &line)
{
    static QString valueLine;
    bool closeEnum = false;

    if (line.startsWith('}'))
        closeEnum = true;
    else
        valueLine += ' ' + line;

    if (!valueLine.isEmpty()) {
        int first = 0;
        if (valueLine.endsWith(';') || valueLine.endsWith(',') || closeEnum) {
            if (valueLine.startsWith(" export const"))
                first = 2;
            auto words = valueLine.split(QRegularExpression("[ :=;,]"), Qt::SkipEmptyParts);
            m_enumeration.values.push_back({words.at(first), comment(), words.last()});
            m_enumeration.isString = words.last().startsWith('\'');
            valueLine.clear();
        }
    }

    if (closeEnum) {
        popState();
        valueLine.clear();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Types
// export type NAME = DATATYPE
///////////////////////////////////////////////////////////////////////////////
QString SpecParser::decodeTypeDataType(const QString &dataType)
{
    if (m_type.name == "TraceValue") {
        Q_ASSERT(dataType == "'off' | 'messages' | 'verbose'");
        m_data.enumerations.push_back(
            {"TraceValue",
             "",
             true,
             {{"off", "", "'off'"}, {"messages", "", "'messages'"}, {"verbose", "", "'verbose'"}}});
        return "std::string";
    } else if (m_type.name == "TextDocumentContentChangeEvent") {
        Q_ASSERT(dataType == "{ range: Range; rangeLength?: uinteger; text: string; } | { text: string; }");
        Data::Interface interface;
        interface.name = "TextDocumentContentChangeEvent";
        interface.properties = {
            {"range?", "/**\n* The range of the document that changed.\n*/", "Range"},
            {"rangeLength?",
             "/**\n* The optional length of the range that got replaced.\n*\n* @deprecated use range instead.\n*/",
             "unsigned int"},
            {"text", "/**\n* The new text for the provided range or the full document.\n*/", "std::string"}};
        interface.dependencies.push_back("Range");
        m_data.interfaces.push_back(interface);
        addDependency("TextDocumentContentChangeEvent");
        return "TextDocumentContentChangeEvent";
    } else if (m_type.name == "MarkedString") {
        Q_ASSERT(dataType == "string | { language: string; value: string }");
        Data::Interface interface;
        interface.name = "MarkedStringFull";
        interface.properties = {{"language", "", "std::string"}, {"value", "", "std::string"}};
        m_data.interfaces.push_back(interface);
        addDependency("MarkedStringFull");
        return "std::variant<std::string, MarkedStringFull>";
    }

    // Special cases to handle types from enums
    if (dataType.startsWith('\''))
        return "std::string";
    else if (dataType[0].isNumber())
        return "int";

    return decodeType(dataType);
}

// Read a type
void SpecParser::readType(const QString &line)
{
    static QString typeLine;
    static int typeBraces = 0;
    typeLine += ' ' + line;
    typeBraces += line.count('{') - line.count('}');

    if (typeBraces == 0 && typeLine.endsWith(';')) {
        int equal = typeLine.indexOf('=');
        m_type.name = typeLine.mid(12, equal - 12).simplified();

        QString dataType = typeLine.mid(equal + 1, typeLine.size() - equal - 2).simplified();
        m_type.dataType = decodeTypeDataType(dataType);

        popState();
        typeLine.clear();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Types
// export interface NAME extends EXTEND1, EXTEND2 {
//     PROPERTY_NAME: PROPERTY_TYPE;
//     ...
// }
///////////////////////////////////////////////////////////////////////////////

// Read interface's name
void SpecParser::readInterface(const QString &line)
{
    static QString interfaceLine;
    interfaceLine += interfaceLine.isEmpty() ? line : (' ' + line);

    // ENsure we read multiple lines if needed
    if (interfaceLine.endsWith('{')) {
        auto words = interfaceLine.split(QRegularExpression("[ ,]"), Qt::SkipEmptyParts);
        Q_ASSERT(words.count() >= 3);

        m_interfaces.back().name = words.at(2).simplified().remove("<T>");

        if (words.at(3) == "extends") {
            for (int i = 4; i < words.size() - 1; ++i)
                m_interfaces.back().extends.push_back(words.at(i));
        }
        addDependency(m_interfaces.back().extends);
        interfaceLine.clear();
        pushState(InInterfaceProperties);
    }

    if (line.startsWith('}')) {
        interfaceLine.clear();
        popState();
        return;
    }
}

QString SpecParser::decodePropertyDataType(const QString &dataType)
{
    if (m_interfaces.back().name == "WorkspaceEdit") {
        if (m_interfaces.back().properties.back().name == "changes?") {
            Q_ASSERT(dataType == "{ [uri: DocumentUri]: TextEdit[]; }");
            Data::Interface interface;
            interface.name = "ChangesType";
            interface.properties = {{"propertyMap", "std::map<DocumentUri, std::vector<TextEdit>>", ""}};
            m_interfaces.back().children.push_back(interface);
            addDependency({"DocumentUri", "TextEdit"});
            return "ChangesType";
        } else if (m_interfaces.back().properties.back().name == "documentChanges?") {
            Q_ASSERT(dataType
                     == "( TextDocumentEdit[] | (TextDocumentEdit | CreateFile | RenameFile | DeleteFile)[] )");
            addDependency({"TextDocumentEdit", "CreateFile", "RenameFile", "DeleteFile"});
            return "std::variant<std::vector<TextDocumentEdit>,std::vector<std::variant<TextDocumentEdit,CreateFile,"
                   "RenameFile,DeleteFile>>>";
        }
    } else if (m_interfaces.back().name == "WorkspaceEdit" || m_interfaces.back().name == "ParameterInformation") {
        if (m_interfaces.back().properties.back().name == "label") {
            Q_ASSERT(dataType == "string | [uinteger, uinteger]");
            return "std::variant<std::string, std::vector<unsigned int>>";
        }
    }
    return decodeType(dataType);
}

bool SpecParser::specialProperty(const QString &line)
{
    if (m_interfaces.back().name == "ChangeAnnotationsType") {
        if (line == "[id: string /* ChangeAnnotationIdentifier */]: ChangeAnnotation;") {
            auto &properties = m_interfaces.back().properties;
            properties.push_back({"propertyMap", comment(), "std::map<std::string, ChangeAnnotation>"});
            addDependency("ChangeAnnotation");
            return true;
        }
    } else if (m_interfaces.back().name == "FormattingOptions") {
        if (line == "[key: string]: boolean | integer | string;") {
            auto &properties = m_interfaces.back().properties;
            properties.push_back(
                {"propertyMap", comment(), "std::map<std::string, std::variant<bool, int, std::string>>"});
            return true;
        }
    } else if (m_interfaces.back().name == "SemanticTokensOptions"
               || (m_interfaces.back().name == "RequestsType"
                   && m_interfaces.front().name == "SemanticTokensClientCapabilities")) {
        if (line == "range?: boolean | {") {
            auto &properties = m_interfaces.back().properties;
            properties.push_back({"range?", comment(), "std::variant<bool, RangeType>"});
            pushState(InInterface);
            m_interfaces.back().name = "RangeType";
            pushState(InInterfaceProperties);
            return true;
        } else if (line == "full?: boolean | {") {
            auto &properties = m_interfaces.back().properties;
            properties.push_back({"full?", comment(), "std::variant<bool, FullType>"});
            pushState(InInterface);
            m_interfaces.back().name = "FullType";
            pushState(InInterfaceProperties);
            return true;
        }
    }
    return false;
}

// Read interface's properties
void SpecParser::readInterfaceProperties(const QString &line)
{
    if (specialProperty(line))
        return;

    static QString typeLine;

    // Close the interface
    if (line.startsWith('}')) {
        typeLine.clear();
        popState();
        readInterface(line);
        return;
    }

    auto &interface = m_interfaces.back();
    if (typeLine.isEmpty()) {
        // Start by reading the property name, and add it to the interface
        int sepIndex = line.indexOf(':');
        if (sepIndex) {
            typeLine += line.mid(sepIndex + 1).simplified();
            interface.properties.push_back({line.left(sepIndex), comment(), ""});
        }
    } else {
        typeLine += ' ' + line;
    }

    auto &property = interface.properties.back();

    // Decode the type
    if (typeLine.endsWith(';')) {
        typeLine.chop(1);
        property.dataType = decodePropertyDataType(typeLine);
        typeLine.clear();
        return;
    }

    // Handle internal struct
    if (typeLine.startsWith('{')) {
        typeLine.clear();
        pushState(InInterface);
        const QString dataType = property.name[0].toUpper() + property.name.mid(1).remove('?') + "Type";
        m_interfaces.back().name = dataType;
        property.dataType = dataType;
        pushState(InInterfaceProperties);
        return;
    }
}
