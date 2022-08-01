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

    if (param.startsWith("void") || param.startsWith("none") || param.startsWith("null"))
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

// MetaModel parsing
MetaData MetaModelSpecParser::parse(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return {};

    QTextStream in(&file);

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(in.readAll().toStdString());
    } catch (nlohmann::json::exception &exception) {
        Q_UNUSED(exception);
        return {};
    }

    // Default types
    {
        auto type = std::make_shared<MetaData::Type>("DocumentUri");
        type->value = "std::string";
        m_data.types.push_back(std::move(type));
    }

    // Read data
    parseRequests(json.at("requests"));
    parseNotifications(json.at("notifications"));
    parseEnumerations(json.at("enumerations"));
    parseStructures(json.at("structures"));
    parseTypeAliases(json.at("typeAliases"));

    // Handle mixins
    handleMixins();

    return m_data;
}

void MetaModelSpecParser::readSpecialInterface(const MetaData::InterfacePtr &interface)
{
    const auto findItem = [&interface](const auto &name) {
        return std::ranges::find_if(interface->items, [&name](const auto &item) {
            return item->name.startsWith(name);
        });
    };

    if (interface->name == "WorkspaceEdit") {
        static const std::vector<QString> propertyToType = {"changes", "changeAnnotations"};
        for (const auto &propertyName : propertyToType) {
            auto it = findItem(propertyName);
            if (it == interface->items.end())
                continue;

            MetaData::TypePtr child = *it;

            const auto childPropertyType = child->value;
            const auto childName = QString("%1%2Type").arg(child->name[0].toUpper(), child->name.mid(1).remove('?'));

            // Update property value
            child->value = childName;

            // Add a new child type
            child = interface->items.emplace_back(std::make_shared<MetaData::Interface>(childName));

            // Add a property to said child type
            child = child->items.emplace_back(std::make_shared<MetaData::Type>("propertyMap"));
            child->value = childPropertyType;
        }
    } else if (interface->name == "FormattingOptions") {
        const auto &property = interface->items.emplace_back(std::make_shared<MetaData::Type>("propertyMap"));
        property->kind = MetaData::TypeKind::Map;
        property->value = "std::map<std::string, std::variant<bool, int, std::string>>";
    } else if (interface->name == "NotebookDocumentSyncOptions") {
        // Remove one NotebookSelectorType
        auto it = findItem("NotebookSelectorType");
        if (it != interface->items.end())
            interface->items.erase(it);

        // Get the next one and fix that one up
        it = findItem("NotebookSelectorType");
        if (it != interface->items.end()) {
            for (auto &property : (*it)->items) {
                if (!property->name.endsWith('?'))
                    property->name.append('?');
            }
        }

        it = findItem("notebookSelector");
        if (it != interface->items.end())
            (*it)->value = "std::vector<NotebookSelectorType>";

    } else if (interface->name.startsWith("TextDocumentContentChangeEvent_")) {
        if (interface->items.size() >= 3)
            interface->name = "TextDocumentContentChangeEventPartial";
        else
            interface->name = "TextDocumentContentChangeEventFull";
    } else if (interface->name.startsWith("MarkedString_")) {
        // There should only be 1
        interface->name = "MarkedStringFull";
    }
}

