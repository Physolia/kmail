/*
    SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <MessageComposer/MessageFactoryNG>
#include <QUrl>

#include <QObject>

class QWidget;
class QAction;
class KJob;
class QAction;
class KActionMenu;
class KActionCollection;
class KXMLGUIClient;
class KMReaderWin;
class QMenu;

namespace Akonadi
{
class Item;
class StandardMailActionManager;
}

namespace TemplateParser
{
class CustomTemplatesMenu;
}

namespace KIO
{
class KUriFilterSearchProviderActions;
}
namespace KMail
{
/**
  Manages common actions that can be performed on one or more messages.
*/
class MessageActions : public QObject
{
    Q_OBJECT
public:
    explicit MessageActions(KActionCollection *ac, QWidget *parent);
    ~MessageActions() override;
    void setMessageView(KMReaderWin *msgView);

    /**
     * This function adds or updates the actions of the forward action menu, taking the
     * preference whether to forward inline or as attachment by default into account.
     * This has to be called when that preference config has been changed.
     */
    void setupForwardActions(KActionCollection *ac);

    /**
     * Sets up action list for forward menu.
     */
    void setupForwardingActionsList(KXMLGUIClient *guiClient);

    void setCurrentMessage(const Akonadi::Item &item, const Akonadi::Item::List &items = Akonadi::Item::List());

    KActionMenu *replyMenu() const;
    QAction *replyListAction() const;
    QAction *forwardInlineAction() const;
    QAction *forwardAttachedAction() const;
    QAction *redirectAction() const;
    QAction *newToRecipientsAction() const;

    KActionMenu *messageStatusMenu() const;
    KActionMenu *forwardMenu() const;

    QAction *annotateAction() const;
    QAction *printAction() const;
    QAction *printPreviewAction() const;
    QAction *listFilterAction() const;

    KActionMenu *mailingListActionMenu() const;
    TemplateParser::CustomTemplatesMenu *customTemplatesMenu() const;

    void addWebShortcutsMenu(QMenu *menu, const QString &text);

    QAction *debugAkonadiSearchAction() const;
    QAction *addFollowupReminderAction() const;

    QAction *sendAgainAction() const;

    QAction *newMessageFromTemplateAction() const;

    QAction *editAsNewAction() const;

    QAction *exportToPdfAction() const;

    QAction *archiveMessageAction() const;

    void fillAkonadiStandardAction(Akonadi::StandardMailActionManager *akonadiStandardActionManager);
    [[nodiscard]] Akonadi::Item currentItem() const;

Q_SIGNALS:
    // This signal is emitted when a reply is triggered and the
    // action has finished.
    // This is useful for the stand-alone reader, it might want to close the window in
    // that case.
    void replyActionFinished();

public Q_SLOTS:
    void editCurrentMessage();

private:
    Q_DISABLE_COPY(MessageActions)
    void annotateMessage();
    void updateActions();
    void replyCommand(MessageComposer::ReplyStrategy strategy);
    void addMailingListAction(const QString &item, const QUrl &url);
    void addMailingListActions(const QString &item, const QList<QUrl> &list);
    void updateMailingListActions(const Akonadi::Item &messageItem);
    void printMessage(bool preview);
    void clearMailingListActions();

private Q_SLOTS:
    void slotItemModified(const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers);
    void slotItemRemoved(const Akonadi::Item &item);

    void slotReplyToMsg();
    void slotReplyAuthorToMsg();
    void slotReplyListToMsg();
    void slotReplyAllToMsg();
    void slotNoQuoteReplyToMsg();
    void slotRunUrl(QAction *urlAction);
    void slotPrintMessage();
    void slotPrintPreviewMsg();

    void slotUpdateActionsFetchDone(KJob *job);
    void slotMailingListFilter();
    void slotDebugAkonadiSearch();

    void slotAddFollowupReminder();
    void slotResendMessage();
    void slotUseTemplate();

    void slotExportToPdf();

    void slotArchiveMessage();

private:
    QList<QAction *> mMailListActionList;
    Akonadi::Item mCurrentItem;
    Akonadi::Item::List mVisibleItems;
    QWidget *const mParent;
    KMReaderWin *mMessageView = nullptr;

    KActionMenu *const mReplyActionMenu;
    QAction *const mReplyAction;
    QAction *const mReplyAllAction;
    QAction *const mReplyAuthorAction;
    QAction *const mReplyListAction;
    QAction *const mNoQuoteReplyAction;
    QAction *const mForwardInlineAction;
    QAction *const mForwardAttachedAction;
    QAction *const mRedirectAction;
    QAction *const mNewToRecipientsAction;
    KActionMenu *const mStatusMenu;
    KActionMenu *const mForwardActionMenu;
    KActionMenu *const mMailingListActionMenu;
    QAction *const mAnnotateAction;
    QAction *const mEditAsNewAction;
    QAction *mPrintAction = nullptr;
    QAction *mPrintPreviewAction = nullptr;
    TemplateParser::CustomTemplatesMenu *mCustomTemplatesMenu = nullptr;
    QAction *const mListFilterAction;
    QAction *const mAddFollowupReminderAction;
    QAction *const mDebugAkonadiSearchAction;
    QAction *const mSendAgainAction;
    QAction *const mNewMessageFromTemplateAction;
    KIO::KUriFilterSearchProviderActions *const mWebShortcutMenuManager;
    QAction *const mExportToPdfAction;
    QAction *const mArchiveMessageAction;
};
}
