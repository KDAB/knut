#include "docwriter.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include "algorithm"

DocWriter::DocWriter(Data data)
    : m_data(data)
{
}

void DocWriter::saveDocumentation()
{
    // Fill the map between type and filename
    for (const auto &type : m_data.types) {
        const QString fileName = QString("../%1/%2.md").arg(type.qmlModule.toLower(), type.name.toLower());
        m_typeFileMap[type.name] = fileName;

        QDir dir(QString(KNUT_DOC_PATH "/API/%1").arg(type.qmlModule.toLower()));
        if (!dir.exists()) {
            dir.cdUp();
            dir.mkdir(type.qmlModule.toLower());
        }
    }

    std::sort(m_data.types.begin(), m_data.types.end(), [](const auto &t1, const auto &t2) {
        return t1.name < t2.name;
    });
    for (const auto &type : m_data.types)
        writeTypeFile(type);

    QString nav("    - Script API:\n");
    auto keys = m_navMap.keys();
    std::sort(keys.begin(), keys.end());
    for (const auto &module : keys) {
        nav += QString("        - %1 Module:\n").arg(module);
        nav += m_navMap.value(module).join("\n") + "\n";
    }
    qDebug().noquote() << nav;
}

// %1 type name
// %2 type brief
// %3 type module
// rest will be added
static const char TypeFile[] = R"(# %1

%2 [More...](#detailed-description)

```qml
import %3 1.0
```
)";

// %1 type since
static const char SinceTypeFile[] = R"(
<table>
<tr><td>Since:</td><td>Knut %1</td></tr>
</table>
)";

// %1 type since
// %2 type inherits
static const char SinceInheritTypeFile[] = R"(
<table>
<tr><td>Since:</td><td>Knut %1</td></tr>
<tr><td>Inherits:</td><td><a href="%2.html">%2</a></td></tr>
</table>
)";

void DocWriter::writeTypeFile(const Data::TypeBlock &type)
{
    m_navMap[type.qmlModule].push_back(
        QString("             - %1: API/%2/%3.md").arg(type.name, type.qmlModule.toLower(), type.name.toLower()));
    const QString &fileName = QString(KNUT_DOC_PATH "/API/%1/%2.md").arg(type.qmlModule.toLower(), type.name.toLower());
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Can't write file: " << fileName;
        return;
    }
    QTextStream stream(&file);

    stream << QString(TypeFile).arg(type.name, type.brief, type.qmlModule);

    if (!type.since.isEmpty()) {
        if (type.inherits.isEmpty()) {
            stream << QString(SinceTypeFile).arg(type.since);
        } else {
            stream << QString(SinceInheritTypeFile).arg(type.since, type.inherits);
        }
    }

    auto properties = propertyForType(type);
    if (properties.size() || !type.inherits.isEmpty()) {
        stream << "\n## Properties\n\n";
        if (properties.size()) {
            std::sort(properties.begin(), properties.end(), [](const auto &prop1, const auto &prop2) {
                return prop1.name < prop2.name;
            });
            stream << "| | Name |\n|-|-|\n";
            for (const auto &prop : properties)
                stream << QString("|%1|**[%2](#%2)**|\n").arg(typeToString(prop.type), prop.name);
        }
        if (!type.inherits.isEmpty())
            stream << QString("\nInherited properties: [%1 properties](%2#properties)\n")
                          .arg(type.inherits, m_typeFileMap.value(type.inherits));
    }

    auto methods = methodForType(type);
    if (methods.size() || !type.inherits.isEmpty()) {
        stream << "\n## Methods\n\n";
        if (methods.size()) {
            std::sort(methods.begin(), methods.end(), [](const auto &method1, const auto &method2) {
                return method1.methods.front().name < method2.methods.front().name;
            });
            stream << "| | Name |\n|-|-|\n";
            for (const auto &method : methods) {
                for (const auto &def : method.methods)
                    stream << methodToString(def, true);
            }
        }
        if (!type.inherits.isEmpty())
            stream << QString("\nInherited methods: [%1 methods](%2#methods)\n")
                          .arg(type.inherits, m_typeFileMap.value(type.inherits));
    }

    auto qmlSignals = signalForType(type);
    if (qmlSignals.size()) {
        stream << "\n## Signals\n\n";
        if (qmlSignals.size()) {
            std::sort(qmlSignals.begin(), qmlSignals.end(), [](const auto &signal1, const auto &signal2) {
                return signal1.method.name < signal2.method.name;
            });
            stream << "| | Name |\n|-|-|\n";
            for (const auto &signal : qmlSignals)
                stream << methodToString(signal.method, true);
        }
    }

    if (!type.description.isEmpty()) {
        stream << "\n## Detailed Description\n\n";
        stream << type.description;
    }

    if (properties.size()) {
        stream << "\n## Property Documentation\n";
        for (const auto &prop : properties) {
            stream << QString("\n#### <a name=\"%1\"></a>%2 **%1**\n").arg(prop.name, typeToString(prop.type));
            if (!prop.since.isEmpty())
                stream << QString(SinceTypeFile).arg(prop.since);
            if (!prop.description.isEmpty())
                stream << "\n" << prop.description;
        }
    }

    if (methods.size()) {
        stream << "\n## Method Documentation\n";
        for (const auto &method : methods) {
            QStringList defs;
            for (const auto &def : method.methods)
                defs.push_back(methodToString(def, false));
            stream << QString("\n#### <a name=\"%1\"></a>%2\n").arg(method.methods.front().name, defs.join("<br/>"));
            if (!method.since.isEmpty())
                stream << QString(SinceTypeFile).arg(method.since);
            if (!method.description.isEmpty())
                stream << "\n" << method.description;
        }
    }
    if (qmlSignals.size()) {
        stream << "\n## Signal Documentation\n";
        for (const auto &signal : qmlSignals) {
            stream << QString("\n#### <a name=\"%1\"></a>%2\n")
                          .arg(signal.method.name, methodToString(signal.method, false));
            if (!signal.since.isEmpty())
                stream << QString(SinceTypeFile).arg(signal.since);
            if (!signal.description.isEmpty())
                stream << "\n" << signal.description;
        }
    }
}

