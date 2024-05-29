/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QPlainTextEdit>

namespace Core {

class Gutter;

class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit TextEditor(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    void updateGutterWidth(int);
    void updateGutter(const QRect &rect, int dy);
    void updateCurrentLine();

private:
    friend class Gutter;

    Gutter *m_gutter;
    int m_currentLine = -1;
};

}
