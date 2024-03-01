#pragma once

#include <QDialog>
#include <QQmlListProperty>
#include <QQmlPropertyMap>

#include <vector>

class QProgressDialog;

namespace Core {

class DynamicObject;

class ScriptDialogItem : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QObject *data READ data CONSTANT)
    Q_PROPERTY(QString uiFilePath WRITE setUiFilePath READ uiFilePath NOTIFY uiFilePathChanged)
    Q_PROPERTY(bool showProgress READ showProgress WRITE setShowProgress NOTIFY showProgressChanged)
    Q_PROPERTY(QQmlListProperty<QObject> childrenData READ childrenData NOTIFY childrenDataChanged FINAL)
    Q_CLASSINFO("DefaultProperty", "childrenData")

public:
    explicit ScriptDialogItem(QWidget *parent = nullptr);

    QObject *data();
    QQmlListProperty<QObject> childrenData();

    QString uiFilePath() const;
    void setUiFilePath(const QString &filePath);

    bool showProgress();
    void setShowProgress(bool value);
    // This method is used to redraw the application while a script is running
    // Long-running scripts will otherwise block the GUI, which may look like Knut is hung up.
    // This method should be called in regular intervals to ensure visual progress.
    static void updateProgress();

public Q_SLOTS:
    void done(int code) override;

signals:
    void clicked(const QString &name);
    void childrenDataChanged();
    void uiFilePathChanged(const QString &uiFilePath);
    void showProgressChanged(bool showProgress);
    void conversionFinished();

private:
    void startShowingProgress();
    void initializeUiAndData();
    void setUiFile(const QString &fileName);
    void createProperties(QWidget *dialogWidget);
    void changeValue(const QString &key, const QVariant &value);

    static void appendChild(QQmlListProperty<QObject> *list, QObject *obj);
    static QObject *atChild(QQmlListProperty<QObject> *list, qsizetype index);
    static qsizetype countChildren(QQmlListProperty<QObject> *list);
    static void clearChildren(QQmlListProperty<QObject> *list);

private:
    mutable QString m_uiFilePath;
    // needs to be mutable for lazy-initialization
    mutable DynamicObject *m_data;
    std::vector<QObject *> m_children;
    QProgressDialog *m_progressDialog = nullptr;
    bool m_showProgress = false;
};

} // namespace Core
