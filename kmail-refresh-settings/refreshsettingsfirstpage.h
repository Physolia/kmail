/*
   SPDX-FileCopyrightText: 2019-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QWidget>

class RefreshSettingsFirstPage : public QWidget
{
    Q_OBJECT
public:
    explicit RefreshSettingsFirstPage(QWidget *parent = nullptr);
    ~RefreshSettingsFirstPage() override;
};
