#include "document.h"

#include <QDebug>
#include <QJsonDocument>

int main(int argc, char *argv[])
{
    Document doc = readFromRcFile(QString(argv[1]));

    qDebug() << createJsonDocument(doc).toJson();

    return 0;
}
