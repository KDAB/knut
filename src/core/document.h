#pragma once

#include "json_utils.h"

#include <QObject>

namespace Core {

class Document : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    Q_PROPERTY(bool exists READ exists NOTIFY existsChanged)
    Q_PROPERTY(Type type READ type CONSTANT)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(bool hasChanged READ hasChanged NOTIFY hasChangedChanged)

public:
    enum class Type {
        Cpp,
        Rc,
        Text,
        Ui,
        Image,
        Slint,
        Qml,
    };
    Q_ENUM(Type)

public:
    explicit Document(Type type, QObject *parent = nullptr);

    const QString &fileName() const;
    void setFileName(const QString &newFileName);

    bool exists() const;

    Type type() const;

    const QString &errorString() const;

    bool hasChanged() const;

public slots:
    bool load(const QString &fileName);
    bool save();
    bool saveAs(const QString &fileName);
    void close();

signals:
    void fileNameChanged();
    void existsChanged();
    void errorStringChanged();
    void hasChangedChanged();

protected:
    virtual bool doSave(const QString &fileName) = 0;
    virtual bool doLoad(const QString &fileName) = 0;

    virtual void didOpen() { }
    virtual void didClose() { }

    void setHasChanged(bool newHasChanged);
    void setErrorString(const QString &error);

private:
    QString m_fileName;
    Type m_type;
    QString m_errorString;
    bool m_hasChanged = false;
};

NLOHMANN_JSON_SERIALIZE_ENUM(Document::Type,
                             {{Document::Type::Cpp, "cpp_type"},
                              {Document::Type::Text, "text_type"},
                              {Document::Type::Rc, "rc_type"},
                              {Document::Type::Ui, "ui_type"},
                              {Document::Type::Image, "image_type"},
                              {Document::Type::Slint, "slint_type"},
                              {Document::Type::Qml, "qml_type"}})

} // namespace Core

Q_DECLARE_METATYPE(Core::Document *)
