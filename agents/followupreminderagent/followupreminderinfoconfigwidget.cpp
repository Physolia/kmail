/*
   SPDX-FileCopyrightText: 2018-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "followupreminderinfoconfigwidget.h"
#include "followupreminderinfowidget.h"
#include "kmail-version.h"
#include <KAboutData>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QLayout>
namespace
{
static const char myConfigGroupName[] = "FollowUpReminderInfoDialog";
}

FollowUpReminderInfoConfigWidget::FollowUpReminderInfoConfigWidget(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args)
    : Akonadi::AgentConfigurationBase(config, parent, args)
    , mWidget(new FollowUpReminderInfoWidget(parent))
{
    parent->layout()->addWidget(mWidget);

    KAboutData aboutData = KAboutData(QStringLiteral("followupreminderagent"),
                                      i18n("Follow Up Reminder Agent"),
                                      QStringLiteral(KDEPIM_VERSION),
                                      i18n("Follow Up Reminder"),
                                      KAboutLicense::GPL_V2,
                                      i18n("Copyright (C) 2014-%1 Laurent Montel", QStringLiteral("2023")));

    aboutData.addAuthor(i18n("Laurent Montel"), i18n("Maintainer"), QStringLiteral("montel@kde.org"));

    aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));

    setKAboutData(aboutData);
}

FollowUpReminderInfoConfigWidget::~FollowUpReminderInfoConfigWidget() = default;

void FollowUpReminderInfoConfigWidget::load()
{
    mWidget->load();
}

bool FollowUpReminderInfoConfigWidget::save() const
{
    return mWidget->save();
}

QSize FollowUpReminderInfoConfigWidget::restoreDialogSize() const
{
    auto group = config()->group(QLatin1String(myConfigGroupName));
    const QSize size = group.readEntry("Size", QSize(800, 600));
    return size;
}

void FollowUpReminderInfoConfigWidget::saveDialogSize(const QSize &size)
{
    auto group = config()->group(QLatin1String(myConfigGroupName));
    group.writeEntry("Size", size);
}

#include "moc_followupreminderinfoconfigwidget.cpp"
