#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QJsonObject>

class Document
{
public:
    Document(const QString &fileName);
    ~Document();

    QString getAsset(const QString &id);
    QJsonObject getDialog(const QString &id);

private:
    void parse(const QString &fileName);

private:
    // List of dialogs
    QJsonObject m_data;
};

#endif // DOCUMENT_H
