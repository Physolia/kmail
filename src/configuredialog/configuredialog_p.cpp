/*
 *   kmail: KDE mail client
 *   SPDX-FileCopyrightText: 2016-2023 Laurent Montel <montel@kde.org>
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */
// configuredialog_p.cpp: classes internal to ConfigureDialog
// see configuredialog.cpp for details.

// my header:
#include "configuredialog_p.h"

// other KMail headers:
#include "settings/kmailsettings.h"

// other KDE headers:
#include <QTabWidget>

// Qt headers:
#include <QShowEvent>
#include <QVBoxLayout>

// Other headers:

ConfigModuleWithTabs::ConfigModuleWithTabs(QObject *parent, const KPluginMetaData &data)
    : ConfigModule(parent, data)
    , mTabWidget(new QTabWidget(widget()))
{
    auto vlay = new QVBoxLayout(widget());
    vlay->setContentsMargins({});
    vlay->addWidget(mTabWidget);
    mWasInitialized = true;
}

void ConfigModuleWithTabs::addTab(ConfigModuleTab *tab, const QString &title)
{
    mTabWidget->addTab(tab, title);
    connect(tab, &ConfigModuleTab::changed, this, [this](bool state) {
        setNeedsSave(state);
    });
}

void ConfigModuleWithTabs::load()
{
    const int numberOfTab = mTabWidget->count();
    for (int i = 0; i < numberOfTab; ++i) {
        auto tab = qobject_cast<ConfigModuleTab *>(mTabWidget->widget(i));
        if (tab) {
            tab->load();
        }
    }
    KCModule::load();
}

void ConfigModuleWithTabs::save()
{
    if (mWasInitialized) {
        KCModule::save();
        const int numberOfTab = mTabWidget->count();
        for (int i = 0; i < numberOfTab; ++i) {
            auto tab = qobject_cast<ConfigModuleTab *>(mTabWidget->widget(i));
            if (tab) {
                tab->save();
            }
        }
    }
}

void ConfigModuleWithTabs::defaults()
{
    auto tab = qobject_cast<ConfigModuleTab *>(mTabWidget->currentWidget());
    if (tab) {
        tab->defaults();
    }
    KCModule::defaults();
}

void ConfigModuleTab::load()
{
    mEmitChanges = false;
    doLoadFromGlobalSettings();
    doLoadOther();
    mEmitChanges = true;
}

void ConfigModuleTab::defaults()
{
    // reset settings which are available via GlobalSettings to their defaults
    // (stolen from KConfigDialogManager::updateWidgetsDefault())
    const bool bUseDefaults = KMailSettings::self()->useDefaults(true);
    doLoadFromGlobalSettings();
    KMailSettings::self()->useDefaults(bUseDefaults);
    // reset other settings to default values
    doResetToDefaultsOther();
}

void ConfigModuleTab::slotEmitChanged()
{
    if (mEmitChanges) {
        Q_EMIT changed(true);
    }
}

#include "moc_configuredialog_p.cpp"