MetaData::TypePtr MetaModelSpecParser::readType(const nlohmann::json &object)
{
    if (object.is_null())
        return nullptr;

    const auto addDependency = [this](const auto &name) {
        // Add dependencies until an interface adds it.
        for (auto it = m_typeStack.begin(); it != m_typeStack.end(); ++it) {
            auto type = *it;
            if (type->name != name)
                type->dependencies.append(name);

            if (type->is_interface())
                break;
        }
    };

    const auto typesToString = [](const auto &items) {
        QStringList stringList;
        for (const auto &item : items) {
            stringList.append(item->value);
        }
        return stringList.join(", ");
    };

    auto type = std::make_shared<MetaData::Type>();

    const auto kind = object["kind"].get<std::string>();
    if (kind == "base") {
        type->kind = MetaData::TypeKind::Base;

        const auto name = object["name"].get<std::string>();
        if (name == "void" || name == "null" || name == "none")
            type->value = QString("std::nullptr_t");
        else if (name == "number" || name == "integer")
            type->value = QString("int");
        else if (name == "uinteger")
            type->value = QString("unsigned int");
        else if (name == "decimal")
            type->value = QString("float");
        else if (name == "boolean")
            type->value = QString("bool");
        else if (name == "string")
            type->value = QString("std::string");
        else if (name == "any" || name == "T" || name == "unknown" || name == "LSPObject" || name == "LSPAny")
            type->value = QString("nlohmann::json");
        else
            type->value = QString::fromStdString(name);
    } else if (kind == "reference") {
        type->kind = MetaData::TypeKind::Reference;

        const auto name = object["name"].get<std::string>();
        if (name == "any" || name == "T" || name == "unknown" || name == "LSPObject" || name == "LSPAny")
            type->value = QString("nlohmann::json");
        else {
            const auto outName = QString::fromStdString(name);
            addDependency(outName);

            type->value = outName;
        }
    } else if (kind == "array") {
        type->kind = MetaData::TypeKind::Array;

        type->items.push_back(readType(object["element"]));

        type->value = QString("std::vector<%1>").arg(typesToString(type->items));
    } else if (kind == "map") {
        type->kind = MetaData::TypeKind::Map;

        type->items.push_back(readType(object["key"]));
        type->items.push_back(readType(object["value"]));

        type->value = QString("std::map<%1>").arg(typesToString(type->items));
    } else if (kind == "or") {
        type->kind = MetaData::TypeKind::Or;
        for (const auto &item : object["items"]) {
            type->items.push_back(readType(item));
        }
        type->value = QString("std::variant<%1>").arg(typesToString(type->items));
    } else if (kind == "and") {
        type->kind = MetaData::TypeKind::And;
        for (const auto &item : object["items"]) {
            type->items.push_back(readType(item));
        }

        // How should these be handled? Spec mentions nothing.
        type->value = typesToString(decltype(type->items) {type->items.front()});
    } else if (kind == "tuple") {
        type->kind = MetaData::TypeKind::Tuple;
        for (const auto &item : object["items"]) {
            type->items.push_back(readType(item));
        }
        type->value = QString("std::tuple<%1>").arg(typesToString(type->items));
    } else if (kind == "literal") {
        type->kind = MetaData::TypeKind::Literal;

        QString name = m_path.back();

        const auto &parent = m_typeStack.back();
        if (parent->is_interface())
            name = QString("%1%2Type").arg(name[0].toUpper(), name.mid(1));
        else {
            static QMap<QString, uint32_t> uniqueLiteralMap;
            name.append(QString("_%1").arg(++uniqueLiteralMap[name]));
        }

        // If parseLiteral return true its a dependency
        if (parseLiteral(object["value"], name)) {
            // The name might have changed, update it
            name = m_data.interfaces.back()->name;
            addDependency(name);
        }

        type->value = name;
    } else if (kind == "stringLiteral") {
        type->kind = MetaData::TypeKind::StringLiteral;
        type->value = QString::fromStdString(object["value"].get<std::string>());
    } else if (kind == "integerLiteral") {
        type->kind = MetaData::TypeKind::IntegerLiteral;
        // Not used at the moment
    } else if (kind == "booleanLiteral") {
        type->kind = MetaData::TypeKind::BooleanLiteral;
        // Not used at the moment
    } else
        Q_UNREACHABLE();

    return type;
}

QString MetaModelSpecParser::readComment(const nlohmann::json &object)
{
    auto documentation = QString::fromStdString(object.get<std::string>());
    if (documentation.contains('\n'))
        documentation = QString("/*\n%1\n*/\n").arg(documentation);
    else if (documentation.endsWith("*/"))
        documentation = QString("/* %1\n").arg(documentation);
    else
        documentation = QString("// %1\n").arg(documentation);

    return documentation;
}

