/*
   SPDX-FileCopyrightText: 2012-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "archivemailinfo.h"
#include <Akonadi/Collection>
#include <MailCommon/BackupJob>
#include <QDialog>
class QUrl;
class QCheckBox;
class KUrlRequester;
class QSpinBox;
class QPushButton;

class FormatComboBox;
class UnitComboBox;
class ArchiveMailRangeWidget;
namespace MailCommon
{
class FolderRequester;
}

class AddArchiveMailDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddArchiveMailDialog(ArchiveMailInfo *info, QWidget *parent = nullptr);
    ~AddArchiveMailDialog() override;

    void setArchiveType(MailCommon::BackupJob::ArchiveType type);
    [[nodiscard]] MailCommon::BackupJob::ArchiveType archiveType() const;

    void setRecursive(bool b);
    [[nodiscard]] bool recursive() const;

    void setSelectedFolder(const Akonadi::Collection &collection);
    [[nodiscard]] Akonadi::Collection selectedFolder() const;

    [[nodiscard]] QUrl path() const;
    void setPath(const QUrl &);

    ArchiveMailInfo *info();

    void setMaximumArchiveCount(int);

    [[nodiscard]] int maximumArchiveCount() const;

private:
    void slotFolderChanged(const Akonadi::Collection &);
    void slotUpdateOkButton();
    void load(ArchiveMailInfo *info);
    MailCommon::FolderRequester *const mFolderRequester;
    FormatComboBox *const mFormatComboBox;
    UnitComboBox *const mUnits;
    QCheckBox *const mRecursiveCheckBox;
    KUrlRequester *const mPath;
    QSpinBox *const mDays;
    QSpinBox *const mMaximumArchive;
    ArchiveMailRangeWidget *const mArchiveMailRangeWidget;

    ArchiveMailInfo *mInfo = nullptr;
    QPushButton *mOkButton = nullptr;
};
