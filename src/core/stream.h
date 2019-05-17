#ifndef PARSESTREAM_P_H
#define PARSESTREAM_P_H

#include <QChar>
#include <QString>

class QIODevice;

namespace Parser {

class Stream
{
public:
    explicit Stream(QIODevice *device);
    Stream(const QString &text);

    bool atEnd() const;
    int line() const;

    QChar next();
    QChar peek();

    QString content() const;

private:
    QString m_content;
    int m_pos = 0;
    int m_line = 1;
};

}
#endif // PARSESTREAM_P_H
