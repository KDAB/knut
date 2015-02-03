#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QJsonDocument>

class Document
{
public:
    Document(const QString &fileName);
    ~Document();

private:
    void parse(const QString &fileName);

private:
    // List of dialogs
    QJsonDocument m_dialogs;
};

#endif // DOCUMENT_H
