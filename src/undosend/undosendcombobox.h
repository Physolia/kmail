/*
   SPDX-FileCopyrightText: 2019-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "kmail_private_export.h"
#include <QComboBox>

class KMAILTESTS_TESTS_EXPORT UndoSendCombobox : public QComboBox
{
    Q_OBJECT
public:
    explicit UndoSendCombobox(QWidget *parent = nullptr);
    ~UndoSendCombobox() override;

    [[nodiscard]] int delay() const;
    void setDelay(int val);

private:
    void initialize();
};
