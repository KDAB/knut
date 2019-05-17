#ifndef FILESELECTOR_H
#define FILESELECTOR_H

#include <QWidget>

class QLineEdit;

class FileSelector : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)

public:
    explicit FileSelector(QWidget *parent = nullptr);

    QString fileName() const;

public slots:
    void setFilter(const QString &filter);
    void setFileName(const QString &fileName);

signals:
    void fileNameChanged(const QString &fileName);

private:
    void chooseFile();

private:
    QLineEdit *m_lineEdit = nullptr;
    QString m_filter;
};

#endif // FILESELECTOR_H
