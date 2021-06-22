#pragma once

#include <QMainWindow>

#include "rccore/data.h"

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
    RcCore::Data m_data;
    QString m_fileName;
};