MetaData::TypePtr MetaModelSpecParser::readProperty(const nlohmann::json &object)
{
    auto name = QString::fromStdString(object["name"].get<std::string>());

    // Add to path
    m_path.push_back(name);

    // Create property
    auto property = readType(object["type"]);
    property->name = name;

    // Check for comments/documentation
    if (object.contains("documentation"))
        property->documentation = readComment(object["documentation"]);

    // Is it optional?
    if (object.contains("optional"))
        property->name.append('?');

    // Remove name from path
    m_path.pop_back();

    // Check if the property is a string literal
    if (!property->items.empty()) {
        bool stringLiterals = std::ranges::all_of(property->items, [](const auto &item) {
            return item->kind == MetaData::TypeKind::StringLiteral;
        });

        if (stringLiterals) {
            QString name = property->name;
            name.remove('?');

            // Split list of stringLiterals to separate properties
            for (const auto &item : property->items) {
                item->name = QString("%1_%2").arg(name, item->value);
            }

            property->kind = MetaData::TypeKind::StringLiteral;
            property->value = "std::string";
        }
    }

    return property;
}

void MetaModelSpecParser::parseRequest(const nlohmann::json &object)
{
    auto &request = m_data.requests.emplace_back();
    request.name = QString::fromStdString(object["method"].get<std::string>());
    if (object.contains("result"))
        request.result = readType(object["result"]);

    if (object.contains("params"))
        request.params = readType(object["params"]);

    if (object.contains("partialResult"))
        request.partialResult = readType(object["partialResult"]);
}

void MetaModelSpecParser::parseNotification(const nlohmann::json &object)
{
    auto &notification = m_data.notifications.emplace_back();
    notification.name = QString::fromStdString(object["method"].get<std::string>());
    if (object.contains("params"))
        notification.params = readType(object["params"]);
}

void MetaModelSpecParser::parseStructure(const nlohmann::json &object)
{
    auto &interface = m_data.interfaces.emplace_back(new MetaData::Interface);
    interface->name = QString::fromStdString(object["name"].get<std::string>());

    // Add type to stack
    m_typeStack.push_back(interface);

    // Add name to path
    m_path.push_back(interface->name);

    if (object.contains("properties")) {
        for (const auto &entry : object["properties"]) {
            interface->items.push_back(readProperty(entry));
        }
    }

    if (object.contains("extends")) {
        for (const auto &item : object["extends"]) {
            interface->extends.push_back(readType(item));
        }
    }

    if (object.contains("mixins")) {
        for (const auto &item : object["mixins"]) {
            interface->mixins.push_back(readType(item));
        }
    }

    // Check if interface requires some changes
    readSpecialInterface(interface);

    // Remove name from path
    m_path.pop_back();

    // Remove interface from stack
    m_typeStack.pop_back();

    // Partition items by type (keep order)
    std::stable_partition(interface->items.begin(), interface->items.end(), [](const auto &value) {
        return value->is_interface();
    });
}

void MetaModelSpecParser::parseEnumeration(const nlohmann::json &object)
{
    MetaData::Enumeration enumeration;
    if (object.contains("type")) {
        auto type = readType(object["type"]);
        if (type->value == "std::string")
            enumeration.type = MetaData::Enumeration::String;
        else if (type->value == "int")
            enumeration.type = MetaData::Enumeration::Integer;
        else if (type->value == "unsigned int")
            enumeration.type = MetaData::Enumeration::UInteger;
        else {
            // Ignore enums without a defined type.
            return;
        }
    }

    enumeration.name = QString::fromStdString(object["name"].get<std::string>());
    if (object.contains("documentation"))
        enumeration.documentation = readComment(object["documentation"]);

    if (object.contains("values")) {
        for (const auto &entry : object["values"]) {
            auto &value = enumeration.values.emplace_back();

            value.name = QString::fromStdString(entry["name"].get<std::string>());
            if (entry.contains("documentation"))
                value.documentation = readComment(entry["documentation"]);

            const auto &entryValue = entry["value"];
            switch (enumeration.type) {
            case MetaData::Enumeration::String:
                value.value = QString::fromStdString(entryValue.get<std::string>());
                break;

            case MetaData::Enumeration::Integer:
                value.value = QString::number(entryValue.get<int>());
                break;

            case MetaData::Enumeration::UInteger:
                value.value = QString::number(entryValue.get<unsigned int>());
                break;

            case MetaData::Enumeration::Invalid:
                Q_UNREACHABLE();
            }
        }
    }

    m_data.enumerations.push_back(std::move(enumeration));
}

