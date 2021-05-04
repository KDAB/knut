#include "specification.h"

#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

#include <optional>
#include <vector>

struct Data
{
    enum State {
        None,
        InNotification,
        InRequest,
    };

    std::vector<State> state = {None};
    State currentState() const { return *state.rbegin(); }

    QString savedText;
};

// Global structure, this just make the code easier to write as you can always access them
Specification specs;
Data data;

// Reading
///////////////////////////////////////////////////////////////////////////////
// read the current line, set the main state
static void readNone(const QString &line)
{
    if (line.startsWith("_Notification_")) {
        data.state.push_back(Data::InNotification);
        specs.notifications.push_back({});
    }
    if (line.startsWith("_Request_")) {
        data.state.push_back(Data::InRequest);
        specs.requests.push_back({});
    }
}

// Return the method name based on the current line
// Used for notifications and requests
static std::optional<QString> extractMethod(const QString &line)
{
    if (!line.startsWith("* method:"))
        return {};

    auto split = line.split(QRegularExpression(R"([`'])"), Qt::SkipEmptyParts);
    Q_ASSERT_X(split.size() >= 2, "method extractions", line.toLatin1());
    return split.at(1);
}

// Decode a parameter (for example, "boolean" => "bool"
// Used for all type of parameters
static QString decodeType(QString param)
{
    if (param.startsWith('(') && param.endsWith(")[]"))
        return QString("std::vector<%1>").arg(decodeType(param.mid(1, param.length() - 4)));

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
    if (param == "boolean")
        return "bool";
    if (param == "string")
        return "std::string";
    if (param == "any")
        return "nlohmann::json";

    return param;
}

// Return some special parameters (too hard to parse, so it's done case by case
// Will assert if the parameters has changed
static std::optional<QString> specialParams(QString params)
{
    if (data.currentState() == Data::InNotification) {
        const auto &notification = specs.currentNotification();
        if (notification.method == "telemetry/event") {
            Q_ASSERT(params.startsWith(R"('object' \| 'number' \| 'boolean' \| 'string')"));
            return "nlohmann::json";
        }
    } else if (data.currentState() == Data::InRequest) {
        const auto &request = specs.currentRequest();
        if (request.method == "textDocument/prepareRename" && !request.params.isEmpty()) {
            Q_ASSERT(params.startsWith(
                R"(`Range | { range: Range, placeholder: string } | { defaultBehavior: boolean } | null`)"));
            // TODO add RenamePlaceHolder{ range: Range, placeholder: string } struct
            // TODO add RenameDefaultBehavior{ defaultBehavior: boolean }
            return "std::variant<Range, RenamePlaceholder, RenameDefaultBehavior, std::nullptr_t>";
        }
    }
    return {};
}

// Extract the parameters based on a line
// Used for notifications and requests params and result
static std::optional<QString> extractParams(QString line, const QString &type = "params")
{
    if (!line.startsWith(QString("* %1:").arg(type)))
        return {};
    line = line.mid(type.length() + 3).simplified();

    if (auto specialParam = specialParams(line))
        return specialParam.value();

    // Extract all params
    auto params = line.split(R"(\|)");
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

// Read a notification (method + params)
static void readNotification(QString line)
{
    line = line.simplified();
    if (line.isEmpty())
        return;

    auto &notification = specs.currentNotification();

    if (auto method = extractMethod(line))
        notification.method = method.value();
    if (auto params = extractParams(line)) {
        notification.params = params.value();
        data.state.pop_back();
    }
}

// Read a request (method + params + results)
static void readRequest(QString line)
{
    line = line.simplified();
    if (line.isEmpty())
        return;

    auto &request = specs.currentRequest();

    if (auto method = extractMethod(line)) {
        request.method = method.value();
        if (request.method == "initialize")
            request.error = "InitializeError";
    }
    if (auto params = extractParams(line))
        request.params = params.value();
    if (auto result = extractParams(line, "result")) {
        request.result = result.value();
        data.state.pop_back();
    }
}

// Entry point for reading the specification
static void readSpecification(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QTextStream stream(&file);

    while (!stream.atEnd()) {
        QString line = stream.readLine();

        switch (data.currentState()) {
        case Data::None:
            readNone(line);
            break;
        case Data::InNotification:
            readNotification(line);
            break;
        case Data::InRequest:
            readRequest(line);
            break;
        }
    }
}

// Printing
///////////////////////////////////////////////////////////////////////////////
static void printSpecification()
{
    qDebug().noquote() << "Notifications:";
    for (const auto &notification : specs.notifications) {
        qDebug().noquote() << QString("  %1(%2)").arg(notification.method, notification.params);
    }

    qDebug().noquote() << "Requests:";
    for (const auto &request : specs.requests) {
        qDebug().noquote() << QString("  %1(%2) => %3").arg(request.method, request.params, request.result);
    }
}

// Saving
///////////////////////////////////////////////////////////////////////////////
static auto methodToName(const QString &method)
{
    auto names = method.split('/');

    // Remove some specifica method prefix : $, textDocument, window, workspace, client
    if (names.first() == "$" || names.first() == "textDocument" || names.first() == "window"
        || names.first() == "workspace" || names.first() == "client")
        names.removeFirst();

    QString charName = names.first();
    for (int i = 1; i < names.count(); ++i) {
        const auto &name = names.at(i);
        charName += name[0].toUpper() + name.mid(1);
    }

    const QString structName = charName[0].toUpper() + charName.mid(1);
    struct Result
    {
        QString charName;
        QString structName;
    };
    return Result {charName, structName};
}

static void saveNotifications()
{
    QFile file(LSP_SOURCE_PATH "/notifications.h");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;
    QTextStream stream(&file);

    stream << R"(#pragma once

#include "notificationmessage.h"
#include "types.h"

namespace Lsp {
)";

    for (const auto &notification : specs.notifications) {
        auto [charName, structName] = methodToName(notification.method);
        stream << QString(R"(
inline constexpr char %1Name[] = "%2";
struct %3Notification : public NotificationMessage<%1Name, %4>
{
};
)")
                      .arg(charName, notification.method, structName, notification.params);
    }
    stream << "}\n";
}

static void saveRequests()
{
    QFile file(LSP_SOURCE_PATH "/requests.h");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;
    QTextStream stream(&file);

    stream << R"(#pragma once

#include "requestmessage.h"
#include "types.h"

namespace Lsp {
)";

    for (const auto &request : specs.requests) {
        auto [charName, structName] = methodToName(request.method);
        stream << QString(R"(
inline constexpr char %1Name[] = "%2";
struct %3Request : public RequestMessage<%1Name, %4, %5, %6>
{
};
)")
                      .arg(charName, request.method, structName, request.params, request.result, request.error);
    }
    stream << "}\n";
}

int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    readSpecification(":/specification-3-16.md");
    printSpecification();
    saveNotifications();
    saveRequests();
}
