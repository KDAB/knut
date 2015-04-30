#ifndef DOCUMENTCACHER_H
#define DOCUMENTCACHER_H

class QString;
class QJsonObject;

void cacheDocument(const QString &filename,
        const QJsonObject &root);

#endif // DOCUMENTCACHER_H
