/*
    SPDX-FileCopyrightText: 2023 Laurent Montel <montel@kde.org>
    SPDX-License-Identifier: GPL-2.0-only
*/

#pragma once

#include <Akonadi/Item>
#include <QDebug>
#include <QString>

class HistoryClosedReaderInfo
{
public:
    HistoryClosedReaderInfo();
    ~HistoryClosedReaderInfo();

    Q_REQUIRED_RESULT QString subject() const;
    void setSubject(const QString &newSubject);

    Q_REQUIRED_RESULT Akonadi::Item::Id item() const;
    void setItem(Akonadi::Item::Id newItem);

private:
    QString mSubject;
    Akonadi::Item::Id mItem;
    // TODO add Item::Collection ?
};

QDebug operator<<(QDebug d, const HistoryClosedReaderInfo &t);
