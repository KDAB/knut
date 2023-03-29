#pragma once

#include <QMainWindow>

#include "rccore/rcfile.h"

namespace RcUi {
class RcFileView;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void load();

private:
    RcUi::RcFileView *m_view = nullptr;
    RcCore::RcFile m_rcFile;
    QString m_fileName;
};
