/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QDialog>
#include <QJSValue>
#include <QQmlListProperty>
#include <QQmlPropertyMap>
#include <vector>

class ScriptProgressDialog;

namespace Core {

class DynamicObject;

class ScriptDialogItem : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QObject *data READ data CONSTANT)
    Q_PROPERTY(int stepCount READ stepCount WRITE setStepCount NOTIFY stepCountChanged)
    Q_PROPERTY(QString uiFilePath WRITE setUiFilePath READ uiFilePath NOTIFY uiFilePathChanged)
    Q_PROPERTY(bool interactive READ isInteractive WRITE setInteractive NOTIFY interactiveChanged)
    Q_PROPERTY(QQmlListProperty<QObject> childrenData READ childrenData NOTIFY childrenDataChanged FINAL)
    Q_CLASSINFO("DefaultProperty", "childrenData")

public:
    explicit ScriptDialogItem(QWidget *parent = nullptr);

    QObject *data();
    QQmlListProperty<QObject> childrenData();

    QString uiFilePath() const;
    void setUiFilePath(const QString &filePath);

    // This method is used to redraw the application while a script is running
    // Long-running scripts will otherwise block the GUI, which may look like Knut is hung up.
    // This method should be called in regular intervals to ensure visual progress.
    static void updateProgress();

    bool isInteractive() const;
    void setInteractive(bool interactive);

    int stepCount() const;

    Q_INVOKABLE void firstStep(const QString &firstStep);
    Q_INVOKABLE void nextStep(const QString &title);
    Q_INVOKABLE void runSteps(QJSValue generator);

public slots:
    void setStepCount(int stepCount);
    void done(int code) override;

signals:
    void clicked(const QString &name);
    void childrenDataChanged();
    void uiFilePathChanged(const QString &uiFilePath);
    void interactiveChanged(bool interactive);
    void stepCountChanged(int stepCount);
    void scriptFinished();

private:
    void continueScript();
    void abortScript();
    void finishScript();
    void runNextStep();
    void showProgressDialog();
    void cleanupProgressDialog();
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
    ScriptProgressDialog *m_progressDialog = nullptr;
    // Used to track existing progressDialog, in case we need to update the UI
    static inline QVector<ScriptProgressDialog *> m_progressDialogs = {};

    int m_stepCount = 0;
    int m_currentStep = 0;
    QString m_currentStepTitle = "Initialization"; // Set a default title in case someone forgot the first step
    QString m_nextStepTitle;

    std::optional<QJSValue> m_stepGenerator;
    bool m_interactive = true;
};

} // namespace Core
