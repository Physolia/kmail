/*
   SPDX-FileCopyrightText: 2014-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmlaunchexternalcomponent.h"
#include "kmail_debug.h"
#include "newmailnotifierinterface.h"
#include <Akonadi/AgentConfigurationDialog>
#include <Akonadi/AgentManager>
#include <KLocalizedString>
#include <KMessageBox>

#include <MailCommon/FilterManager>

#include <KDialogJobUiDelegate>
#include <KIO/ApplicationLauncherJob>
#include <KIO/CommandLauncherJob>
#include <QPointer>

#include <QProcess>
#include <QStandardPaths>

KMLaunchExternalComponent::KMLaunchExternalComponent(QWidget *parentWidget, QObject *parent)
    : QObject(parent)
    , mParentWidget(parentWidget)
{
}

KMLaunchExternalComponent::~KMLaunchExternalComponent() = default;

void KMLaunchExternalComponent::slotConfigureAutomaticArchiving()
{
    auto agent = Akonadi::AgentManager::self()->instance(QStringLiteral("akonadi_archivemail_agent"));
    if (agent.isValid()) {
        Akonadi::AgentConfigurationDialog dlg(agent, mParentWidget);
        dlg.exec();
    } else {
        KMessageBox::error(mParentWidget, i18n("Archive Mail Agent was not registered."));
    }
}

void KMLaunchExternalComponent::slotConfigureSendLater()
{
    auto agent = Akonadi::AgentManager::self()->instance(QStringLiteral("akonadi_sendlater_agent"));
    if (agent.isValid()) {
        Akonadi::AgentConfigurationDialog dlg(agent, mParentWidget);
        dlg.exec();
    } else {
        KMessageBox::error(mParentWidget, i18n("Send Later Agent was not registered."));
    }
}

void KMLaunchExternalComponent::slotConfigureMailMerge()
{
    auto agent = Akonadi::AgentManager::self()->instance(QStringLiteral("akonadi_mailmerge_agent"));
    if (agent.isValid()) {
        Akonadi::AgentConfigurationDialog dlg(agent, mParentWidget);
        dlg.exec();
    } else {
        KMessageBox::error(mParentWidget, i18n("Mail Merge Agent was not registered."));
    }
}

void KMLaunchExternalComponent::slotConfigureFollowupReminder()
{
    auto agent = Akonadi::AgentManager::self()->instance(QStringLiteral("akonadi_followupreminder_agent"));
    if (agent.isValid()) {
        QPointer<Akonadi::AgentConfigurationDialog> dlg = new Akonadi::AgentConfigurationDialog(agent, mParentWidget);
        dlg->exec();
        delete dlg;
    } else {
        KMessageBox::error(mParentWidget, i18n("Followup Reminder Agent was not registered."));
    }
}

void KMLaunchExternalComponent::slotStartCertManager()
{
    const KService::Ptr service = KService::serviceByDesktopName(QStringLiteral("org.kde.kleopatra"));
    if (service) {
        auto job = new KIO::ApplicationLauncherJob(service);
        job->setUiDelegate(new KDialogJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, mParentWidget->window()));
        job->start();
    } else {
        KMessageBox::error(mParentWidget,
                           i18n("Could not start certificate manager; "
                                "please make sure you have Kleopatra properly installed."),
                           i18nc("@title:window", "KMail Error"));
    }
}

void KMLaunchExternalComponent::slotImportWizard()
{
    const KService::Ptr service = KService::serviceByDesktopName(QStringLiteral("org.kde.akonadiimportwizard"));
    if (service) {
        auto job = new KIO::ApplicationLauncherJob(service);
        job->setUiDelegate(new KDialogJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, mParentWidget->window()));
        job->start();
    } else {
        KMessageBox::error(mParentWidget,
                           i18n("Could not start the import wizard. "
                                "Please make sure you have ImportWizard properly installed."),
                           i18nc("@title:window", "Unable to start import wizard"));
    }
}

void KMLaunchExternalComponent::slotExportData()
{
    const KService::Ptr service = KService::serviceByDesktopName(QStringLiteral("org.kde.pimdataexporter"));
    if (service) {
        auto job = new KIO::ApplicationLauncherJob(service);
        job->setUiDelegate(new KDialogJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, mParentWidget->window()));
        job->start();
    } else {
        KMessageBox::error(mParentWidget,
                           i18n("Could not start \"PIM Data Exporter\" program. "
                                "Please check your installation."),
                           i18nc("@title:window", "Unable to start \"PIM Data Exporter\" program"));
    }
}

void KMLaunchExternalComponent::slotRunAddressBook()
{
    auto job = new KIO::CommandLauncherJob(QStringLiteral("kaddressbook"), {}, this);
    job->setDesktopName(QStringLiteral("org.kde.kaddressbook"));
    job->setUiDelegate(new KDialogJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, mParentWidget->window()));
    job->start();
}

void KMLaunchExternalComponent::slotImport()
{
    const QStringList lst = {QStringLiteral("--mode"), QStringLiteral("manual")};
    const QString path = QStandardPaths::findExecutable(QStringLiteral("akonadiimportwizard"));
    if (path.isEmpty() || !QProcess::startDetached(path, lst)) {
        KMessageBox::error(mParentWidget,
                           i18n("Could not start the ImportWizard. "
                                "Please make sure you have ImportWizard properly installed."),
                           i18nc("@title:window", "Unable to start ImportWizard"));
    }
}

void KMLaunchExternalComponent::slotAccountWizard()
{
    const QString path = QStandardPaths::findExecutable(QStringLiteral("accountwizard"));
    if (path.isEmpty() || !QProcess::startDetached(path, {})) {
        KMessageBox::error(mParentWidget,
                           i18n("Could not start the account wizard. "
                                "Please make sure you have AccountWizard properly installed."),
                           i18nc("@title:window", "Unable to start account wizard"));
    }
}

void KMLaunchExternalComponent::slotFilterLogViewer()
{
    MailCommon::FilterManager::instance()->showFilterLogDialog(static_cast<qlonglong>(mParentWidget->winId()));
}

void KMLaunchExternalComponent::slotShowNotificationHistory()
{
    const auto service = Akonadi::ServerManager::self()->agentServiceName(Akonadi::ServerManager::Agent, QStringLiteral("akonadi_newmailnotifier_agent"));
    auto newMailNotifierInterface =
        new OrgFreedesktopAkonadiNewMailNotifierInterface(service, QStringLiteral("/NewMailNotifierAgent"), QDBusConnection::sessionBus(), this);
    if (!newMailNotifierInterface->isValid()) {
        qCDebug(KMAIL_LOG) << " org.freedesktop.Akonadi.NewMailNotifierAgent not found. Please verify your installation";
    } else {
        newMailNotifierInterface->showNotNotificationHistoryDialog(0); // TODO fix me windid
    }
    delete newMailNotifierInterface;
}

#include "moc_kmlaunchexternalcomponent.cpp"