std::vector<Data::PropertyBlock> DocWriter::propertyForType(const Data::TypeBlock &type) const
{
    std::vector<Data::PropertyBlock> results;
    std::copy_if(m_data.properties.begin(), m_data.properties.end(), std::back_inserter(results),
                 [type](const auto &prop) {
                     return prop.qmlType == type.name;
                 });
#if 0
    if (!type.inherits.isEmpty()) {
        auto it = std::find_if(m_data.types.begin(), m_data.types.end(), [&type](const auto &t) {
            return t.name == type.inherits;
        });
        if (it != m_data.types.end()) {
            const auto inheritProps = propertyForType(*it);
            results.insert(results.end(), inheritProps.begin(), inheritProps.end());
        }
    }
#endif
    return results;
}

std::vector<Data::MethodBlock> DocWriter::methodForType(const Data::TypeBlock &type) const
{
    std::vector<Data::MethodBlock> results;
    std::copy_if(m_data.methods.begin(), m_data.methods.end(), std::back_inserter(results), [type](const auto &method) {
        return method.qmlType == type.name;
    });
#if 0
    if (!type.inherits.isEmpty()) {
        auto it = std::find_if(m_data.types.begin(), m_data.types.end(), [&type](const auto &t) {
            return t.name == type.inherits;
        });
        if (it != m_data.types.end()) {
            const auto inheritMethods = methodForType(*it);
            results.insert(results.end(), inheritMethods.begin(), inheritMethods.end());
        }
    }
#endif
    return results;
}

std::vector<Data::SignalBlock> DocWriter::signalForType(const Data::TypeBlock &type) const
{
    std::vector<Data::SignalBlock> results;
    std::copy_if(m_data.qmlSignals.begin(), m_data.qmlSignals.end(), std::back_inserter(results),
                 [type](const auto &signal) {
                     return signal.qmlType == type.name;
                 });
#if 0
    if (!type.inherits.isEmpty()) {
        auto it = std::find_if(m_data.types.begin(), m_data.types.end(), [&type](const auto &t) {
            return t.name == type.inherits;
        });
        if (it != m_data.types.end()) {
            const auto inheritSignals = signalForType(*it);
            results.insert(results.end(), inheritSignals.begin(), inheritSignals.end());
        }
    }
#endif
    return results;
}

QString DocWriter::methodToString(const Data::QmlMethod &method, bool summary) const
{
    QStringList params;
    for (const auto &param : method.parameters)
        params.push_back(typeToString(param.type) + ' ' + param.name);
    QString returnType = typeToString(method.returnType.simplified());
    if (!returnType.isEmpty())
        returnType += ' ';
    if (summary)
        return QString("|%1|**[%2](#%2)**(%3)|\n").arg(returnType, method.name, params.join(", ").simplified());
    else
        return QString("%1**%2**(%3)").arg(returnType, method.name, params.join(", ").simplified());
}

QString DocWriter::typeToString(QString type) const
{
    // Note: inefficient, but good enough for now
    const auto &keys = m_typeFileMap.keys();
    for (const auto &key : keys) {
        int index = type.indexOf(key);
        while (index != -1) {
            bool doContinue = false;
            if (index != 0 && type[index - 1].isLetterOrNumber())
                doContinue = true;
            if (index + key.length() < type.length() && type[index + key.length()].isLetterOrNumber())
                doContinue = true;
            if (!doContinue)
                type.replace(index, key.length(), QString("[%1](%2)").arg(key, m_typeFileMap.value(key)));
            index = type.indexOf(key, index + key.length());
        }
    }
    return type;
}
