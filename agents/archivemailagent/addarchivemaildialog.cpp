/*
   SPDX-FileCopyrightText: 2012-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "addarchivemaildialog.h"
#include "archivemailrangewidget.h"
#include "widgets/formatcombobox.h"
#include "widgets/unitcombobox.h"

#include <MailCommon/FolderRequester>

#include <KLineEdit>
#include <KLocalizedString>
#include <KSeparator>
#include <KUrlRequester>
#include <QIcon>
#include <QSpinBox>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AddArchiveMailDialog::AddArchiveMailDialog(ArchiveMailInfo *info, QWidget *parent)
    : QDialog(parent)
    , mFolderRequester(new MailCommon::FolderRequester(this))
    , mFormatComboBox(new FormatComboBox(this))
    , mUnits(new UnitComboBox(this))
    , mRecursiveCheckBox(new QCheckBox(i18n("Archive all subfolders"), this))
    , mPath(new KUrlRequester(this))
    , mDays(new QSpinBox(this))
    , mMaximumArchive(new QSpinBox(this))
    , mArchiveMailRangeWidget(new ArchiveMailRangeWidget(this))
    , mInfo(info)
{
    if (info) {
        setWindowTitle(i18nc("@title:window", "Modify Archive Mail"));
    } else {
        setWindowTitle(i18nc("@title:window", "Add Archive Mail"));
    }
    setModal(true);
    setWindowIcon(QIcon::fromTheme(QStringLiteral("kmail")));

    auto topLayout = new QVBoxLayout(this);

    auto mainLayout = new QGridLayout;
    mainLayout->setContentsMargins({});

    int row = 0;

    auto folderLabel = new QLabel(i18n("&Folder:"), this);
    mainLayout->addWidget(folderLabel, row, 0);
    mFolderRequester->setObjectName(QStringLiteral("folder_requester"));
    mFolderRequester->setMustBeReadWrite(false);
    mFolderRequester->setNotAllowToCreateNewFolder(true);
    connect(mFolderRequester, &MailCommon::FolderRequester::folderChanged, this, &AddArchiveMailDialog::slotFolderChanged);
    if (info) { // Don't autorize to modify folder when we just modify item.
        mFolderRequester->setEnabled(false);
    }
    folderLabel->setBuddy(mFolderRequester);
    mainLayout->addWidget(mFolderRequester, row, 1);
    ++row;

    auto formatLabel = new QLabel(i18n("Format:"), this);
    formatLabel->setObjectName(QStringLiteral("label_format"));
    mainLayout->addWidget(formatLabel, row, 0);

    mainLayout->addWidget(mFormatComboBox, row, 1);
    ++row;

    mRecursiveCheckBox->setObjectName(QStringLiteral("recursive_checkbox"));
    mainLayout->addWidget(mRecursiveCheckBox, row, 0, 1, 2, Qt::AlignLeft);
    mRecursiveCheckBox->setChecked(true);
    ++row;

    auto pathLabel = new QLabel(i18n("Path:"), this);
    mainLayout->addWidget(pathLabel, row, 0);
    pathLabel->setObjectName(QStringLiteral("path_label"));
    mPath->lineEdit()->setTrapReturnKey(true);
    connect(mPath, &KUrlRequester::textChanged, this, &AddArchiveMailDialog::slotUpdateOkButton);
    mPath->setMode(KFile::Directory);
    mainLayout->addWidget(mPath);
    ++row;

    auto dateLabel = new QLabel(i18n("Backup each:"), this);
    dateLabel->setObjectName(QStringLiteral("date_label"));
    mainLayout->addWidget(dateLabel, row, 0);

    auto hlayout = new QHBoxLayout;
    mDays->setMinimum(1);
    mDays->setMaximum(3600);
    hlayout->addWidget(mDays);

    hlayout->addWidget(mUnits);

    mainLayout->addLayout(hlayout, row, 1);
    ++row;

    auto maxCountlabel = new QLabel(i18n("Maximum number of archive:"), this);
    mainLayout->addWidget(maxCountlabel, row, 0);
    mMaximumArchive->setMinimum(0);
    mMaximumArchive->setMaximum(9999);
    mMaximumArchive->setSpecialValueText(i18n("unlimited"));
    maxCountlabel->setBuddy(mMaximumArchive);
    mainLayout->addWidget(mMaximumArchive, row, 1);
    ++row;

    mArchiveMailRangeWidget->setObjectName(QStringLiteral("mArchiveMailRangeWidget"));
    mainLayout->addWidget(mArchiveMailRangeWidget, row, 0, 1, 2);
    ++row;

    mainLayout->addWidget(new KSeparator, row, 0, 1, 2);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), row, 0);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AddArchiveMailDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AddArchiveMailDialog::reject);

    if (mInfo) {
        load(mInfo);
    } else {
        mOkButton->setEnabled(false);
    }
    topLayout->addLayout(mainLayout);
    topLayout->addWidget(buttonBox);

    // Make it a bit bigger, else the folder requester cuts off the text too early
    resize(500, minimumSize().height());
}

AddArchiveMailDialog::~AddArchiveMailDialog() = default;

void AddArchiveMailDialog::load(ArchiveMailInfo *info)
{
    mPath->setUrl(info->url());
    mRecursiveCheckBox->setChecked(info->saveSubCollection());
    mFolderRequester->setCollection(Akonadi::Collection(info->saveCollectionId()));
    mFormatComboBox->setFormat(info->archiveType());
    mDays->setValue(info->archiveAge());
    mUnits->setUnit(info->archiveUnit());
    mMaximumArchive->setValue(info->maximumArchiveCount());
    const bool useRange{info->useRange()};
    mArchiveMailRangeWidget->setRangeEnabled(useRange);
    if (useRange) {
        mArchiveMailRangeWidget->setRange(info->range());
    }
    slotUpdateOkButton();
}

ArchiveMailInfo *AddArchiveMailDialog::info()
{
    if (!mInfo) {
        mInfo = new ArchiveMailInfo();
    }
    mInfo->setSaveSubCollection(mRecursiveCheckBox->isChecked());
    mInfo->setArchiveType(mFormatComboBox->format());
    mInfo->setSaveCollectionId(mFolderRequester->collection().id());
    mInfo->setUrl(mPath->url());
    mInfo->setArchiveAge(mDays->value());
    mInfo->setArchiveUnit(mUnits->unit());
    mInfo->setMaximumArchiveCount(mMaximumArchive->value());
    const bool isRangeEnabled = mArchiveMailRangeWidget->isRangeEnabled();
    mInfo->setUseRange(isRangeEnabled);
    if (isRangeEnabled) {
        mInfo->setRange(mArchiveMailRangeWidget->range());
    }
    return mInfo;
}

void AddArchiveMailDialog::slotUpdateOkButton()
{
    const bool valid = (!mPath->lineEdit()->text().trimmed().isEmpty() && !mPath->url().isEmpty() && mFolderRequester->collection().isValid());
    mOkButton->setEnabled(valid);
}

void AddArchiveMailDialog::slotFolderChanged(const Akonadi::Collection &collection)
{
    Q_UNUSED(collection)
    slotUpdateOkButton();
}

void AddArchiveMailDialog::setArchiveType(MailCommon::BackupJob::ArchiveType type)
{
    mFormatComboBox->setFormat(type);
}

MailCommon::BackupJob::ArchiveType AddArchiveMailDialog::archiveType() const
{
    return mFormatComboBox->format();
}

void AddArchiveMailDialog::setRecursive(bool b)
{
    mRecursiveCheckBox->setChecked(b);
}

bool AddArchiveMailDialog::recursive() const
{
    return mRecursiveCheckBox->isChecked();
}

void AddArchiveMailDialog::setSelectedFolder(const Akonadi::Collection &collection)
{
    mFolderRequester->setCollection(collection);
}

Akonadi::Collection AddArchiveMailDialog::selectedFolder() const
{
    return mFolderRequester->collection();
}

QUrl AddArchiveMailDialog::path() const
{
    return mPath->url();
}

void AddArchiveMailDialog::setPath(const QUrl &url)
{
    mPath->setUrl(url);
}

void AddArchiveMailDialog::setMaximumArchiveCount(int max)
{
    mMaximumArchive->setValue(max);
}

int AddArchiveMailDialog::maximumArchiveCount() const
{
    return mMaximumArchive->value();
}

#include "moc_addarchivemaildialog.cpp"