void MetaModelSpecParser::parseTypeAlias(const nlohmann::json &object)
{
    auto &type = m_data.types.emplace_back(new MetaData::Type);
    type->name = QString::fromStdString(object["name"].get<std::string>());

    // Add type to stack
    m_typeStack.push_back(type);

    // Add name to path
    m_path.push_back(type->name);

    // Check for comments/documentation
    if (object.contains("documentation"))
        type->documentation = readComment(object["documentation"]);

    // Parse type and steal the data
    auto data = readType(object["type"]);
    type->kind = data->kind;
    type->value = std::move(data->value);
    type->items = std::move(data->items);

    // Remove name from path
    m_path.pop_back();

    // Remove type from stack
    m_typeStack.pop_back();
}

bool MetaModelSpecParser::parseLiteral(const nlohmann::json &object, const QString &name)
{
    auto interface = std::make_shared<MetaData::Interface>();
    interface->name = name;

    // Add type to stack
    m_typeStack.push_back(interface);

    // Add name to path
    m_path.push_back(interface->name);

    if (object.contains("properties")) {
        for (const auto &entry : object["properties"]) {
            interface->items.push_back(readProperty(entry));
        }
    }

    // Check if interface requires some changes
    readSpecialInterface(interface);

    // Remove name from path
    m_path.pop_back();

    // Remove type from stack
    m_typeStack.pop_back();

    // Partition items by type (keep order)
    std::stable_partition(interface->items.begin(), interface->items.end(), [](const auto &value) {
        return value->is_interface();
    });

    // Add to parent (if any)
    const auto &parent = m_typeStack.back();
    if (parent->is_interface()) {
        parent->items.push_back(interface);
        return false;
    }

    m_data.interfaces.push_back(interface);
    return true;
}

void MetaModelSpecParser::parseRequests(const nlohmann::json &json)
{
    for (const auto &obj : json) {
        parseRequest(obj);
    }
}

void MetaModelSpecParser::parseNotifications(const nlohmann::json &json)
{
    for (const auto &obj : json) {
        parseNotification(obj);
    }
}

void MetaModelSpecParser::parseStructures(const nlohmann::json &json)
{
    for (const auto &obj : json) {
        parseStructure(obj);
    }
}

void MetaModelSpecParser::parseEnumerations(const nlohmann::json &json)
{
    for (const auto &obj : json) {
        parseEnumeration(obj);
    }
}

void MetaModelSpecParser::parseTypeAliases(const nlohmann::json &json)
{
    for (const auto &obj : json) {
        parseTypeAlias(obj);
    }
}

void MetaModelSpecParser::handleMixins()
{
    const auto findType = [](auto &types, const auto &name) -> MetaData::TypePtr {
        auto it = std::ranges::find_if(types, [&name](const auto &value) {
            return value->name == name;
        });
        if (it != types.end())
            return *it;

        return nullptr;
    };

    for (const auto &interface : m_data.interfaces) {
        for (const auto &mixin : interface->mixins) {
            auto type = findType(m_data.interfaces, mixin->value);
            if (!type)
                type = findType(m_data.types, mixin->value);

            if (type) {
                const auto &items = type->items;
                interface->items.insert(interface->items.end(), items.begin(), items.end());

                interface->dependencies.append(type->dependencies);
            }
        }

        interface->dependencies.removeDuplicates();
    }
}
