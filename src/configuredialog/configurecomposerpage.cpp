/*
  SPDX-FileCopyrightText: 2013-2023 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-only
*/

#include "configurecomposerpage.h"
#include <PimCommon/ConfigureImmutableWidgetUtils>
using namespace PimCommon::ConfigureImmutableWidgetUtils;
#include "kmkernel.h"

#include "configuredialog/configuredialoglistview.h"
#include "kmmainwidget.h"
#include "settings/kmailsettings.h"
#include <MessageComposer/ImageScalingWidget>
#include <MessageComposer/MessageComposerSettings>
#include <MessageCore/MessageCoreSettings>
#include <PimCommon/SimpleStringListEditor>
#include <TemplateParser/CustomTemplates>
#include <TemplateParser/TemplatesConfiguration>
#include <TextAutoCorrectionWidgets/AutoCorrectionWidget>
#include <templateparser/globalsettings_templateparser.h>
#include <templateparser/templatesconfiguration_kfg.h>

#include <KLDAPWidgets/LdapClientSearch>
#include <PimCommonAkonadi/CompletionOrderEditor>
#include <PimCommonAkonadi/RecentAddresses>
using PimCommon::RecentAddresses;

#include "kmail_debug.h"
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluralHandlingSpinBox>
#include <KSeparator>
#include <QHBoxLayout>
#include <QSpinBox>

#include <KConfigGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScrollArea>
#include <QStringEncoder>
#include <QTextCodec>
#include <QVBoxLayout>

#include <PimCommonAkonadi/CompletionConfigureDialog>
#if KDEPIM_ENTERPRISE_BUILD
#include <QComboBox>
#endif
QString ComposerPage::helpAnchor() const
{
    return QStringLiteral("configure-composer");
}
ComposerPage::ComposerPage(QObject *parent, const KPluginMetaData &data)
    : ConfigModuleWithTabs(parent, data)
{
    //
    // "General" tab:
    //
    auto generalTab = new ComposerPageGeneralTab();
    addTab(generalTab, i18nc("General settings for the composer.", "General"));
    addConfig(KMailSettings::self(), generalTab);

    //
    // "Templates" tab:
    //
    auto templatesTab = new ComposerPageTemplatesTab();
    addTab(templatesTab, i18n("Standard Templates"));

    //
    // "Custom Templates" tab:
    //
    auto customTemplatesTab = new ComposerPageCustomTemplatesTab();
    addTab(customTemplatesTab, i18n("Custom Templates"));

    //
    // "Subject" tab:
    //
    auto subjectTab = new ComposerPageSubjectTab();
    addTab(subjectTab, i18nc("Settings regarding the subject when composing a message.", "Subject"));
    addConfig(KMailSettings::self(), subjectTab);

    //
    // "Charset" tab:
    //
    auto charsetTab = new ComposerPageCharsetTab();
    addTab(charsetTab, i18n("Charset"));

    //
    // "Headers" tab:
    //
    auto headersTab = new ComposerPageHeadersTab();
    addTab(headersTab, i18n("Headers"));

    //
    // "Attachments" tab:
    //
    auto attachmentsTab = new ComposerPageAttachmentsTab();
    addTab(attachmentsTab, i18nc("Config->Composer->Attachments", "Attachments"));

    //
    // "autocorrection" tab:
    //
    auto autoCorrectionTab = new ComposerPageAutoCorrectionTab();
    addTab(autoCorrectionTab, i18n("Autocorrection"));

    //
    // "autoresize" tab:
    //
    auto autoImageResizeTab = new ComposerPageAutoImageResizeTab();
    addTab(autoImageResizeTab, i18n("Auto Resize Image"));
}

QString ComposerPageGeneralTab::helpAnchor() const
{
    return QStringLiteral("configure-composer-general");
}

