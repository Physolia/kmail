/*
  SPDX-FileCopyrightText: 2015-2023 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/

#include "potentialphishingemailwarning.h"
#include "potentialphishingdetaildialog.h"
#include <KLocalizedString>
#include <QAction>

PotentialPhishingEmailWarning::PotentialPhishingEmailWarning(QWidget *parent)
    : KMessageWidget(parent)
{
    setVisible(false);
    setCloseButtonVisible(true);
    setMessageType(Warning);
    setWordWrap(true);

    setText(i18n("Found potential phishing email addresses <a href=\"phishingdetails\">(details...)</a>"));

    connect(this, &KMessageWidget::linkActivated, this, &PotentialPhishingEmailWarning::slotShowDetails);
    auto action = new QAction(i18n("Send Now"), this);
    action->setObjectName(QStringLiteral("sendnow"));
    connect(action, &QAction::triggered, this, &PotentialPhishingEmailWarning::sendNow);
    addAction(action);
}

PotentialPhishingEmailWarning::~PotentialPhishingEmailWarning() = default;

void PotentialPhishingEmailWarning::slotShowDetails(const QString &link)
{
    if (link == QLatin1String("phishingdetails")) {
        PotentialPhishingDetailDialog dlg(this);
        dlg.fillList(mPotentialPhishingEmails);
        dlg.exec();
    }
}

void PotentialPhishingEmailWarning::setPotentialPhisingEmail(const QStringList &lst)
{
    mPotentialPhishingEmails = lst;
    if (!mPotentialPhishingEmails.isEmpty()) {
        animatedShow();
    }
}

#include "moc_potentialphishingemailwarning.cpp"