ComposerPageGeneralTab::ComposerPageGeneralTab(QWidget *parent)
    : ConfigModuleTab(parent)
{
    // Main layout
    auto mainLayout = new QVBoxLayout(this);
    auto wrapper = new QWidget;
    auto layout = new QVBoxLayout(wrapper);

    // "Signature" group
    auto groupBox = new QGroupBox(i18nc("@title:group", "Signature"));
    auto groupVBoxLayout = new QVBoxLayout();

    // "Automatically insert signature" checkbox
    mAutoAppSignFileCheck = new QCheckBox(MessageComposer::MessageComposerSettings::self()->autoTextSignatureItem()->label(), this);

    QString helpText = i18n("Automatically insert the configured signature when starting to compose a message");
    mAutoAppSignFileCheck->setToolTip(helpText);
    mAutoAppSignFileCheck->setWhatsThis(helpText);

    connect(mAutoAppSignFileCheck, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    groupVBoxLayout->addWidget(mAutoAppSignFileCheck);

    // "Insert signature above quoted text" checkbox
    mTopQuoteCheck = new QCheckBox(MessageComposer::MessageComposerSettings::self()->prependSignatureItem()->label(), this);
    mTopQuoteCheck->setEnabled(false);

    helpText = i18n("Insert the signature above any quoted text");
    mTopQuoteCheck->setToolTip(helpText);
    mTopQuoteCheck->setWhatsThis(helpText);

    connect(mTopQuoteCheck, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    connect(mAutoAppSignFileCheck, &QAbstractButton::toggled, mTopQuoteCheck, &QWidget::setEnabled);
    groupVBoxLayout->addWidget(mTopQuoteCheck);

    // "Prepend separator to signature" checkbox
    mDashDashCheck = new QCheckBox(MessageComposer::MessageComposerSettings::self()->dashDashSignatureItem()->label(), this);
    mDashDashCheck->setEnabled(false);

    helpText = i18n("Insert the RFC-compliant signature separator (two dashes and a space on a line) before the signature");
    mDashDashCheck->setToolTip(helpText);
    mDashDashCheck->setWhatsThis(helpText);

    connect(mDashDashCheck, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    connect(mAutoAppSignFileCheck, &QAbstractButton::toggled, mDashDashCheck, &QWidget::setEnabled);
    groupVBoxLayout->addWidget(mDashDashCheck);

    // "Remove signature when replying" checkbox
    mStripSignatureCheck = new QCheckBox(TemplateParser::TemplateParserSettings::self()->stripSignatureItem()->label(), this);

    helpText = i18n("When replying, do not quote any existing signature");
    mStripSignatureCheck->setToolTip(helpText);
    mStripSignatureCheck->setWhatsThis(helpText);

    connect(mStripSignatureCheck, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    groupVBoxLayout->addWidget(mStripSignatureCheck);
    groupVBoxLayout->addStretch(1);

    groupBox->setLayout(groupVBoxLayout);
    layout->addWidget(groupBox);

    // "Format" group
    groupBox = new QGroupBox(i18nc("@title:group", "Format"));
    groupVBoxLayout = new QVBoxLayout;

    // "Only quote selected text when replying" checkbox
    mQuoteSelectionOnlyCheck = new QCheckBox(MessageComposer::MessageComposerSettings::self()->quoteSelectionOnlyItem()->label(), this);
    helpText = i18n("When replying, only quote the selected text (instead of the complete message), if there is text selected in the message window.");
    mQuoteSelectionOnlyCheck->setToolTip(helpText);
    mQuoteSelectionOnlyCheck->setWhatsThis(helpText);

    connect(mQuoteSelectionOnlyCheck, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    groupVBoxLayout->addWidget(mQuoteSelectionOnlyCheck);

    // "Use smart quoting" checkbox
    mSmartQuoteCheck = new QCheckBox(TemplateParser::TemplateParserSettings::self()->smartQuoteItem()->label(), this);
    helpText = i18n(
        "When replying, add quote signs in front of all lines of the quoted text, even when the line was created by adding an additional line break while "
        "word-wrapping the text.");
    mSmartQuoteCheck->setToolTip(helpText);
    mSmartQuoteCheck->setWhatsThis(helpText);

    connect(mSmartQuoteCheck, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    groupVBoxLayout->addWidget(mSmartQuoteCheck);

    // "Word wrap at column" checkbox/spinbox
    mWordWrapCheck = new QCheckBox(MessageComposer::MessageComposerSettings::self()->wordWrapItem()->label(), this);

    helpText = i18n("Enable automatic word wrapping at the specified width");
    mWordWrapCheck->setToolTip(helpText);
    mWordWrapCheck->setWhatsThis(helpText);

    mWrapColumnSpin = new QSpinBox(this);
    mWrapColumnSpin->setMaximum(200);
    mWrapColumnSpin->setMinimum(30);
    mWrapColumnSpin->setSingleStep(1);
    mWrapColumnSpin->setValue(78);
    mWrapColumnSpin->setEnabled(false); // since !mWordWrapCheck->isChecked()

    helpText = i18n("Set the text width for automatic word wrapping");
    mWrapColumnSpin->setToolTip(helpText);
    mWrapColumnSpin->setWhatsThis(helpText);

    connect(mWordWrapCheck, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    connect(mWrapColumnSpin, &QSpinBox::valueChanged, this, &ComposerPageGeneralTab::slotEmitChanged);
    // only enable the spinbox if the checkbox is checked
    connect(mWordWrapCheck, &QAbstractButton::toggled, mWrapColumnSpin, &QWidget::setEnabled);

    auto wordWrapWrapper = new QWidget;
    auto wordWrapWrapperLayout = new QHBoxLayout(wordWrapWrapper);
    wordWrapWrapperLayout->setContentsMargins(0, 0, 0, 0);
    wordWrapWrapperLayout->addWidget(mWordWrapCheck);
    wordWrapWrapperLayout->addWidget(mWrapColumnSpin);
    wordWrapWrapperLayout->addStretch();
    groupVBoxLayout->addWidget(wordWrapWrapper);

    // "Reply/Forward using HTML if present" checkbox
    mReplyUsingVisualFormat = new QCheckBox(TemplateParser::TemplateParserSettings::self()->replyUsingVisualFormatItem()->label(), this);
    helpText = i18n(
        "When replying or forwarding, quote the message in the original format it was received. If unchecked, the reply will be as plain text by default.");
    mReplyUsingVisualFormat->setToolTip(helpText);
    mReplyUsingVisualFormat->setWhatsThis(helpText);

    connect(mReplyUsingVisualFormat, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    groupVBoxLayout->addWidget(mReplyUsingVisualFormat);

    // "Improve plain text of HTML" checkbox
    mImprovePlainTextOfHtmlMessage = new QCheckBox(MessageComposer::MessageComposerSettings::self()->improvePlainTextOfHtmlMessageItem()->label(), this);

    // For what is supported see http://www.grantlee.org/apidox/classGrantlee_1_1PlainTextMarkupBuilder.html
    helpText =
        i18n("Format the plain text part of a message from the HTML markup. Bold, italic and underlined text, lists, and external references are supported.");
    mImprovePlainTextOfHtmlMessage->setToolTip(helpText);
    mImprovePlainTextOfHtmlMessage->setWhatsThis(helpText);

    connect(mImprovePlainTextOfHtmlMessage, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    groupVBoxLayout->addWidget(mImprovePlainTextOfHtmlMessage);
    QLabel *label = nullptr;
#if KDEPIM_ENTERPRISE_BUILD
    // "Default forwarding type" combobox
    mForwardTypeCombo = new QComboBox(this);
    mForwardTypeCombo->addItems(QStringList() << i18nc("@item:inlistbox Inline mail forwarding", "Inline") << i18n("As Attachment"));

    helpText = i18n("Set the default forwarded message format");
    mForwardTypeCombo->setToolTip(helpText);
    mForwardTypeCombo->setWhatsThis(helpText);

    label = new QLabel(i18n("Default forwarding type:"), this);
    label->setBuddy(mForwardTypeCombo);

    connect(mForwardTypeCombo, &QComboBox::activated, this, &ComposerPageGeneralTab::slotEmitChanged);

    auto forwardTypeWrapper = new QWidget;
    auto forwardTypeWrapperLayout = new QHBoxLayout(forwardTypeWrapper);
    forwardTypeWrapperLayout->setContentsMargins({});
    forwardTypeWrapperLayout->addWidget(label);
    forwardTypeWrapperLayout->addWidget(mForwardTypeCombo);
    forwardTypeWrapperLayout->addStretch();
    groupVBoxLayout->addWidget(forwardTypeWrapper);
#endif

    groupBox->setLayout(groupVBoxLayout);
    layout->addWidget(groupBox);

    // "Recipients" group
    groupBox = new QGroupBox(i18nc("@title:group", "Recipients"));
    auto groupHBoxLayout = new QHBoxLayout(groupBox);
    auto groupBoxWrapper = new QWidget;
    auto groupGridLayout = new QGridLayout(groupBoxWrapper);
    groupHBoxLayout->addWidget(groupBoxWrapper);
    groupHBoxLayout->addStretch();

    int row = 0;

    // "Automatically request MDNs" checkbox
    mAutoRequestMDNCheck = new QCheckBox(KMailSettings::self()->requestMDNItem()->label(), this);

    helpText = i18n(
        "By default, request an MDN when starting to compose a message. You can select this on a per-message basis using \"Options - Request Disposition "
        "Notification\"");
    mAutoRequestMDNCheck->setToolTip(helpText);
    mAutoRequestMDNCheck->setWhatsThis(helpText);

    connect(mAutoRequestMDNCheck, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    groupGridLayout->addWidget(mAutoRequestMDNCheck, row, 0, 1, -1);
    ++row;

    // Spacing
    ++row;

    // "Use Baloo search in composer" checkbox
    mShowAkonadiSearchAddressesInComposer = new QCheckBox(MessageComposer::MessageComposerSettings::self()->showBalooSearchInComposerItem()->label(), this);

    connect(mShowAkonadiSearchAddressesInComposer, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    groupGridLayout->addWidget(mShowAkonadiSearchAddressesInComposer, row, 0, 1, -1);
    ++row;

#if KDEPIM_ENTERPRISE_BUILD
    // "Warn if too many recipients" checkbox/spinbox
    mRecipientCheck = new QCheckBox(KMailSettings::self()->tooManyRecipientsItem()->label(), this);
    mRecipientCheck->setObjectName(QStringLiteral("kcfg_TooManyRecipients"));
    helpText = i18n(KMailSettings::self()->tooManyRecipientsItem()->whatsThis().toUtf8().constData());
    mRecipientCheck->setWhatsThis(helpText);
    mRecipientCheck->setToolTip(i18n("Warn if too many recipients are specified"));

    mRecipientSpin = new QSpinBox(this);
    mRecipientSpin->setMaximum(100 /*max*/);
    mRecipientSpin->setMinimum(1 /*min*/);
    mRecipientSpin->setSingleStep(1 /*step*/);
    mRecipientSpin->setValue(5 /*init*/);
    mRecipientSpin->setObjectName(QStringLiteral("kcfg_RecipientThreshold"));
    mRecipientSpin->setEnabled(false);
    helpText = i18n(KMailSettings::self()->recipientThresholdItem()->whatsThis().toUtf8().constData());
    mRecipientSpin->setWhatsThis(helpText);
    mRecipientSpin->setToolTip(i18n("Set the maximum number of recipients for the warning"));

    connect(mRecipientCheck, &QCheckBox::stateChanged, this, &ComposerPageGeneralTab::slotEmitChanged);
    connect(mRecipientSpin, &QSpinBox::valueChanged, this, &ComposerPageGeneralTab::slotEmitChanged);
    // only enable the spinbox if the checkbox is checked
    connect(mRecipientCheck, &QCheckBox::toggled, mRecipientSpin, &QSpinBox::setEnabled);

    groupGridLayout->addWidget(mRecipientCheck, row, 0, 1, 2);
    groupGridLayout->addWidget(mRecipientSpin, row, 2);
    ++row;
#endif

    // "Maximum Reply-to-All recipients" spinbox
    mMaximumRecipients = new QSpinBox(this);
    mMaximumRecipients->setMaximum(9999);
    mMaximumRecipients->setMinimum(0);
    mMaximumRecipients->setSingleStep(1);
    mMaximumRecipients->setValue(1);

    helpText = i18n(
        "Only allow this many recipients to be specified for the message. This applies to doing a \"Reply to All\", entering recipients manually"
        " or using the \"Select...\" picker.  Setting this limit helps you to avoid accidentally sending a message to too many people.  Note,"
        " however, that it does not take account of distribution lists or mailing lists.");
    mMaximumRecipients->setToolTip(helpText);
    mMaximumRecipients->setWhatsThis(helpText);

    label = new QLabel(MessageComposer::MessageComposerSettings::self()->maximumRecipientsItem()->label(), this);
    label->setBuddy(mMaximumRecipients);

    connect(mMaximumRecipients, &QSpinBox::valueChanged, this, &ConfigModuleTab::slotEmitChanged);

    groupGridLayout->addWidget(label, row, 0, 1, 2);
    groupGridLayout->addWidget(mMaximumRecipients, row, 2);
    ++row;

    // Spacing
    ++row;

    // "Use recent addresses for autocompletion" checkbox
    mShowRecentAddressesInComposer = new QCheckBox(MessageComposer::MessageComposerSettings::self()->showRecentAddressesInComposerItem()->label(), this);

    helpText = i18n("Remember recent addresses entered, and offer them for recipient completion");
    mShowRecentAddressesInComposer->setToolTip(helpText);
    mShowRecentAddressesInComposer->setWhatsThis(helpText);

    connect(mShowRecentAddressesInComposer, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    groupGridLayout->addWidget(mShowRecentAddressesInComposer, row, 0, 1, -1);
    ++row;

    // "Maximum recent addresses retained" spinbox
    mMaximumRecentAddress = new QSpinBox(this);
    mMaximumRecentAddress->setMinimum(0);
    mMaximumRecentAddress->setMaximum(999);
    mMaximumRecentAddress->setSpecialValueText(i18nc("No addresses are retained", "No save"));
    mMaximumRecentAddress->setEnabled(false);

    label = new QLabel(i18n("Maximum recent addresses retained:"));
    label->setBuddy(mMaximumRecentAddress);
    label->setEnabled(false);

    helpText = i18n("The maximum number of recently entered addresses that will be remembered for completion");
    mMaximumRecentAddress->setToolTip(helpText);
    mMaximumRecentAddress->setWhatsThis(helpText);

    connect(mMaximumRecentAddress, &QSpinBox::valueChanged, this, &ConfigModuleTab::slotEmitChanged);
    connect(mShowRecentAddressesInComposer, &QAbstractButton::toggled, mMaximumRecentAddress, &QWidget::setEnabled);
    connect(mShowRecentAddressesInComposer, &QAbstractButton::toggled, label, &QWidget::setEnabled);

    groupGridLayout->addWidget(label, row, 0, 1, 2);
    groupGridLayout->addWidget(mMaximumRecentAddress, row, 2);
    ++row;

    // Configure All Address settings
    auto configureCompletionButton = new QPushButton(i18n("Configure Completion..."), this);
    connect(configureCompletionButton, &QAbstractButton::clicked, this, &ComposerPageGeneralTab::slotConfigureAddressCompletion);
    groupGridLayout->addWidget(configureCompletionButton, row, 1, 1, 2);
    groupGridLayout->setRowStretch(row, 1);

    groupBox->setLayout(groupGridLayout);
    layout->addWidget(groupBox);

    // "Autosave" group
    groupBox = new QGroupBox(i18nc("@title:group", "Autosave"));
    groupHBoxLayout = new QHBoxLayout(groupBox);

    // "Autosave interval" spinbox
    mAutoSave = new KPluralHandlingSpinBox(this);
    mAutoSave->setMaximum(60);
    mAutoSave->setMinimum(0);
    mAutoSave->setSingleStep(1);
    mAutoSave->setValue(1);
    mAutoSave->setObjectName(QStringLiteral("kcfg_AutosaveInterval"));
    mAutoSave->setSpecialValueText(i18n("No autosave"));
    mAutoSave->setSuffix(ki18ncp("Interval suffix", " minute", " minutes"));

    helpText = i18n("Automatically save the message at this specified interval");
    mAutoSave->setToolTip(helpText);
    mAutoSave->setWhatsThis(helpText);

    label = new QLabel(KMailSettings::self()->autosaveIntervalItem()->label(), this);
    label->setBuddy(mAutoSave);

    connect(mAutoSave, &QSpinBox::valueChanged, this, &ConfigModuleTab::slotEmitChanged);

    groupHBoxLayout->addWidget(label);
    groupHBoxLayout->addWidget(mAutoSave);
    groupHBoxLayout->addStretch();

    layout->addWidget(groupBox);

    // Prevent all other tabs to inherit the height of this one
    auto scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(wrapper);
    mainLayout->addWidget(scrollArea);
}

void ComposerPageGeneralTab::doResetToDefaultsOther()
{
    const bool bUseDefaults = MessageComposer::MessageComposerSettings::self()->useDefaults(true);

    const bool autoAppSignFile = MessageComposer::MessageComposerSettings::self()->autoTextSignature() == QLatin1String("auto");
    const bool topQuoteCheck = MessageComposer::MessageComposerSettings::self()->prependSignature();
    const bool dashDashSignature = MessageComposer::MessageComposerSettings::self()->dashDashSignature();
    const bool smartQuoteCheck = MessageComposer::MessageComposerSettings::self()->quoteSelectionOnly();
    const bool wordWrap = MessageComposer::MessageComposerSettings::self()->wordWrap();
    const int wrapColumn = MessageComposer::MessageComposerSettings::self()->lineWrapWidth();
    const bool showRecentAddress = MessageComposer::MessageComposerSettings::self()->showRecentAddressesInComposer();
    const int maximumRecipient = MessageComposer::MessageComposerSettings::self()->maximumRecipients();
    const bool improvePlainText = MessageComposer::MessageComposerSettings::self()->improvePlainTextOfHtmlMessage();
    const bool showBalooSearchInComposer = MessageComposer::MessageComposerSettings::self()->showBalooSearchInComposer();
    MessageComposer::MessageComposerSettings::self()->useDefaults(bUseDefaults);

    mAutoAppSignFileCheck->setChecked(autoAppSignFile);
    mTopQuoteCheck->setChecked(topQuoteCheck);
    mDashDashCheck->setChecked(dashDashSignature);
    mQuoteSelectionOnlyCheck->setChecked(smartQuoteCheck);
    mWordWrapCheck->setChecked(wordWrap);
    mWrapColumnSpin->setValue(wrapColumn);
    mMaximumRecipients->setValue(maximumRecipient);
    mShowRecentAddressesInComposer->setChecked(showRecentAddress);
    mShowAkonadiSearchAddressesInComposer->setChecked(showBalooSearchInComposer);
    mImprovePlainTextOfHtmlMessage->setChecked(improvePlainText);

    mMaximumRecentAddress->setValue(200);
}

void ComposerPageGeneralTab::doLoadFromGlobalSettings()
{
    // various check boxes:

    mAutoAppSignFileCheck->setChecked(MessageComposer::MessageComposerSettings::self()->autoTextSignature() == QLatin1String("auto"));
    loadWidget(mTopQuoteCheck, MessageComposer::MessageComposerSettings::self()->prependSignatureItem());
    loadWidget(mDashDashCheck, MessageComposer::MessageComposerSettings::self()->dashDashSignatureItem());
    loadWidget(mSmartQuoteCheck, TemplateParser::TemplateParserSettings::self()->smartQuoteItem());
    loadWidget(mQuoteSelectionOnlyCheck, MessageComposer::MessageComposerSettings::self()->quoteSelectionOnlyItem());

    loadWidget(mReplyUsingVisualFormat, TemplateParser::TemplateParserSettings::self()->replyUsingVisualFormatItem());
    loadWidget(mStripSignatureCheck, TemplateParser::TemplateParserSettings::self()->stripSignatureItem());
    loadWidget(mAutoRequestMDNCheck, KMailSettings::self()->requestMDNItem());
    loadWidget(mWordWrapCheck, MessageComposer::MessageComposerSettings::self()->wordWrapItem());

    loadWidget(mWrapColumnSpin, MessageComposer::MessageComposerSettings::self()->lineWrapWidthItem());
    loadWidget(mMaximumRecipients, MessageComposer::MessageComposerSettings::self()->maximumRecipientsItem());
    mAutoSave->setValue(KMailSettings::self()->autosaveInterval());
    loadWidget(mShowRecentAddressesInComposer, MessageComposer::MessageComposerSettings::self()->showRecentAddressesInComposerItem());
    loadWidget(mShowAkonadiSearchAddressesInComposer, MessageComposer::MessageComposerSettings::self()->showBalooSearchInComposerItem());
    mImprovePlainTextOfHtmlMessage->setChecked(MessageComposer::MessageComposerSettings::self()->improvePlainTextOfHtmlMessage());

#if KDEPIM_ENTERPRISE_BUILD
    mRecipientCheck->setChecked(KMailSettings::self()->tooManyRecipients());
    mRecipientSpin->setValue(KMailSettings::self()->recipientThreshold());
    if (KMailSettings::self()->forwardingInlineByDefault()) {
        mForwardTypeCombo->setCurrentIndex(0);
    } else {
        mForwardTypeCombo->setCurrentIndex(1);
    }
#endif

    mMaximumRecentAddress->setValue(RecentAddresses::self(MessageComposer::MessageComposerSettings::self()->config())->maxCount());
}

void ComposerPageGeneralTab::save()
{
    saveCheckBox(mTopQuoteCheck, MessageComposer::MessageComposerSettings::self()->prependSignatureItem());
    saveCheckBox(mDashDashCheck, MessageComposer::MessageComposerSettings::self()->dashDashSignatureItem());
    saveCheckBox(mSmartQuoteCheck, TemplateParser::TemplateParserSettings::self()->smartQuoteItem());
    saveCheckBox(mQuoteSelectionOnlyCheck, MessageComposer::MessageComposerSettings::self()->quoteSelectionOnlyItem());

    saveCheckBox(mReplyUsingVisualFormat, TemplateParser::TemplateParserSettings::self()->replyUsingVisualFormatItem());
    saveCheckBox(mStripSignatureCheck, TemplateParser::TemplateParserSettings::self()->stripSignatureItem());
    saveCheckBox(mAutoRequestMDNCheck, KMailSettings::self()->requestMDNItem());
    saveCheckBox(mWordWrapCheck, MessageComposer::MessageComposerSettings::self()->wordWrapItem());

    MessageComposer::MessageComposerSettings::self()->setAutoTextSignature(mAutoAppSignFileCheck->isChecked() ? QStringLiteral("auto")
                                                                                                              : QStringLiteral("manual"));
    saveSpinBox(mWrapColumnSpin, MessageComposer::MessageComposerSettings::self()->lineWrapWidthItem());
    saveSpinBox(mMaximumRecipients, MessageComposer::MessageComposerSettings::self()->maximumRecipientsItem());
    KMailSettings::self()->setAutosaveInterval(mAutoSave->value());
    MessageComposer::MessageComposerSettings::self()->setShowRecentAddressesInComposer(mShowRecentAddressesInComposer->isChecked());
    MessageComposer::MessageComposerSettings::self()->setShowBalooSearchInComposer(mShowAkonadiSearchAddressesInComposer->isChecked());
    MessageComposer::MessageComposerSettings::self()->setImprovePlainTextOfHtmlMessage(mImprovePlainTextOfHtmlMessage->isChecked());
#if KDEPIM_ENTERPRISE_BUILD
    KMailSettings::self()->setTooManyRecipients(mRecipientCheck->isChecked());
    KMailSettings::self()->setRecipientThreshold(mRecipientSpin->value());
    KMailSettings::self()->setForwardingInlineByDefault(mForwardTypeCombo->currentIndex() == 0);
#endif

    RecentAddresses::self(MessageComposer::MessageComposerSettings::self()->config())->setMaxCount(mMaximumRecentAddress->value());

    MessageComposer::MessageComposerSettings::self()->requestSync();
}

void ComposerPageGeneralTab::slotConfigureAddressCompletion()
{
    KLDAPWidgets::LdapClientSearch search;
    QPointer<PimCommon::CompletionConfigureDialog> dlg(new PimCommon::CompletionConfigureDialog(this));
    dlg->setRecentAddresses(PimCommon::RecentAddresses::self(MessageComposer::MessageComposerSettings::self()->config())->addresses());
    dlg->setLdapClientSearch(&search);
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("kpimbalooblacklist"));
    KConfigGroup group(config, QStringLiteral("AddressLineEdit"));
    const QStringList balooBlackList = group.readEntry("BalooBackList", QStringList());

    dlg->setEmailBlackList(balooBlackList);
    dlg->load();
    if (dlg->exec()) {
        if (dlg->recentAddressWasChanged()) {
            PimCommon::RecentAddresses::self(MessageComposer::MessageComposerSettings::self()->config())->clear();
            dlg->storeAddresses(MessageComposer::MessageComposerSettings::self()->config());
        }
    }
    delete dlg;
}

QString ComposerPageTemplatesTab::helpAnchor() const
{
    return QStringLiteral("configure-composer-templates");
}

ComposerPageTemplatesTab::ComposerPageTemplatesTab(QWidget *parent)
    : ConfigModuleTab(parent)
    , mWidget(new TemplateParser::TemplatesConfiguration(this))
{
    auto vlay = new QVBoxLayout(this);

    vlay->addWidget(mWidget);

    connect(mWidget, &TemplateParser::TemplatesConfiguration::changed, this, &ConfigModuleTab::slotEmitChanged);
}

void ComposerPageTemplatesTab::doLoadFromGlobalSettings()
{
    mWidget->loadFromGlobal();
}

void ComposerPageTemplatesTab::save()
{
    mWidget->saveToGlobal();
}

void ComposerPageTemplatesTab::doResetToDefaultsOther()
{
    mWidget->resetToDefault();
}

QString ComposerPageCustomTemplatesTab::helpAnchor() const
{
    return QStringLiteral("configure-composer-custom-templates");
}

ComposerPageCustomTemplatesTab::ComposerPageCustomTemplatesTab(QWidget *parent)
    : ConfigModuleTab(parent)
{
    auto vlay = new QVBoxLayout(this);

    mWidget = new TemplateParser::CustomTemplates(kmkernel->getKMMainWidget() ? kmkernel->getKMMainWidget()->actionCollections() : QList<KActionCollection *>(),
                                                  this);
    vlay->addWidget(mWidget);

    connect(mWidget, &TemplateParser::CustomTemplates::changed, this, &ConfigModuleTab::slotEmitChanged);
    if (KMKernel::self()) {
        connect(mWidget, &TemplateParser::CustomTemplates::templatesUpdated, KMKernel::self(), &KMKernel::updatedTemplates);
    }
}

void ComposerPageCustomTemplatesTab::doLoadFromGlobalSettings()
{
    mWidget->load();
}

void ComposerPageCustomTemplatesTab::save()
{
    mWidget->save();
}

QString ComposerPageSubjectTab::helpAnchor() const
{
    return QStringLiteral("configure-composer-subject");
}

ComposerPageSubjectTab::ComposerPageSubjectTab(QWidget *parent)
    : ConfigModuleTab(parent)
{
    auto vlay = new QVBoxLayout(this);

    auto group = new QGroupBox(i18n("Repl&y Subject Prefixes"), this);
    auto layout = new QVBoxLayout(group);

    // row 0: help text:
    auto label = new QLabel(i18n("Recognize any sequence of the following prefixes (entries are case-insensitive regular expressions):"), group);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignLeft);

    // row 1, string list editor:
    auto buttonCode = static_cast<PimCommon::SimpleStringListEditor::ButtonCode>(
        PimCommon::SimpleStringListEditor::Add | PimCommon::SimpleStringListEditor::Remove | PimCommon::SimpleStringListEditor::Modify);
    mReplyListEditor =
        new PimCommon::SimpleStringListEditor(group, buttonCode, i18n("A&dd..."), i18n("Re&move"), i18n("Mod&ify..."), i18n("Enter new reply prefix:"));
    mReplyListEditor->setRemoveDialogLabel(i18n("Do you want to remove reply prefix?"));
    connect(mReplyListEditor, &PimCommon::SimpleStringListEditor::changed, this, &ConfigModuleTab::slotEmitChanged);
    mReplyListEditor->setAddDialogLabel(i18n("Reply Prefix:"));

    // row 2: "replace [...]" check box:
    mReplaceReplyPrefixCheck = new QCheckBox(MessageCore::MessageCoreSettings::self()->replaceReplyPrefixItem()->label(), group);
    connect(mReplaceReplyPrefixCheck, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    layout->addWidget(label);
    layout->addWidget(mReplyListEditor);
    layout->addWidget(mReplaceReplyPrefixCheck);

    vlay->addWidget(group);

    group = new QGroupBox(i18n("For&ward Subject Prefixes"), this);
    layout = new QVBoxLayout(group);

    // row 0: help text:
    label = new QLabel(i18n("Recognize any sequence of the following prefixes (entries are case-insensitive regular expressions):"), group);
    label->setAlignment(Qt::AlignLeft);
    label->setWordWrap(true);

    // row 1: string list editor
    mForwardListEditor =
        new PimCommon::SimpleStringListEditor(group, buttonCode, i18n("Add..."), i18n("Remo&ve"), i18n("Modify..."), i18n("Enter new forward prefix:"));
    mForwardListEditor->setRemoveDialogLabel(i18n("Do you want to remove forward prefix?"));
    mForwardListEditor->setAddDialogLabel(i18n("Forward Prefix:"));
    connect(mForwardListEditor, &PimCommon::SimpleStringListEditor::changed, this, &ConfigModuleTab::slotEmitChanged);

    // row 3: "replace [...]" check box:
    mReplaceForwardPrefixCheck = new QCheckBox(MessageCore::MessageCoreSettings::self()->replaceForwardPrefixItem()->label(), group);
    connect(mReplaceForwardPrefixCheck, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    layout->addWidget(label);
    layout->addWidget(mForwardListEditor);
    layout->addWidget(mReplaceForwardPrefixCheck);
    vlay->addWidget(group);
}

void ComposerPageSubjectTab::doLoadFromGlobalSettings()
{
    loadWidget(mReplyListEditor, MessageCore::MessageCoreSettings::self()->replyPrefixesItem());
    loadWidget(mForwardListEditor, MessageCore::MessageCoreSettings::self()->forwardPrefixesItem());
    loadWidget(mReplaceForwardPrefixCheck, MessageCore::MessageCoreSettings::self()->replaceForwardPrefixItem());
    loadWidget(mReplaceReplyPrefixCheck, MessageCore::MessageCoreSettings::self()->replaceReplyPrefixItem());
}

void ComposerPageSubjectTab::save()
{
    saveSimpleStringListEditor(mReplyListEditor, MessageCore::MessageCoreSettings::self()->replyPrefixesItem());
    saveSimpleStringListEditor(mForwardListEditor, MessageCore::MessageCoreSettings::self()->forwardPrefixesItem());
    saveCheckBox(mReplaceForwardPrefixCheck, MessageCore::MessageCoreSettings::self()->replaceForwardPrefixItem());
    saveCheckBox(mReplaceReplyPrefixCheck, MessageCore::MessageCoreSettings::self()->replaceReplyPrefixItem());
}

void ComposerPageSubjectTab::doResetToDefaultsOther()
{
    const bool bUseDefaults = MessageComposer::MessageComposerSettings::self()->useDefaults(true);
    loadWidget(mReplyListEditor, MessageCore::MessageCoreSettings::self()->replyPrefixesItem());
    loadWidget(mForwardListEditor, MessageCore::MessageCoreSettings::self()->forwardPrefixesItem());
    loadWidget(mReplaceForwardPrefixCheck, MessageCore::MessageCoreSettings::self()->replaceForwardPrefixItem());
    loadWidget(mReplaceReplyPrefixCheck, MessageCore::MessageCoreSettings::self()->replaceReplyPrefixItem());
    MessageComposer::MessageComposerSettings::self()->useDefaults(bUseDefaults);
}

QString ComposerPageCharsetTab::helpAnchor() const
{
    return QStringLiteral("configure-composer-charset");
}

ComposerPageCharsetTab::ComposerPageCharsetTab(QWidget *parent)
    : ConfigModuleTab(parent)
{
    auto vlay = new QVBoxLayout(this);

    auto label = new QLabel(i18n("This list is checked for every outgoing message "
                                 "from the top to the bottom for a charset that "
                                 "contains all required characters."),
                            this);
    label->setWordWrap(true);
    vlay->addWidget(label);

    mCharsetListEditor = new PimCommon::SimpleStringListEditor(this,
                                                               PimCommon::SimpleStringListEditor::All,
                                                               i18n("A&dd..."),
                                                               i18n("Remo&ve"),
                                                               i18n("&Modify..."),
                                                               i18n("Enter charset:"));
    mCharsetListEditor->setRemoveDialogLabel(i18n("Do you want to remove this selected charset?"));
    mCharsetListEditor->setAddDialogLabel(i18n("Add Charset:"));
    mCharsetListEditor->setUpDownAutoRepeat(true);
    connect(mCharsetListEditor, &PimCommon::SimpleStringListEditor::changed, this, &ConfigModuleTab::slotEmitChanged);

    vlay->addWidget(mCharsetListEditor, 1);

    mKeepReplyCharsetCheck = new QCheckBox(i18n("&Keep original charset when "
                                                "replying or forwarding (if "
                                                "possible)"),
                                           this);
    connect(mKeepReplyCharsetCheck, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    vlay->addWidget(mKeepReplyCharsetCheck);

    connect(mCharsetListEditor, &PimCommon::SimpleStringListEditor::aboutToAdd, this, &ComposerPageCharsetTab::slotVerifyCharset);
    setEnabled(kmkernel);
}

void ComposerPageCharsetTab::slotVerifyCharset(QString &charset)
{
    if (charset.isEmpty()) {
        return;
    }

    // KCharsets::codecForName("us-ascii") returns "iso-8859-1" (cf. Bug #49812)
    // therefore we have to treat this case specially
    const QString charsetLower = charset.toLower();
    if (charsetLower == QLatin1String("us-ascii")) {
        charset = QStringLiteral("us-ascii");
        return;
    } else if (charsetLower == QLatin1String("locale")) {
        charset = QStringLiteral("%1 (locale)").arg(QString::fromLatin1(kmkernel->networkCodec()->name()).toLower());
        return;
    }

    QStringEncoder codec(charset.toLatin1().constData());
    if (codec.isValid()) {
        charset = QString::fromLatin1(codec.name()).toLower();
        return;
    }

    KMessageBox::error(this, i18n("This charset is not supported."));
    charset.clear();
}

void ComposerPageCharsetTab::doLoadOther()
{
    if (!kmkernel) {
        return;
    }
    QStringList charsets = MessageComposer::MessageComposerSettings::preferredCharsets();
    QStringList::Iterator end(charsets.end());
    for (QStringList::Iterator it = charsets.begin(); it != end; ++it) {
        if ((*it) == QLatin1String("locale")) {
            QByteArray cset = kmkernel->networkCodec()->name();
            cset = cset.toLower();
            (*it) = QStringLiteral("%1 (locale)").arg(QString::fromLatin1(cset));
        }
    }

    mCharsetListEditor->setStringList(charsets);
    loadWidget(mKeepReplyCharsetCheck, MessageComposer::MessageComposerSettings::self()->forceReplyCharsetItem());
}

void ComposerPageCharsetTab::doResetToDefaultsOther()
{
    const bool bUseDefaults = MessageComposer::MessageComposerSettings::self()->useDefaults(true);
    mCharsetListEditor->setStringList(MessageComposer::MessageComposerSettings::preferredCharsets());
    mKeepReplyCharsetCheck->setChecked(MessageComposer::MessageComposerSettings::forceReplyCharset());
    saveCheckBox(mKeepReplyCharsetCheck, MessageComposer::MessageComposerSettings::self()->forceReplyCharsetItem());

    MessageComposer::MessageComposerSettings::self()->useDefaults(bUseDefaults);
    slotEmitChanged();
}

void ComposerPageCharsetTab::save()
{
    if (!kmkernel) {
        return;
    }
    QStringList charsetList = mCharsetListEditor->stringList();
    QStringList::Iterator it = charsetList.begin();
    QStringList::Iterator end = charsetList.end();

    for (; it != end; ++it) {
        if ((*it).endsWith(QLatin1String("(locale)"))) {
            (*it) = QStringLiteral("locale");
        }
    }
    MessageComposer::MessageComposerSettings::setPreferredCharsets(charsetList);
    saveCheckBox(mKeepReplyCharsetCheck, MessageComposer::MessageComposerSettings::self()->forceReplyCharsetItem());
}

QString ComposerPageHeadersTab::helpAnchor() const
{
    return QStringLiteral("configure-composer-headers");
}

ComposerPageHeadersTab::ComposerPageHeadersTab(QWidget *parent)
    : ConfigModuleTab(parent)
{
    auto vlay = new QVBoxLayout(this);

    // "Use custom Message-Id suffix" checkbox:
    mCreateOwnMessageIdCheck = new QCheckBox(i18n("&Use custom message-id suffix"), this);
    connect(mCreateOwnMessageIdCheck, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    vlay->addWidget(mCreateOwnMessageIdCheck);

    // "Message-Id suffix" line edit and label:
    auto hlay = new QHBoxLayout(); // inherits spacing
    vlay->addLayout(hlay);
    mMessageIdSuffixEdit = new QLineEdit(this);
    mMessageIdSuffixEdit->setClearButtonEnabled(true);
    // only ASCII letters, digits, plus, minus and dots are allowed
    auto messageIdSuffixValidator = new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[a-zA-Z0-9+-]+(?:\\.[a-zA-Z0-9+-]+)*")), this);
    mMessageIdSuffixEdit->setValidator(messageIdSuffixValidator);
    auto label = new QLabel(i18n("Custom message-&id suffix:"), this);
    label->setBuddy(mMessageIdSuffixEdit);
    label->setEnabled(false); // since !mCreateOwnMessageIdCheck->isChecked()
    mMessageIdSuffixEdit->setEnabled(false);
    hlay->addWidget(label);
    hlay->addWidget(mMessageIdSuffixEdit, 1);
    connect(mCreateOwnMessageIdCheck, &QAbstractButton::toggled, label, &QWidget::setEnabled);
    connect(mCreateOwnMessageIdCheck, &QAbstractButton::toggled, mMessageIdSuffixEdit, &QWidget::setEnabled);
    connect(mMessageIdSuffixEdit, &QLineEdit::textChanged, this, &ConfigModuleTab::slotEmitChanged);

    // horizontal rule and "custom header fields" label:
    vlay->addWidget(new KSeparator(Qt::Horizontal, this));
    vlay->addWidget(new QLabel(i18n("Define custom mime header fields:"), this));

    // "custom header fields" listbox:
    auto glay = new QGridLayout(); // inherits spacing
    vlay->addLayout(glay);
    glay->setRowStretch(2, 1);
    glay->setColumnStretch(1, 1);
    mHeaderList = new ListView(this);
    mHeaderList->setHeaderLabels(QStringList() << i18nc("@title:column Name of the mime header.", "Name")
                                               << i18nc("@title:column Value of the mimeheader.", "Value"));
    mHeaderList->setSortingEnabled(false);
    connect(mHeaderList, &QTreeWidget::currentItemChanged, this, &ComposerPageHeadersTab::slotMimeHeaderSelectionChanged);
    connect(mHeaderList, &ListView::addHeader, this, &ComposerPageHeadersTab::slotNewMimeHeader);
    connect(mHeaderList, &ListView::removeHeader, this, &ComposerPageHeadersTab::slotRemoveMimeHeader);
    glay->addWidget(mHeaderList, 0, 0, 3, 2);

    // "new" and "remove" buttons:
    auto button = new QPushButton(i18nc("@action:button Add new mime header field.", "Ne&w"), this);
    connect(button, &QAbstractButton::clicked, this, &ComposerPageHeadersTab::slotNewMimeHeader);
    button->setAutoDefault(false);
    glay->addWidget(button, 0, 2);
    mRemoveHeaderButton = new QPushButton(i18n("Re&move"), this);
    connect(mRemoveHeaderButton, &QAbstractButton::clicked, this, &ComposerPageHeadersTab::slotRemoveMimeHeader);
    button->setAutoDefault(false);
    glay->addWidget(mRemoveHeaderButton, 1, 2);

    // "name" and "value" line edits and labels:
    mTagNameEdit = new QLineEdit(this);
    mTagNameEdit->setClearButtonEnabled(true);
    mTagNameEdit->setEnabled(false);
    mTagNameLabel = new QLabel(i18nc("@label:textbox Name of the mime header.", "&Name:"), this);
    mTagNameLabel->setBuddy(mTagNameEdit);
    mTagNameLabel->setEnabled(false);
    glay->addWidget(mTagNameLabel, 3, 0);
    glay->addWidget(mTagNameEdit, 3, 1);
    connect(mTagNameEdit, &QLineEdit::textChanged, this, &ComposerPageHeadersTab::slotMimeHeaderNameChanged);

    mTagValueEdit = new QLineEdit(this);
    mTagValueEdit->setClearButtonEnabled(true);
    mTagValueEdit->setEnabled(false);
    mTagValueLabel = new QLabel(i18n("&Value:"), this);
    mTagValueLabel->setBuddy(mTagValueEdit);
    mTagValueLabel->setEnabled(false);
    glay->addWidget(mTagValueLabel, 4, 0);
    glay->addWidget(mTagValueEdit, 4, 1);
    connect(mTagValueEdit, &QLineEdit::textChanged, this, &ComposerPageHeadersTab::slotMimeHeaderValueChanged);
}

void ComposerPageHeadersTab::slotMimeHeaderSelectionChanged()
{
    mEmitChanges = false;
    QTreeWidgetItem *item = mHeaderList->currentItem();

    if (item) {
        mTagNameEdit->setText(item->text(0));
        mTagValueEdit->setText(item->text(1));
    } else {
        mTagNameEdit->clear();
        mTagValueEdit->clear();
    }
    mRemoveHeaderButton->setEnabled(item);
    mTagNameEdit->setEnabled(item);
    mTagValueEdit->setEnabled(item);
    mTagNameLabel->setEnabled(item);
    mTagValueLabel->setEnabled(item);
    mEmitChanges = true;
}

void ComposerPageHeadersTab::slotMimeHeaderNameChanged(const QString &text)
{
    // is called on ::setup(), when clearing the line edits. So be
    // prepared to not find a selection:
    QTreeWidgetItem *item = mHeaderList->currentItem();
    if (item) {
        item->setText(0, text);
    }
    slotEmitChanged();
}

void ComposerPageHeadersTab::slotMimeHeaderValueChanged(const QString &text)
{
    // is called on ::setup(), when clearing the line edits. So be
    // prepared to not find a selection:
    QTreeWidgetItem *item = mHeaderList->currentItem();
    if (item) {
        item->setText(1, text);
    }
    slotEmitChanged();
}

void ComposerPageHeadersTab::slotNewMimeHeader()
{
    auto listItem = new QTreeWidgetItem(mHeaderList);
    mHeaderList->setCurrentItem(listItem);
    slotEmitChanged();
}

void ComposerPageHeadersTab::slotRemoveMimeHeader()
{
    // calling this w/o selection is a programming error:
    QTreeWidgetItem *item = mHeaderList->currentItem();
    if (!item) {
        qCDebug(KMAIL_LOG) << "=================================================="
                           << "Error: Remove button was pressed although no custom header was selected\n"
                           << "==================================================\n";
        return;
    }

    QTreeWidgetItem *below = mHeaderList->itemBelow(item);

    if (below) {
        qCDebug(KMAIL_LOG) << "below";
        mHeaderList->setCurrentItem(below);
        delete item;
        item = nullptr;
    } else if (mHeaderList->topLevelItemCount() > 0) {
        delete item;
        item = nullptr;
        mHeaderList->setCurrentItem(mHeaderList->topLevelItem(mHeaderList->topLevelItemCount() - 1));
    }

    slotEmitChanged();
}

void ComposerPageHeadersTab::doLoadOther()
{
    mMessageIdSuffixEdit->setText(MessageComposer::MessageComposerSettings::customMsgIDSuffix());
    const bool state =
        (!MessageComposer::MessageComposerSettings::customMsgIDSuffix().isEmpty() && MessageComposer::MessageComposerSettings::useCustomMessageIdSuffix());
    mCreateOwnMessageIdCheck->setChecked(state);

    mHeaderList->clear();
    mTagNameEdit->clear();
    mTagValueEdit->clear();

    QTreeWidgetItem *item = nullptr;

    const int count = KMailSettings::self()->customMessageHeadersCount();
    for (int i = 0; i < count; ++i) {
        KConfigGroup config(KMKernel::self()->config(), QLatin1String("Mime #") + QString::number(i));
        const QString name = config.readEntry("name");
        const QString value = config.readEntry("value");
        if (!name.isEmpty()) {
            item = new QTreeWidgetItem(mHeaderList, item);
            item->setText(0, name);
            item->setText(1, value);
        }
    }
    if (mHeaderList->topLevelItemCount() > 0) {
        mHeaderList->setCurrentItem(mHeaderList->topLevelItem(0));
    } else {
        // disable the "Remove" button
        mRemoveHeaderButton->setEnabled(false);
    }
}

void ComposerPageHeadersTab::save()
{
    MessageComposer::MessageComposerSettings::self()->setCustomMsgIDSuffix(mMessageIdSuffixEdit->text());
    MessageComposer::MessageComposerSettings::self()->setUseCustomMessageIdSuffix(mCreateOwnMessageIdCheck->isChecked());

    // Clean config
    const int oldHeadersCount = KMailSettings::self()->customMessageHeadersCount();
    for (int i = 0; i < oldHeadersCount; ++i) {
        const QString groupMimeName = QStringLiteral("Mime #%1").arg(i);
        if (KMKernel::self()->config()->hasGroup(groupMimeName)) {
            KConfigGroup config(KMKernel::self()->config(), groupMimeName);
            config.deleteGroup();
        }
    }

    int numValidEntries = 0;
    QTreeWidgetItem *item = nullptr;
    const int numberOfEntry = mHeaderList->topLevelItemCount();
    for (int i = 0; i < numberOfEntry; ++i) {
        item = mHeaderList->topLevelItem(i);
        const QString str = item->text(0).trimmed();
        if (!str.isEmpty()) {
            if (str == QLatin1String("Content-Type")) {
                KMessageBox::error(this, i18n("\'Content-Type\' is not an authorized string. This header will be not saved."), i18n("Invalid header"));
                continue;
            }
            KConfigGroup config(KMKernel::self()->config(), QStringLiteral("Mime #%1").arg(numValidEntries));
            config.writeEntry("name", str);
            config.writeEntry("value", item->text(1));
            numValidEntries++;
        }
    }
    KMailSettings::self()->setCustomMessageHeadersCount(numValidEntries);
}

void ComposerPageHeadersTab::doResetToDefaultsOther()
{
    const bool bUseDefaults = MessageComposer::MessageComposerSettings::self()->useDefaults(true);
    const QString messageIdSuffix = MessageComposer::MessageComposerSettings::customMsgIDSuffix();
    const bool useCustomMessageIdSuffix = MessageComposer::MessageComposerSettings::useCustomMessageIdSuffix();
    MessageComposer::MessageComposerSettings::self()->useDefaults(bUseDefaults);

    mMessageIdSuffixEdit->setText(messageIdSuffix);
    const bool state = (!messageIdSuffix.isEmpty() && useCustomMessageIdSuffix);
    mCreateOwnMessageIdCheck->setChecked(state);

    mHeaderList->clear();
    mTagNameEdit->clear();
    mTagValueEdit->clear();
    // disable the "Remove" button
    mRemoveHeaderButton->setEnabled(false);
}

QString ComposerPageAttachmentsTab::helpAnchor() const
{
    return QStringLiteral("configure-composer-attachments");
}

ComposerPageAttachmentsTab::ComposerPageAttachmentsTab(QWidget *parent)
    : ConfigModuleTab(parent)
{
    auto vlay = new QVBoxLayout(this);

    // "Outlook compatible attachment naming" check box
    mOutlookCompatibleCheck = new QCheckBox(i18n("Outlook-compatible attachment naming"), this);
    mOutlookCompatibleCheck->setChecked(false);
    mOutlookCompatibleCheck->setToolTip(
        i18n("Turn this option on to make Outlook(tm) understand attachment names "
             "containing non-English characters"));
    connect(mOutlookCompatibleCheck, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    connect(mOutlookCompatibleCheck, &QAbstractButton::clicked, this, &ComposerPageAttachmentsTab::slotOutlookCompatibleClicked);
    vlay->addWidget(mOutlookCompatibleCheck);
    vlay->addSpacing(5);

    // "Enable detection of missing attachments" check box
    mMissingAttachmentDetectionCheck = new QCheckBox(i18n("E&nable detection of missing attachments"), this);
    mMissingAttachmentDetectionCheck->setChecked(true);
    connect(mMissingAttachmentDetectionCheck, &QCheckBox::stateChanged, this, &ConfigModuleTab::slotEmitChanged);
    vlay->addWidget(mMissingAttachmentDetectionCheck);

    // "Attachment key words" label and string list editor
    auto label = new QLabel(i18n("Recognize any of the following key words as "
                                 "intention to attach a file:"),
                            this);
    label->setAlignment(Qt::AlignLeft);
    label->setWordWrap(true);

    vlay->addWidget(label);

    auto buttonCode = static_cast<PimCommon::SimpleStringListEditor::ButtonCode>(
        PimCommon::SimpleStringListEditor::Add | PimCommon::SimpleStringListEditor::Remove | PimCommon::SimpleStringListEditor::Modify);
    mAttachWordsListEditor =
        new PimCommon::SimpleStringListEditor(this, buttonCode, i18n("A&dd..."), i18n("Re&move"), i18n("Mod&ify..."), i18n("Enter new key word:"));
    mAttachWordsListEditor->setRemoveDialogLabel(i18n("Do you want to remove this attachment word?"));
    mAttachWordsListEditor->setAddDialogLabel(i18n("Attachment Word:"));
    connect(mAttachWordsListEditor, &PimCommon::SimpleStringListEditor::changed, this, &ConfigModuleTab::slotEmitChanged);
    vlay->addWidget(mAttachWordsListEditor);

    connect(mMissingAttachmentDetectionCheck, &QAbstractButton::toggled, label, &QWidget::setEnabled);
    connect(mMissingAttachmentDetectionCheck, &QAbstractButton::toggled, mAttachWordsListEditor, &QWidget::setEnabled);

    auto layAttachment = new QHBoxLayout;
    label = new QLabel(i18n("Maximum Attachment Size:"), this);
    label->setAlignment(Qt::AlignLeft);
    layAttachment->addWidget(label);

    mMaximumAttachmentSize = new QSpinBox(this);
    mMaximumAttachmentSize->setRange(-1, 99999);
    mMaximumAttachmentSize->setSingleStep(100);
    mMaximumAttachmentSize->setSuffix(i18nc("spinbox suffix: unit for kilobyte", " kB"));
    connect(mMaximumAttachmentSize, &QSpinBox::valueChanged, this, &ConfigModuleTab::slotEmitChanged);
    mMaximumAttachmentSize->setSpecialValueText(i18n("No limit"));
    layAttachment->addWidget(mMaximumAttachmentSize);
    vlay->addLayout(layAttachment);
}

void ComposerPageAttachmentsTab::doLoadFromGlobalSettings()
{
    loadWidget(mOutlookCompatibleCheck, MessageComposer::MessageComposerSettings::self()->outlookCompatibleAttachmentsItem());
    loadWidget(mMissingAttachmentDetectionCheck, KMailSettings::self()->showForgottenAttachmentWarningItem());
    loadWidget(mAttachWordsListEditor, KMailSettings::self()->attachmentKeywordsItem());
    const int maximumAttachmentSize(MessageCore::MessageCoreSettings::self()->maximumAttachmentSize());
    mMaximumAttachmentSize->setValue(maximumAttachmentSize == -1 ? -1 : MessageCore::MessageCoreSettings::self()->maximumAttachmentSize() / 1024);
}

void ComposerPageAttachmentsTab::save()
{
    saveCheckBox(mOutlookCompatibleCheck, MessageComposer::MessageComposerSettings::self()->outlookCompatibleAttachmentsItem());
    saveCheckBox(mMissingAttachmentDetectionCheck, KMailSettings::self()->showForgottenAttachmentWarningItem());
    saveSimpleStringListEditor(mAttachWordsListEditor, KMailSettings::self()->attachmentKeywordsItem());

    KMime::setUseOutlookAttachmentEncoding(mOutlookCompatibleCheck->isChecked());
    const int maximumAttachmentSize(mMaximumAttachmentSize->value());
    MessageCore::MessageCoreSettings::self()->setMaximumAttachmentSize(maximumAttachmentSize == -1 ? -1 : maximumAttachmentSize * 1024);
}

void ComposerPageAttachmentsTab::slotOutlookCompatibleClicked()
{
    if (mOutlookCompatibleCheck->isChecked()) {
        KMessageBox::information(nullptr,
                                 i18n("You have chosen to "
                                      "encode attachment names containing non-English characters in a way that "
                                      "is understood by Outlook(tm) and other mail clients that do not "
                                      "support standard-compliant encoded attachment names.\n"
                                      "Note that KMail may create non-standard compliant messages, "
                                      "and consequently it is possible that your messages will not be "
                                      "understood by standard-compliant mail clients; so, unless you have no "
                                      "other choice, you should not enable this option."));
    }
}

ComposerPageAutoCorrectionTab::ComposerPageAutoCorrectionTab(QWidget *parent)
    : ConfigModuleTab(parent)
{
    auto vlay = new QVBoxLayout(this);
    vlay->setSpacing(0);
    vlay->setContentsMargins({});
    autocorrectionWidget = new TextAutoCorrectionWidgets::AutoCorrectionWidget(this);
    if (KMKernel::self()) {
        autocorrectionWidget->setAutoCorrection(KMKernel::self()->composerAutoCorrection());
    }
    vlay->addWidget(autocorrectionWidget);
    connect(autocorrectionWidget, &TextAutoCorrectionWidgets::AutoCorrectionWidget::changed, this, &ConfigModuleTab::slotEmitChanged);
}

QString ComposerPageAutoCorrectionTab::helpAnchor() const
{
    return QStringLiteral("configure-autocorrection");
}

void ComposerPageAutoCorrectionTab::save()
{
    autocorrectionWidget->writeConfig();
}

void ComposerPageAutoCorrectionTab::doLoadFromGlobalSettings()
{
    autocorrectionWidget->loadConfig();
}

void ComposerPageAutoCorrectionTab::doResetToDefaultsOther()
{
    autocorrectionWidget->resetToDefault();
}

ComposerPageAutoImageResizeTab::ComposerPageAutoImageResizeTab(QWidget *parent)
    : ConfigModuleTab(parent)
{
    auto vlay = new QVBoxLayout(this);
    vlay->setSpacing(0);
    vlay->setContentsMargins({});
    autoResizeWidget = new MessageComposer::ImageScalingWidget(this);
    vlay->addWidget(autoResizeWidget);
    connect(autoResizeWidget, &MessageComposer::ImageScalingWidget::changed, this, &ConfigModuleTab::slotEmitChanged);
}

QString ComposerPageAutoImageResizeTab::helpAnchor() const
{
    return QStringLiteral("configure-image-resize");
}

void ComposerPageAutoImageResizeTab::save()
{
    autoResizeWidget->writeConfig();
}

void ComposerPageAutoImageResizeTab::doLoadFromGlobalSettings()
{
    autoResizeWidget->loadConfig();
}

void ComposerPageAutoImageResizeTab::doResetToDefaultsOther()
{
    autoResizeWidget->resetToDefault();
}

#include "moc_configurecomposerpage.cpp"
